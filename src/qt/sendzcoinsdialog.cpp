// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendzcoinsdialog.h"
#include "ui_sendzcoinsdialog.h"

#include "addresstablemodel.h"
#include "bitcoinunits.h"
#include "coinselectiondialog.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "sendzcoinsentry.h"
#include "walletmodel.h"

#include "base58.h"
#include "coincontrol.h"
#include "key_io.h"
#include "main.h" // mempool and minRelayTxFee
#include "ui_interface.h"
#include "txmempool.h"
#include "wallet/wallet.h"

#include <univalue.h>
#include "rpc/server.h"

#include "resultsdialog.h"

#include <QMessageBox>
#include <QScrollBar>
#include <QTextDocument>
#include <QTimer>

QList<CAmount> SendZCoinsDialog::payAmounts;

SendZCoinsDialog::SendZCoinsDialog(const PlatformStyle *platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendZCoinsDialog),
    model(0),
    fNewRecipientAllowed(true),
    platformStyle(platformStyle)
{
    ui->setupUi(this);

    if (!platformStyle->getImagesOnButtons()) {
        ui->addButton->setIcon(QIcon());
        ui->clearButton->setIcon(QIcon());
        ui->sendButton->setIcon(QIcon());
    } else {
        ui->addButton->setIcon(platformStyle->MultiColorIcon(":/images/add3"));
        ui->clearButton->setIcon(platformStyle->MultiColorIcon(":/images/clear"));
        ui->sendButton->setIcon(platformStyle->MultiColorIcon(":/images/send2"));
    }

    addEntry();

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addEntry()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    // Coin Control
    connect(ui->pushButtonCoinSelection, SIGNAL(clicked()), this, SLOT(coinSelectionButtonClicked()));

    // Coin Control: clipboard actions
    QAction *clipboardAmountAction = new QAction(tr("Copy amount"), this);
    QAction *clipboardFeeAction = new QAction(tr("Copy fee"), this);
    QAction *clipboardAfterFeeAction = new QAction(tr("Copy after fee"), this);
    QAction *clipboardLowOutputAction = new QAction(tr("Copy dust"), this);
    QAction *clipboardChangeAction = new QAction(tr("Copy change"), this);

    connect(clipboardAmountAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAmount()));
    connect(clipboardFeeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardFee()));
    connect(clipboardAfterFeeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAfterFee()));
    connect(clipboardLowOutputAction, SIGNAL(triggered()), this, SLOT(clipboardLowOutput()));
    connect(clipboardChangeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardChange()));

    ui->labelCoinSelectionAmount->addAction(clipboardAmountAction);
    ui->labelCoinSelectionFee->addAction(clipboardFeeAction);
    ui->labelCoinSelectionAfterFee->addAction(clipboardAfterFeeAction);
    ui->labelCoinSelectionLowOutput->addAction(clipboardLowOutputAction);
    ui->labelCoinSelectionChange->addAction(clipboardChangeAction);

    ui->widgetCoinSelection->hide();
}

void SendZCoinsDialog::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
    {
        for(int i = 0; i < ui->entries->count(); ++i)
        {
            SendZCoinsEntry *entry = qobject_cast<SendZCoinsEntry*>(ui->entries->itemAt(i)->widget());
            if(entry)
            {
                entry->setModel(model);
            }
        }

        setBalance(model->getBalance(), model->getUnconfirmedBalance(), model->getImmatureBalance(),
                   model->getWatchBalance(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance(),
                   model->getTBalance(), model->getZBalance(false), model->getUnshielded());
        connect(model, SIGNAL(balanceChanged(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)), this, SLOT(setBalance(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)));
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
        updateDisplayUnit();

        // Coin Control
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(coinControlUpdateLabels()));
        coinControlUpdateLabels();

        updateGlobalFeeVariables();
    }
}

SendZCoinsDialog::~SendZCoinsDialog()
{
    delete ui;
}

void SendZCoinsDialog::on_sendButton_clicked()
{
    if(!model || !model->getOptionsModel())
        return;

    QList<SendCoinsRecipient> recipients;
    bool valid = true;

    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendZCoinsEntry *entry = qobject_cast<SendZCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            if(entry->validate())
            {
                recipients.append(entry->getValue());
            }
            else
            {
                valid = false;
            }
        }
    }

    if(!valid || recipients.isEmpty() || inputAddress.isEmpty())
    {
        return;
    }

    bool fromTaddr = false;
    bool fromSapling = false;

    CTxDestination taddr = DecodeDestination(inputAddress.toStdString());
    fromTaddr = IsValidDestination(taddr);
    if (!fromTaddr) {
        auto res = DecodePaymentAddress(inputAddress.toStdString());
        if (!IsValidPaymentAddress(res)) {
            QMessageBox msgBox("", "Invalid from address, should be a taddr or zaddr.", QMessageBox::Critical, 0, 0, 0, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
            msgBox.exec();
            return;
        }
        fromSapling = boost::get<libzcash::SaplingPaymentAddress>(&res) != nullptr;
    }

    bool fromSprout = !(fromTaddr || fromSapling);
    bool noSproutAddrs = !fromSprout;

    fNewRecipientAllowed = false;

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        fNewRecipientAllowed = true;
        return;
    }

    CAmount txFee = ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE;

    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;
    prepareStatus = model->prepareZTransaction(currentTransaction, inputAddress, inputAmount, txFee);

    // process prepareStatus and on error generate message shown to user
    processSendCoinsReturn(prepareStatus,
        BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));

    if(prepareStatus.status != WalletModel::OK) {
        fNewRecipientAllowed = true;
        return;
    }

    bool containsSproutOutput = false;
    bool containsSaplingOutput = false;

    Q_FOREACH(const SendCoinsRecipient &rcp, currentTransaction.getRecipients())
    {
        CTxDestination taddr = DecodeDestination(rcp.address.toStdString());
        if (!IsValidDestination(taddr)) {
            auto res = DecodePaymentAddress(rcp.address.toStdString());
            if (IsValidPaymentAddress(res)) {
                bool toSapling = boost::get<libzcash::SaplingPaymentAddress>(&res) != nullptr;
                bool toSprout = !toSapling;
                noSproutAddrs = noSproutAddrs && toSapling;

                containsSproutOutput |= toSprout;
                containsSaplingOutput |= toSapling;

                if (containsSproutOutput && containsSaplingOutput) {
                    QMessageBox msgBox("", "Cannot send to both Sprout and Sapling addresses.", QMessageBox::Critical, 0, 0, 0, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
                    msgBox.exec();
                    return;
                }

                if ((fromSprout && toSapling) || (fromSapling && toSprout)) {
                    QMessageBox msgBox("", "Cannot send between Sprout and Sapling addresses.", QMessageBox::Critical, 0, 0, 0, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
                    msgBox.exec();
                    return;
                }
            }
        }
    }

    UniValue params(UniValue::VARR);
    UniValue amounts(UniValue::VARR);
    params.push_back(inputAddress.toStdString());

    // Format confirmation message
    QStringList formatted;
    Q_FOREACH(const SendCoinsRecipient &rcp, currentTransaction.getRecipients())
    {
        // generate bold amount string
        QString amount = "<b>" + BitcoinUnits::formatHtmlWithUnit(model->getOptionsModel()->getDisplayUnit(), rcp.amount);
        amount.append("</b>");
        // generate monospace address string
        QString address = "<span style='font-family: monospace;'>" + rcp.address;
        address.append("</span>");

        QString recipientElement;

        if (!rcp.paymentRequest.IsInitialized()) // normal payment
        {
            if(rcp.label.length() > 0) // label with address
            {
                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
                recipientElement.append(QString(" (%1)").arg(address));
            }
            else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
        }
        else if(!rcp.authenticatedMerchant.isEmpty()) // authenticated payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
        }
        else // unauthenticated payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, address);
        }

        formatted.append(recipientElement);

        UniValue json(UniValue::VOBJ);
        json.pushKV("address", rcp.address.toStdString());
        json.pushKV("amount", ValueFromAmount(rcp.amount));
        if (!rcp.message.isEmpty())
        {
            QByteArray ba;
            ba.append(rcp.message);
            QString qs = QString(ba.toHex());
            json.pushKV("memo", qs.toStdString());
        }
        amounts.push_back(json);
    }
    params.push_back(amounts);

    params.push_back(1);
    params.push_back(ValueFromAmount(txFee));

    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />%1");

    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatHtmlWithUnit(model->getOptionsModel()->getDisplayUnit(), txFee));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));
    }

    // add total amount in all subdivision units
    questionString.append("<hr />");
    CAmount totalAmount = currentTransaction.getTotalTransactionAmount() + txFee;
    QStringList alternativeUnits;
    Q_FOREACH(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
    {
        if(u != model->getOptionsModel()->getDisplayUnit())
            alternativeUnits.append(BitcoinUnits::formatHtmlWithUnit(u, totalAmount));
    }
    questionString.append(tr("Total Amount %1")
        .arg(BitcoinUnits::formatHtmlWithUnit(model->getOptionsModel()->getDisplayUnit(), totalAmount)));
    questionString.append(QString("<span style='font-size:10pt;font-weight:normal;'><br />(=%2)</span>")
        .arg(alternativeUnits.join(" " + tr("or") + "<br />")));

    SendZConfirmationDialog confirmationDialog(tr("Confirm send coins"),
        questionString.arg(formatted.join("<br />")), SEND_CONFIRM_DELAY, this);
    confirmationDialog.exec();
    QMessageBox::StandardButton retval = (QMessageBox::StandardButton)confirmationDialog.result();

    if(retval != QMessageBox::Yes)
    {
        fNewRecipientAllowed = true;
        return;
    }

    QString strStatus;

    try {
        auto ret = z_sendmany(params, false);
        QString opid = QString::fromStdString(ret.get_str());

        ResultsDialog dlg(this);
        dlg.setOperationId(opid);

        dlg.exec();
        strStatus = dlg.getResultStatus();
    } catch (std::exception &e) {
        qDebug("Error %s ", e.what());
        QMessageBox msgBox("", e.what(), QMessageBox::Critical, 0, 0, 0, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        msgBox.exec();
    } catch (...) {
        qFatal("Error <unknown>");
        QMessageBox msgBox("", "Error <unknown>", QMessageBox::Critical, 0, 0, 0, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        msgBox.exec();
    }

    if (strStatus == "success")
    {
        accept();
        coinControlUpdateLabels();
    }
    fNewRecipientAllowed = true;
}

void SendZCoinsDialog::clear()
{
    ui->coinSelectionText->clear();
    inputAmount = 0;
    inputAddress.clear();

    // Remove entries until only one left
    while(ui->entries->count())
    {
        ui->entries->takeAt(0)->widget()->deleteLater();
    }
    addEntry();

    updateTabsAndLabels();
}

void SendZCoinsDialog::reject()
{
    clear();
}

void SendZCoinsDialog::accept()
{
    clear();
}

SendZCoinsEntry *SendZCoinsDialog::addEntry()
{
    SendZCoinsEntry *entry = new SendZCoinsEntry(platformStyle, this);
    entry->setModel(model);
    ui->entries->addWidget(entry);
    connect(entry, SIGNAL(removeEntry(SendZCoinsEntry*)), this, SLOT(removeEntry(SendZCoinsEntry*)));
    connect(entry, SIGNAL(payAmountChanged()), this, SLOT(coinControlUpdateLabels()));

    // Focus the field, so that entry can start immediately
    entry->clear();
    entry->setFocus();
    ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
    qApp->processEvents();
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    if(bar)
        bar->setSliderPosition(bar->maximum());

    updateTabsAndLabels();
    return entry;
}

void SendZCoinsDialog::updateTabsAndLabels()
{
    setupTabChain(0);
    coinControlUpdateLabels();
}

void SendZCoinsDialog::removeEntry(SendZCoinsEntry* entry)
{
    entry->hide();

    // If the last entry is about to be removed add an empty one
    if (ui->entries->count() == 1)
        addEntry();

    entry->deleteLater();

    updateTabsAndLabels();
}

QWidget *SendZCoinsDialog::setupTabChain(QWidget *prev)
{
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendZCoinsEntry *entry = qobject_cast<SendZCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            prev = entry->setupTabChain(prev);
        }
    }
    QWidget::setTabOrder(prev, ui->sendButton);
    QWidget::setTabOrder(ui->sendButton, ui->clearButton);
    QWidget::setTabOrder(ui->clearButton, ui->addButton);
    return ui->addButton;
}

void SendZCoinsDialog::setAddress(const QString &address)
{
    SendZCoinsEntry *entry = 0;
    // Replace the first entry if it is still unused
    if(ui->entries->count() == 1)
    {
        SendZCoinsEntry *first = qobject_cast<SendZCoinsEntry*>(ui->entries->itemAt(0)->widget());
        if(first->isClear())
        {
            entry = first;
        }
    }
    if(!entry)
    {
        entry = addEntry();
    }

    entry->setAddress(address);
}

void SendZCoinsDialog::pasteEntry(const SendCoinsRecipient &rv)
{
    if(!fNewRecipientAllowed)
        return;

    SendZCoinsEntry *entry = 0;
    // Replace the first entry if it is still unused
    if(ui->entries->count() == 1)
    {
        SendZCoinsEntry *first = qobject_cast<SendZCoinsEntry*>(ui->entries->itemAt(0)->widget());
        if(first->isClear())
        {
            entry = first;
        }
    }
    if(!entry)
    {
        entry = addEntry();
    }

    entry->setValue(rv);
    updateTabsAndLabels();
}

bool SendZCoinsDialog::handlePaymentRequest(const SendCoinsRecipient &rv)
{
    // Just paste the entry, all pre-checks
    // are done in paymentserver.cpp.
    pasteEntry(rv);
    return true;
}

void SendZCoinsDialog::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                                  const CAmount& watchBalance, const CAmount& watchUnconfirmedBalance, const CAmount& watchImmatureBalance,
                                  const CAmount& t_balance, const CAmount& z_balance, const CAmount& unshielded)
{
    Q_UNUSED(balance);
    Q_UNUSED(unconfirmedBalance);
    Q_UNUSED(immatureBalance);
    Q_UNUSED(watchBalance);
    Q_UNUSED(watchUnconfirmedBalance);
    Q_UNUSED(watchImmatureBalance);

    if(model && model->getOptionsModel())
    {
        ui->labelBalance->setText(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), (t_balance + z_balance - unshielded)));
    }
}

void SendZCoinsDialog::updateDisplayUnit()
{
    setBalance(0, 0, 0, 0, 0, 0, model->getTBalance(), model->getZBalance(false), model->getUnshielded());
}

void SendZCoinsDialog::processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg)
{
    QPair<QString, CClientUIInterface::MessageBoxFlags> msgParams;
    // Default to a warning message, override if error message is needed
    msgParams.second = CClientUIInterface::MSG_WARNING;

    // This comment is specific to SendZCoinsDialog usage of WalletModel::SendCoinsReturn.
    // WalletModel::TransactionCommitFailed is used only in WalletModel::sendCoins()
    // all others are used only in WalletModel::prepareTransaction()
    switch(sendCoinsReturn.status)
    {
    case WalletModel::InvalidAddress:
        msgParams.first = tr("The recipient address is not valid. Please recheck.");
        break;
    case WalletModel::InvalidAmount:
        msgParams.first = tr("The amount to pay must be larger than 0.");
        break;
    case WalletModel::AmountExceedsBalance:
        msgParams.first = tr("The amount exceeds your balance.");
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        msgParams.first = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgArg);
        break;
    case WalletModel::DuplicateAddress:
        msgParams.first = tr("Duplicate address found: addresses should only be used once each.");
        break;
    case WalletModel::TransactionCreationFailed:
        msgParams.first = tr("Transaction creation failed!");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::TransactionCommitFailed:
        msgParams.first = tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::AbsurdFee:
        msgParams.first = tr("A fee higher than %1 is considered an absurdly high fee.").arg(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), maxTxFee));
        break;
    case WalletModel::PaymentRequestExpired:
        msgParams.first = tr("Payment request expired.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    // included to prevent a compiler warning.
    case WalletModel::OK:
    default:
        return;
    }

    Q_EMIT message(tr("Send Coins"), msgParams.first, msgParams.second);
}

void SendZCoinsDialog::updateGlobalFeeVariables()
{
    nTxConfirmTarget = defaultZConfirmTarget;
    payTxFee = CFeeRate(ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE);
}

// Coin Control: copy label "Amount" to clipboard
void SendZCoinsDialog::coinControlClipboardAmount()
{
    GUIUtil::setClipboard(ui->labelCoinSelectionAmount->text().left(ui->labelCoinSelectionAmount->text().indexOf(" ")));
}

// Coin Control: copy label "Fee" to clipboard
void SendZCoinsDialog::coinControlClipboardFee()
{
    GUIUtil::setClipboard(ui->labelCoinSelectionFee->text().left(ui->labelCoinSelectionFee->text().indexOf(" ")));
}

// Coin Control: copy label "After fee" to clipboard
void SendZCoinsDialog::coinControlClipboardAfterFee()
{
    GUIUtil::setClipboard(ui->labelCoinSelectionAfterFee->text().left(ui->labelCoinSelectionAfterFee->text().indexOf(" ")));
}

// copy label "Dust" to clipboard
void SendZCoinsDialog::clipboardLowOutput()
{
    GUIUtil::setClipboard(ui->labelCoinSelectionLowOutput->text());
}

// Coin Control: copy label "Change" to clipboard
void SendZCoinsDialog::coinControlClipboardChange()
{
    GUIUtil::setClipboard(ui->labelCoinSelectionChange->text().left(ui->labelCoinSelectionChange->text().indexOf(" ")));
}

// Coin Control: button inputs -> show actual coin selection dialog
void SendZCoinsDialog::coinSelectionButtonClicked()
{
    CoinSelectionDialog dlg(this);
    dlg.setModel(model);
    if(dlg.exec())
    {
        inputAddress = dlg.getReturnAddress();
        QString returnAmount = BitcoinUnits::removeSpaces(dlg.getReturnAmount());
        if (model->getOptionsModel()->getDisplayUnit() == BitcoinUnits::uLTZ)
            inputAmount = AmountFromValue(returnAmount.toStdString()) / 1000000;
        else if (model->getOptionsModel()->getDisplayUnit() == BitcoinUnits::mLTZ)
            inputAmount = AmountFromValue(returnAmount.toStdString()) / 1000;
        else
            inputAmount = AmountFromValue(returnAmount.toStdString());

        ui->coinSelectionText->setText(inputAddress);
        coinControlUpdateLabels();
    }
}

// Coin Control: update labels
void SendZCoinsDialog::coinControlUpdateLabels()
{
    if (!model || !model->getOptionsModel())
        return;

    // set pay amounts
    SendZCoinsDialog::payAmounts.clear();
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendZCoinsEntry *entry = qobject_cast<SendZCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry && !entry->isHidden())
        {
            SendCoinsRecipient rcp = entry->getValue();
            SendZCoinsDialog::payAmounts.append(rcp.amount);
        }
    }

    if (!ui->coinSelectionText->text().isEmpty())
    {
        // actual coin control calculation
        SendZCoinsDialog::updateLabels();

        // show coin control stats
        ui->widgetCoinSelection->show();
    }
    else
    {
        // hide coin control stats
        ui->widgetCoinSelection->hide();
        ui->labelCoinSelectionInsuffFunds->hide();
    }
}

void SendZCoinsDialog::updateLabels()
{
    // nPayAmount
    CAmount nPayAmount = 0;
    bool fDust = false;
    CMutableTransaction txDummy;

    Q_FOREACH(const CAmount &amount, SendZCoinsDialog::payAmounts)
    {
        nPayAmount += amount;

        if (amount > 0)
        {
            CTxOut txout(amount, (CScript)std::vector<unsigned char>(24, 0));
            txDummy.vout.push_back(txout);
            if (txout.IsDust(::minRelayTxFee))
               fDust = true;
        }
    }

    CAmount nAmount             = 0;
    CAmount nPayFee             = 0;
    CAmount nAfterFee           = 0;
    CAmount nChange             = 0;
    bool fAllowFree             = false;

    // Amount
    nAmount += inputAmount;

    // Fee
    nPayFee = ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE;

    if (nPayAmount > 0)
    {
        nChange = nAmount - nPayAmount;

        // Never create dust outputs; if we would, just add the dust to the fee.
        if (nChange > 0 && nChange < MIN_CHANGE)
        {
            CTxOut txout(nChange, (CScript)std::vector<unsigned char>(24, 0));
            if (txout.IsDust(::minRelayTxFee))
            {
                nPayFee += nChange;
                nChange = 0;
            }
        }
    }

    // after fee
    nAfterFee = nAmount - nPayFee;
    if (nAfterFee < 0)
        nAfterFee = 0;

    // actually update labels
    int nDisplayUnit = BitcoinUnits::LTZ;
    if (model && model->getOptionsModel())
        nDisplayUnit = model->getOptionsModel()->getDisplayUnit();

    // enable/disable "dust" and "change"
    ui->labelCoinSelectionLowOutputText->setEnabled(nPayAmount > 0);
    ui->labelCoinSelectionLowOutput->setEnabled(nPayAmount > 0);
    ui->labelCoinSelectionChangeText->setEnabled(nPayAmount > 0);
    ui->labelCoinSelectionChange->setEnabled(nPayAmount > 0);
 
    // stats
    ui->labelCoinSelectionAmount->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nAmount));     // Amount
    ui->labelCoinSelectionFee->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nPayFee));        // Fee
    ui->labelCoinSelectionAfterFee->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nAfterFee)); // After Fee
    ui->labelCoinSelectionChange->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nChange));     // Change
    ui->labelCoinSelectionLowOutput->setText(fDust ? tr("yes") : tr("no"));                            // Dust

    ui->labelCoinSelectionLowOutput->setStyleSheet((fDust) ? "color:red;" : "");                                     // Dust = "yes"

    QString toolTip3 = tr("This label turns red if any recipient receives an amount smaller than the current dust threshold.");

    // how many satoshis the estimated fee can vary per byte we guess wrong
    double dFeeVary;
    if (payTxFee.GetFeePerK() > 0)
        dFeeVary = (double)std::max(CWallet::minTxFee.GetFeePerK(), payTxFee.GetFeePerK()) / 1000;
    else
        dFeeVary = (double)std::max(CWallet::minTxFee.GetFeePerK(), mempool.estimateFee(nTxConfirmTarget).GetFeePerK()) / 1000;
    QString toolTip4 = tr("Can vary +/- %1 satoshi(s) per input.").arg(dFeeVary);

    ui->labelCoinSelectionFee->setToolTip(toolTip4);
    ui->labelCoinSelectionAfterFee->setToolTip(toolTip4);
    ui->labelCoinSelectionLowOutput->setToolTip(toolTip3);
    ui->labelCoinSelectionChange->setToolTip(toolTip4);

    // Insufficient funds
    ui->labelCoinSelectionInsuffFunds->setVisible(nChange < 0);
}

SendZConfirmationDialog::SendZConfirmationDialog(const QString &title, const QString &text, int secDelay,
    QWidget *parent) :
    QMessageBox(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::Cancel, parent), secDelay(secDelay)
{
    setDefaultButton(QMessageBox::Cancel);
    yesButton = button(QMessageBox::Yes);
    updateYesButton();
    connect(&countDownTimer, SIGNAL(timeout()), this, SLOT(countDown()));
}

int SendZConfirmationDialog::exec()
{
    updateYesButton();
    countDownTimer.start(1000);
    return QMessageBox::exec();
}

void SendZConfirmationDialog::countDown()
{
    secDelay--;
    updateYesButton();

    if(secDelay <= 0)
    {
        countDownTimer.stop();
    }
}

void SendZConfirmationDialog::updateYesButton()
{
    if(secDelay > 0)
    {
        yesButton->setEnabled(false);
        yesButton->setText(tr("Yes") + " (" + QString::number(secDelay) + ")");
    }
    else
    {
        yesButton->setEnabled(true);
        yesButton->setText(tr("Yes"));
    }
}

// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "shieldcoinsdialog.h"
#include "ui_shieldcoinsdialog.h"

#include "bitcoinunits.h"
#include "addressbookdialog.h"
#include "addresstablemodel.h"
#include "resultsdialog.h"

#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "walletmodel.h"
#include <univalue.h>
#include "rpc/server.h"

#include <sstream>
#include <iomanip>

#include <QApplication>
#include <QMessageBox>
#include <QTimer>

ShieldCoinsDialog::ShieldCoinsDialog(const PlatformStyle *platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShieldCoinsDialog),
    model(0),
    platformStyle(platformStyle)
{
    ui->setupUi(this);

    ui->shieldButton->setEnabled(false);
    ui->reqShieldAddress->setEnabled(false);

    // init transaction fee section
    ui->operationLimit->setMinimum(50);
    ui->operationLimit->setMaximum(5000);
    ui->operationLimit->setSingleStep(10);
    ui->operationLimit->setSuffix(" utxos");

    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void ShieldCoinsDialog::done(int retval)
{
    clear();
    QDialog::done(retval);
}

ShieldCoinsDialog::~ShieldCoinsDialog()
{
    delete ui;
}

void ShieldCoinsDialog::on_shieldButton_clicked()
{

    UniValue params(UniValue::VARR);
    UniValue ret;

    try {
        params.push_back("*");
        params.push_back(ui->reqShieldAddress->text().toStdString());
        params.push_back(ValueFromAmount(payTxFee.GetFeePerK()));
        params.push_back(ui->operationLimit->value());

        ret = z_shieldcoinbase(params, false);
    } catch (std::exception &e) {
        qFatal("Error %s ", e.what());
    } catch (...) {
        qFatal("Error <unknown>");
    }

    try {
        UniValue ret1 = find_value(ret, "remainingUTXOs");
        UniValue ret2 = find_value(ret, "remainingValue");
        UniValue ret3 = find_value(ret, "shieldingUTXOs");
        UniValue ret4 = find_value(ret, "shieldingValue");
        UniValue ret5 = find_value(ret, "opid");

        QString remainingUTXOs = QString("%1").arg(ret1.get_int());
        QString remainingValue = BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), AmountFromValue(ret2));
        QString shieldingUTXOs = QString("%1").arg(ret3.get_int());
        QString shieldingValue = BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), AmountFromValue(ret4));
        QString opid = QString::fromStdString(ret5.get_str());

        QString label1 = QString::fromStdString("") +
                         QString::fromStdString("<p><strong>remainingUTXOs:</strong></p>") +
                         QString::fromStdString("<p><strong>remainingValue:</strong></p>") +
                         QString::fromStdString("<p><strong>shieldingUTXOs:</strong></p>") +
                         QString::fromStdString("<p><strong>shieldingValue:</strong></p>");

        QString label2 = QString::fromStdString("<p>") + remainingUTXOs + QString::fromStdString("</p>") +
                         QString::fromStdString("<p>") + remainingValue + QString::fromStdString("</p>") +
                         QString::fromStdString("<p>") + shieldingUTXOs + QString::fromStdString("</p>") +
                         QString::fromStdString("<p>") + shieldingValue + QString::fromStdString("</p>");

        ResultsDialog dlg(this);
        dlg.setOperationId(opid);
        dlg.setLabels(label1, label2);
        dlg.exec();

        this->close();
    } catch (std::exception &e) {
        qDebug("Error %s ", e.what());
        QMessageBox msgBox("", e.what(), QMessageBox::Critical, 0, 0, 0, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        msgBox.exec();
    } catch (...) {
        qFatal("Error <unknown>");
        QMessageBox msgBox("", "Error <unknown>", QMessageBox::Critical, 0, 0, 0, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        msgBox.exec();
    }
}

void ShieldCoinsDialog::on_deleteButton_clicked()
{
    ui->reqShieldAddress->clear();
    ui->shieldButton->setEnabled(false);
}

void ShieldCoinsDialog::on_AddressBookButton_clicked()
{
    if(!model)
        return;

    AddressBookDialog dlg(platformStyle, AddressBookDialog::ForZSelection, AddressBookDialog::ReceivingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->reqShieldAddress->setText(dlg.getReturnValue());
        ui->shieldButton->setEnabled(true);
        ui->shieldButton->setFocus();
    }
}

void ShieldCoinsDialog::setModel(WalletModel *model)
{
    this->model = model;
    if(model && model->getOptionsModel())
    {
        updateGlobalFeeVariables();
    }
}

void ShieldCoinsDialog::updateGlobalFeeVariables()
{
    payTxFee = CFeeRate(ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE);
}

void ShieldCoinsDialog::setAddress(const QString &address)
{
    ui->reqShieldAddress->setText(address);
    ui->shieldButton->setFocus();
}

void ShieldCoinsDialog::setFocus()
{
    ui->reqShieldAddress->setFocus();
}

void ShieldCoinsDialog::clear()
{
    // clear UI elements
    ui->reqShieldAddress->clear();
}

bool ShieldCoinsDialog::isClear()
{
    return ui->reqShieldAddress->text().isEmpty();
}

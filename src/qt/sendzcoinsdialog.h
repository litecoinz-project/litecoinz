// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_SENDZCOINSDIALOG_H
#define BITCOIN_QT_SENDZCOINSDIALOG_H

#include "walletmodel.h"

#include <QDialog>
#include <QMessageBox>
#include <QString>
#include <QTimer>

#define SEND_CONFIRM_DELAY   3
#define ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE   10000

class OptionsModel;
class PlatformStyle;
class SendZCoinsEntry;
class SendCoinsRecipient;

namespace Ui {
    class SendZCoinsDialog;
}

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

const int defaultZConfirmTarget = 25;

/** Dialog for sending bitcoins */
class SendZCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendZCoinsDialog(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~SendZCoinsDialog();

    void setModel(WalletModel *model);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases (issue https://bugreports.qt-project.org/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void setAddress(const QString &address);
    void pasteEntry(const SendCoinsRecipient &rv);
    bool handlePaymentRequest(const SendCoinsRecipient &recipient);

    void updateLabels();

    static QList<CAmount> payAmounts;

    CAmount inputAmount;
    QString inputAddress;

public Q_SLOTS:
    void clear();
    void reject();
    void accept();
    SendZCoinsEntry *addEntry();
    void updateTabsAndLabels();
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance,
                    const CAmount& t_balance, const CAmount& z_balance, const CAmount& unshielded);

private:
    Ui::SendZCoinsDialog *ui;
    WalletModel *model;
    bool fNewRecipientAllowed;
    const PlatformStyle *platformStyle;

    // Process WalletModel::SendCoinsReturn and generate a pair consisting
    // of a message and message flags for use in Q_EMIT message().
    // Additional parameter msgArg can be used via .arg(msgArg).
    void processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg = QString());

private Q_SLOTS:
    void on_sendButton_clicked();
    void removeEntry(SendZCoinsEntry* entry);
    void updateDisplayUnit();
    void coinSelectionButtonClicked();
    void coinControlUpdateLabels();
    void coinControlClipboardAmount();
    void coinControlClipboardFee();
    void coinControlClipboardAfterFee();
    void clipboardLowOutput();
    void coinControlClipboardChange();
    void updateGlobalFeeVariables();

Q_SIGNALS:
    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);
};



class SendZConfirmationDialog : public QMessageBox
{
    Q_OBJECT

public:
    SendZConfirmationDialog(const QString &title, const QString &text, int secDelay = 0, QWidget *parent = 0);
    int exec();

private Q_SLOTS:
    void countDown();
    void updateYesButton();

private:
    QAbstractButton *yesButton;
    QTimer countDownTimer;
    int secDelay;
};

#endif // BITCOIN_QT_SENDZCOINSDIALOG_H

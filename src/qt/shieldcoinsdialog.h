// Copyright (c) 2017-2018 The LitecoinZ developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_SHIELDCOINSDIALOG_H
#define BITCOIN_QT_SHIELDCOINSDIALOG_H

#include "walletmodel.h"

#include <QDialog>
#include <QTimer>

#define ASYNC_RPC_OPERATION_DEFAULT_MINERS_FEE   10000

class AddressTableModel;
class WalletModel;
class PlatformStyle;

namespace Ui {
    class ShieldCoinsDialog;
}

/** Widget that shows a list of sending or receiving addresses.
  */
class ShieldCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShieldCoinsDialog(const PlatformStyle *platformStyle, QWidget *parent);
    ~ShieldCoinsDialog();

    void setModel(WalletModel *model);
    void setAddress(const QString &address);
    void setFocus();
    bool isClear();

public Q_SLOTS:
    void clear();
    void done(int retval);

private Q_SLOTS:
    /** Create a new address for receiving coins and / or add a new address book entry */
    void on_AddressBookButton_clicked();
    /** Clear zaddress field */
    void on_deleteButton_clicked();
    /** Shield coins in coinbase from taddress to zaddress */
    void on_shieldButton_clicked();

    void updateGlobalFeeVariables();

private:
    Ui::ShieldCoinsDialog *ui;
    WalletModel *model;
    const PlatformStyle *platformStyle;
};

#endif // BITCOIN_QT_SHIELDCOINSDIALOG_H

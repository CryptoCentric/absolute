#ifndef MASTERNODELIST_H
#define MASTERNODELIST_H

#include "platformstyle.h"
#include "primitives/transaction.h"
#include "sync.h"
#include "util.h"

#include <evo/deterministicmns.h>

#include <QMenu>
#include <QTimer>
#include <QWidget>

#define MY_MASTERNODELIST_UPDATE_SECONDS 60
#define MASTERNODELIST_UPDATE_SECONDS 15
#define MASTERNODELIST_FILTER_COOLDOWN_SECONDS 3

namespace Ui
{
class MasternodeList;
}

class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Masternode Manager page widget */
class MasternodeList : public QWidget
{
    Q_OBJECT

public:
    explicit MasternodeList(const PlatformStyle* platformStyle, QWidget* parent = 0);
    ~MasternodeList();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);
    void ShowQRCode(std::string strAlias);
    void StartAlias(std::string strAlias);
    void StartAll(std::string strCommand = "start-all");
    CDeterministicMNCPtr GetSelectedAIP3MN();

private:
    QMenu* contextMenu;
    QMenu* contextMenuAIP3;
    int64_t nTimeFilterUpdated;
    int64_t nTimeFilterUpdatedAIP3;
    bool fFilterUpdated;
    bool fFilterUpdatedAIP3;

public Q_SLOTS:
    void updateMyMasternodeInfo(QString strAlias, QString strAddr, const COutPoint& outpoint);
    void updateMyNodeList(bool fForce = false);
    void updateNodeList();
    void updateAIP3List();

Q_SIGNALS:
    void doubleClicked(const QModelIndex&);

private:
    QTimer* timer;
    Ui::MasternodeList* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;

    // Protects tableWidgetMasternodes
    CCriticalSection cs_mnlist;

    // Protects tableWidgetMyMasternodes
    CCriticalSection cs_mymnlist;

    // Protects tableWidgetMasternodesAIP3
    CCriticalSection cs_aip3list;

    QString strCurrentFilter;
    QString strCurrentFilterAIP3;

private Q_SLOTS:
    void showContextMenu(const QPoint&);
    void showContextMenuAIP3(const QPoint&);
    void on_filterLineEdit_textChanged(const QString& strFilterIn);
    void on_filterLineEditAIP3_textChanged(const QString& strFilterIn);
    void on_QRButton_clicked();
    void on_startButton_clicked();
    void on_startAllButton_clicked();
    void on_startMissingButton_clicked();
    void on_tableWidgetMyMasternodes_itemSelectionChanged();
    void on_UpdateButton_clicked();

    void extraInfoAIP3_clicked();
    void copyProTxHash_clicked();
    void copyCollateralOutpoint_clicked();
};
#endif // MASTERNODELIST_H

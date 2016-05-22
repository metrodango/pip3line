/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PACKETANALYSERTAB_H
#define PACKETANALYSERTAB_H

#include "tababstract.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QTimer>
#include <QQueue>
#include "sources/blocksources/blockssource.h"

namespace Ui {
class PacketAnalyserTab;
}

class DetachTabButton;
class Packet;
class PacketModelAbstract;
class PacketStyledItemDelegate;
class PacketSortFilterProxyModel;
class HexView;
class QMenu;
class QAction;
class QDialog;
class Block;
class PacketAnalyserOptionsDialog;
class SourcesOrchestatorAbstract;
class NewViewButton;
class QTabBar;
class NewViewMenu;
class OrchestratorChooser;
class SendToMenu;

class PacketAnalyserTab : public TabAbstract
{
        Q_OBJECT
    public:
        explicit PacketAnalyserTab(GuiHelper *guiHelper, QWidget *parent = 0);
        ~PacketAnalyserTab();
        void loadFromFile(QString fileName);
        int getBlockCount() const;
        ByteSourceAbstract *getSource(int blockIndex);
        ByteTableView *getHexTableView(int blockIndex);
        void setData(const QByteArray &data);
        BaseStateAbstract *getStateMngtObj();
        bool canReceiveData();
        bool getForwarding() const;
        bool getIntercepting() const;
    private slots:
        void onAddNewColumn();
        void onImport();
        void onExport();
        void onModelReseted();
        void clearCurrentPacket();
        void copyAsUpdate();
        void onSelectionChanged(const QItemSelection &selection);
        void onContextMenuAction(QAction *action);
        void onRightClick(QPoint pos);
        void updateStatus();
        void onOptionsClicked();
        void onOptionDialogClosed();
        void setOrchestrator(SourcesOrchestatorAbstract *orch);
        void receiveNewPacket(Packet * packet);
        void orchestratorForwardingChanged(bool val);
        void onForwardPressed();
        void onInterceptClicked();
        void onOrchestratorConnectionsChanged();
        void onNewTabRequested();
        void addViewTab(TabAbstract::ViewTab data);
        void onDeleteTab(int index);
        void onScrollBarChanged(int min, int max);
        void onCurrentPacketUpdated(quintptr source);
        void setTrackingLast(bool value);
        void onSendToTriggered(QAction* action);
        void onClearListClicked();
        void onCopyAs(QAction* action);
        void setIntercepting(bool value);
        void setForwarding(bool value);
        void onMergeRequested();
        void onInboundTransformRequested();
        void onOutboundTransformRequested();
        void logMessage(const QString &message,const QString &source = QString(), Pip3lineConst::LOGLEVEL level = Pip3lineConst::LSTATUS);
    private:
        static const int TIMESTAMP_COLUMN_WIDTH;
        static const int DIRECTION_COLUMN_WIDTH;
        static const int RAWDATA_COLUMN_WIDTH;
        void selectLastPacket();
        void checkIfOriginalTabNeeded();
        void removeTabButton(int index);
        void buildContextMenu();
        bool eventFilter(QObject * receiver, QEvent * event);
        Ui::PacketAnalyserTab *ui;
        DetachTabButton *detachButton;
        PacketModelAbstract *packetModel;
        PacketStyledItemDelegate *styleDelegate;
        PacketSortFilterProxyModel *sortFilterProxyModel;
        ByteSourceAbstract *bytesource;
        HexView *hexView;
        ByteSourceAbstract *oriBytesource;
        HexView *oriHexView;
        QMenu * globalContextMenu;
        SendToMenu * sendToMenu;
        QMenu * copyAsMenu;
        QAction * merge;
        QAction * split;
        QAction * deleteSelection;
        PacketAnalyserOptionsDialog * optionsDialog;

        quint32 pcapLinkType;
        bool exportFormattedXML;
        bool exportFormattedJson;
        Packet * currentPacket;
        NewViewMenu * tabHeaderViewsContextMenu;
        QTabBar *tabBarRef;
        QList<ViewTab> tabData;
        OrchestratorChooser * orchCombo;

        SourcesOrchestatorAbstract *orchestrator;
        bool forwarding;
        bool trackingLast;
        QTimer updateTimer;
        bool intercepting;
        QQueue<Packet *> packetQueue;

        friend class PacketAnalyserTabStateObj;
};

class PacketAnalyserTabStateObj : public TabStateObj
{
        Q_OBJECT
    public:
        explicit PacketAnalyserTabStateObj(PacketAnalyserTab *tab);
        ~PacketAnalyserTabStateObj();
        void run();
};

class PacketAnalyserTabClosingStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit PacketAnalyserTabClosingStateObj(PacketAnalyserTab *tab);
        ~PacketAnalyserTabClosingStateObj();
        void run();

    protected:
        PacketAnalyserTab *tab;
};

#endif // PACKETANALYSERTAB_H

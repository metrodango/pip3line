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
class FilterDialog;

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
        void addPackets(const QList<QSharedPointer<Packet> >  &packets);
        BaseStateAbstract *getStateMngtObj();
        bool canReceiveData();
        bool getForwarding() const;
        bool getIntercepting() const;
        bool isTrackingLast() const;
        void setAutoMerge(bool value);
        void registerToGlobal();
        void unregisterFromGlobal();
    private slots:
        void onAddNewColumn();
        void onImport();
        void onExport();
        void onModelReseted();
        void clearCurrentPacket();
        void copyAsUpdate();
        void sendPacketsToUpdate();
        void onSelectionChanged(const QItemSelection &selection);
        void onContextMenuAction(QAction *action);
        void onRightClick(QPoint pos);
        void onHeaderContextMenu(QPoint pos);
        void updateStatus();
        void onOptionsClicked();
        void onOptionDialogClosed();
        void setOrchestrator(SourcesOrchestatorAbstract *orch);
        void receiveNewPacket(QSharedPointer<Packet> packet);
        void onForwardPressed();
        void onInterceptClicked();
        void onOrchestratorConnectionsChanged();
        void onNewTabRequested();
        void addViewTab(TabAbstract::ViewTab data);
        void onDeleteTab(int index);
        void onScrollBarChanged(int min, int max);
        void onByteSourceUpdated(quintptr source);
        void setTrackingLast(bool value);
        void onSendToTriggered(QAction* action);
        void onClearListClicked();
        void onCopyAs(QAction* action);
        void onSendPackets(QAction* action);
        void setIntercepting(bool value);
        void setForwarding(bool value);
        void onAutoMergeRequested();
        void onInboundTransformRequested();
        void onOutboundTransformRequested();
        void onSaveLoadFinished();
        void onItemClicked(const QModelIndex &index);
        void onFilter();
        void onFilterDialogClosed();
        void onDeleteColumn();
        void logMessage(const QString &message,const QString &source = QString(), Pip3lineConst::LOGLEVEL level = Pip3lineConst::LSTATUS);
        void onFontUpdated();

    private:
        void selectLastPacket();
        void checkIfOriginalTabNeeded();
        void removeTabButton(int index);
        void buildContextMenu();
        void buildHeadersContextMenu();
        QList<ViewTab> getTabData();
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
        QMenu * headerContextMenu;
        SendToMenu * sendToMenu;
        QMenu *autoMergeMenu;
        QMenu * copyAsMenu;
        QMenu * sendPacketsMenu;
        QAction * sendPacketsToNew;
        QAction * merge;
        QAction * deleteColumn;
        QAction * split;
        QAction * deleteSelection;
        PacketAnalyserOptionsDialog * optionsDialog;
        FilterDialog * filterDialog;

        int selectedHeader;

        quint32 pcapLinkType;
        bool exportFormattedXML;
        bool exportFormattedJson;
        QSharedPointer<Packet> currentPacket;
        NewViewMenu * tabHeaderViewsContextMenu;
        QTabBar *tabBarRef;
        QList<ViewTab> tabData;
        OrchestratorChooser * orchCombo;

        SourcesOrchestatorAbstract *orchestrator;
        bool forwarding;
        bool trackingLast;
        QTimer updateTimer;
        bool intercepting;
        QQueue<QSharedPointer<Packet> > packetQueue;

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

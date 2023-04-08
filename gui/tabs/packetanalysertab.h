/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef PACKETANALYSERTAB_H
#define PACKETANALYSERTAB_H

#include "tababstract.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QTimer>
#include <QQueue>
#include "sources/blocksources/blockssource.h"
#include <pipelinecommon.h>
using namespace Pip3lineCommon;

namespace Ui {
class PacketAnalyserTab;
}

class DetachTabButton;
class PacketModelAbstract;
class PacketStyledItemDelegate;
class PacketSortFilterProxyModel;
class HexView;
class QMenu;
class QAction;
class QDialog;
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
        explicit PacketAnalyserTab(GuiHelper *guiHelper, QWidget *parent = nullptr);
        ~PacketAnalyserTab();
        void loadFromFile(QString fileName) override;
        int getBlockCount() const  override;
        ByteSourceAbstract *getSource(int blockIndex)  override;
        ByteTableView *getHexTableView(int blockIndex)  override;
        void setData(const QByteArray &data)  override;
        void addPackets(const QList<QSharedPointer<Packet> >  &packets);
        BaseStateAbstract *getStateMngtObj()  override;
        bool canReceiveData()  override;
        bool getForwarding() const;
        bool getIntercepting() const;
        bool isTrackingLast() const;
        void setAutoMerge(bool value);
        void registerToGlobal()  override;
        void unregisterFromGlobal()  override;
        bool getTrackChanges() const;
        void setTrackChanges(bool value);
        void loadConfigFrom(const QString filename, const QByteArray config = QByteArray());
    private slots:
        void onAddNewColumn();
        void onImport();
        void onExport();
        void onModelReseted();
        void clearCurrentPacket();
        void updateCopyAsMenu();
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
        void onSaveLoadFinished();
        void onItemClicked(const QModelIndex &index);
        void onFilter();
        void onFilterDialogClosed();
        void onDeleteColumn();
        void logMessage(const QString &message,const QString &source = QString(), Pip3lineConst::LOGLEVEL level = Pip3lineConst::PLSTATUS);
        void onFontUpdated();
        void onTrackingToggled(bool checked);
        void onHighlightMenu(QAction *action);
        void updateHighlightMenu();
    private:
        Q_DISABLE_COPY(PacketAnalyserTab)
        int translateTabViewIndex(int index);
        void selectLastPacket();
        void checkIfOriginalTabNeeded();
        void removeTabButton(int index);
        void buildContextMenu();
        void buildHeadersContextMenu();
        QList<ViewTab> getTabData();
        bool eventFilter(QObject * receiver, QEvent * event)  override;
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
        QMenu * highlightMenu;
        QAction * clearHighlights;
        QAction * newHighlight;
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
        bool trackChanges;
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
    private:
        Q_DISABLE_COPY(PacketAnalyserTabStateObj)
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
    private:
        Q_DISABLE_COPY(PacketAnalyserTabClosingStateObj)
};

#endif // PACKETANALYSERTAB_H

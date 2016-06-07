/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "packetanalysertab.h"
#include "ui_packetanalysertab.h"
#include "shared/guiconst.h"
#include "shared/detachtabbutton.h"
#include "loggerwidget.h"
#include "packetanalyser/packetanalyseroptionsdialog.h"
#include <QDialog>
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif
#include <QtConcurrentRun>
#include <transformchain.h>
#include <QItemSelectionModel>
#include <QPalette>
#include <QMessageBox>
#include "sources/basicsource.h"
#include "views/hexview.h"
#include "packetanalyser/memorypacketmodel.h"
#include "packetanalyser/importexportdialog.h"
#include "packetanalyser/importexportworker.h"
#include "packetanalyser/packetsortfilterproxymodel.h"
#include "packetanalyser/packetstyleditemdelegate.h"
#include "packetanalyser/packet.h"
#include "packetanalyser/sourcesorchestatorabstract.h"
#include <QDebug>
#include <QModelIndex>
#include <QMenu>
#include <QMap>
#include <threadedprocessor.h>
#include <transformmgmt.h>
#include <commonstrings.h>
#include <QScrollBar>
#include <QDialog>
#include <QAction>
#include "guihelper.h"
#include "pcapio/pcapdef.h"
#include "sources/blocksources/blockssource.h"
#include "packetanalyser/orchestratorchooser.h"
#include "shared/newviewbutton.h"
#include "shared/newviewmenu.h"
#include "quickviewitemconfig.h"
#include "shared/sendtomenu.h"

#include <state/stateorchestrator.h>

const int PacketAnalyserTab::TIMESTAMP_COLUMN_WIDTH = 150;
const int PacketAnalyserTab::DIRECTION_COLUMN_WIDTH = 25;
const int PacketAnalyserTab::RAWDATA_COLUMN_WIDTH = 200;

PacketAnalyserTab::PacketAnalyserTab(GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper, parent),
    ui(new Ui::PacketAnalyserTab),
    forwarding(false),
    trackingLast(true),
    intercepting(false)

{
    updateTimer.setInterval(200);
    updateTimer.setSingleShot(true);
    pcapLinkType = PcapDef::LINKTYPE_ETHERNET;
    orchestrator = nullptr;
    exportFormattedXML = true;
    exportFormattedJson = true;
    preTabType = GuiConst::PACKET_ANALYSER_PRETAB;

    ui->setupUi(this);
    sendToMenu = nullptr;
    copyAsMenu = nullptr;

    optionsDialog = nullptr;
    tabBarRef = nullptr;
    tabHeaderViewsContextMenu = new(std::nothrow) NewViewMenu(guiHelper, this);
    if (tabHeaderViewsContextMenu == nullptr) {
        qFatal("Cannot allocate memory for NewViewMenu X{");
    }

    connect(tabHeaderViewsContextMenu, SIGNAL(newViewRequested()), SLOT(onNewTabRequested()));

    styleDelegate = new(std::nothrow) PacketStyledItemDelegate();
    if (styleDelegate == nullptr) {
        qFatal("Cannot allocate memory for PacketStyledItemDelegate X{");
    }

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == nullptr) {
        qFatal("Cannot allocate memory for detachButton X{");
    }
    ui->toolBarLayout->insertWidget(ui->toolBarLayout->indexOf(ui->exportPushButton) + 1,detachButton);

    buildContextMenu();

    ui->forwardPushButton->setEnabled(false);
    // setting up the TableView

    // setting custom display delegate
    QAbstractItemDelegate * idold = ui->packetTableView->itemDelegate(); // getting the old one for cleaning
    ui->packetTableView->setItemDelegate(styleDelegate);
    delete idold; // cleaning the old one

    // creating the base packetModel
    packetModel = new(std::nothrow) MemoryPacketModel(guiHelper->getTransformFactory());
    if (packetModel == nullptr) {
        qFatal("Cannot allocate memory for MemoryPacketModel X{");
    }

    // creating the view proxy model
    sortFilterProxyModel = new(std::nothrow) PacketSortFilterProxyModel();
    if (sortFilterProxyModel == nullptr) {
        qFatal("Cannot allocate memory for PacketSortFilterProxyModel X{");
    }

    sortFilterProxyModel->setPacketSourceModel(packetModel);
    sortFilterProxyModel->setEnableEqualityView(true);
    sortFilterProxyModel->setEqualitycolumn(PacketModelAbstract::COLUMN_PAYLOAD,true);

    QAbstractItemModel *old = ui->packetTableView->model(); // getting the old one for cleaning
    ui->packetTableView->setModel(sortFilterProxyModel);
    delete old; // cleaning

    ui->packetTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->packetTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->packetTableView->verticalHeader()->setFont(GuiStyles::GLOBAL_REGULAR_FONT);
    ui->packetTableView->horizontalHeader()->setFont(GuiStyles::GLOBAL_REGULAR_FONT);
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_TIMESPTAMP,TIMESTAMP_COLUMN_WIDTH);
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_DIRECTION,DIRECTION_COLUMN_WIDTH);
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_PAYLOAD,RAWDATA_COLUMN_WIDTH);
    ui->packetTableView->verticalHeader()->setDefaultSectionSize(20);
#if QT_VERSION >= 0x050000
    ui->packetTableView->horizontalHeader()->setSectionsMovable( true );
#else
    ui->packetTableView->horizontalHeader()->setMovable(true);
#endif
    ui->packetTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->packetTableView->setSortingEnabled(true);

    ui->packetTableView->hideColumn(PacketModelAbstract::COLUMN_CID);

    // setting up the hex view
    bytesource = new(std::nothrow) BasicSource();
    if (bytesource == nullptr) {
        qFatal("Cannot allocate memory for BasicSource X{");
    }

    hexView = new(std::nothrow) HexView(bytesource, guiHelper,nullptr,true); // giving owership of the basicSource to the view
    if (hexView == nullptr) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    ui->viewTabWidget->addTab(hexView,GuiConst::HEXADECIMAL_STRING);

    oriBytesource = new(std::nothrow) BasicSource();
    if (oriBytesource == nullptr) {
        qFatal("Cannot allocate memory for BasicSource X{");
    }

    oriBytesource->setReadOnly(true);
    oriHexView = new(std::nothrow) HexView(oriBytesource, guiHelper,nullptr,true); // giving owership of the basicSource to the view
    if (oriHexView == nullptr) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    // removing the button of the first tab, which in this case will always be the hexview
    removeTabButton(0);

    ui->viewTabWidget->installEventFilter(this);

    orchCombo = new(std::nothrow) OrchestratorChooser(guiHelper);
    if (orchCombo == nullptr) {
        qFatal("Cannot allocate memory for OrchestratorChooser X{");
    }

    ui->toolBarLayout->insertWidget(ui->toolBarLayout->indexOf(ui->optionsPushButton), orchCombo);

    ui->trackPushButton->setChecked(trackingLast);

    setIntercepting(false);

    QMenu *autoMergeMenu = new(std::nothrow) QMenu();
    if (autoMergeMenu == nullptr) {
        qFatal("Cannot allocate memory for QMenu X{");
    }

    ui->actionAuto_merge_packets->setChecked(packetModel->isAutoMergeConsecutivePackets());
    autoMergeMenu->addAction(ui->actionAuto_merge_packets);
    autoMergeMenu->addAction(ui->actionMergePacketsNow);

    ui->mergePacketsPushButton->setMenu(autoMergeMenu);

    // connecting stuff
    connect(bytesource,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);
    connect(ui->packetTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onSelectionChanged(QItemSelection)));
    connect(ui->packetTableView,SIGNAL(customContextMenuRequested(QPoint)), SLOT(onRightClick(QPoint)));
    connect(packetModel, SIGNAL(sendRequest(TransformRequest*)), guiHelper->getCentralTransProc(), SLOT(processRequest(TransformRequest*)), Qt::QueuedConnection);
    connect(packetModel, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), guiHelper->getLogger(), SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
    connect(packetModel, SIGNAL(modelReset()), SLOT(onModelReseted()));
    connect(packetModel, SIGNAL(updated()),&updateTimer, SLOT(start()));
    connect(ui->actionMergePacketsNow, SIGNAL(triggered(bool)),this, SLOT(onMergeRequested()));
    connect(ui->actionAuto_merge_packets, SIGNAL(toggled(bool)), packetModel, SLOT(setAutoMergeConsecutivePackets(bool)));
    connect(ui->clearPushButton, SIGNAL(clicked(bool)), this, SLOT(onClearListClicked()));
    connect(orchCombo, SIGNAL(newOrchestrator(SourcesOrchestatorAbstract*)), this, SLOT(setOrchestrator(SourcesOrchestatorAbstract*)));

    connect(ui->viewTabWidget, SIGNAL(tabCloseRequested(int)), SLOT(onDeleteTab(int)));

    connect(ui->importPushButton, SIGNAL(clicked(bool)), SLOT(onImport()));
    connect(ui->exportPushButton, SIGNAL(clicked(bool)), SLOT(onExport()));
    connect(ui->optionsPushButton, SIGNAL(clicked(bool)), SLOT(onOptionsClicked()));
    connect(ui->refreshPushButton, SIGNAL(clicked(bool)), packetModel, SLOT(refreshAllColumn()));
    connect(ui->forwardPushButton, SIGNAL(clicked(bool)), this, SLOT(onForwardPressed()));
    connect(ui->addColumnPushButton, SIGNAL(clicked(bool)), this, SLOT(onAddNewColumn()));
    connect(ui->addViewPushButton, SIGNAL(clicked(bool)), this, SLOT(onNewTabRequested()));

    connect(ui->packetTableView->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onScrollBarChanged(int, int)));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));

    connect(ui->interceptPushButton, SIGNAL(clicked(bool)), this, SLOT(onInterceptClicked()));
    // connect(bytesource, SIGNAL(updated(quintptr)), this, SLOT(onCurrentPacketUpdated(quintptr))); doing that later, dynamically
    connect(ui->trackPushButton, SIGNAL(toggled(bool)), this, SLOT(setTrackingLast(bool)));
    connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(copyAsUpdate()));
}

PacketAnalyserTab::~PacketAnalyserTab()
{
    delete styleDelegate;
    delete packetModel;
    delete detachButton;
    delete ui;

    delete orchestrator;
    // hexview is deleted by the parent ui
    // byteSource should be destroyed by hexview (owner)
}

void PacketAnalyserTab::loadFromFile(QString )
{
    qCritical() << "[PacketAnalyserTab::loadFromFile] not implemented T_T";
}

int PacketAnalyserTab::getBlockCount() const
{
    return 0;
}

ByteSourceAbstract *PacketAnalyserTab::getSource(int)
{
    return bytesource;
}

ByteTableView *PacketAnalyserTab::getHexTableView(int)
{
    return hexView->getHexTableView();
}

void PacketAnalyserTab::setData(const QByteArray & data)
{
    if (data.isEmpty()) {
        return; // nothign to do data is empty
    }
    if (orchestrator != nullptr) {
        Packet *recPac = new(std::nothrow) Packet(data);
        if (recPac == nullptr)
            qFatal("Cannot allocate memory for Packet X{");

        packetModel->addPacket(recPac);
        orchestrator->postPacket(recPac);
    }
}

BaseStateAbstract *PacketAnalyserTab::getStateMngtObj()
{
    PacketAnalyserTabStateObj *stateObj = new(std::nothrow) PacketAnalyserTabStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for PacketAnalyserTabStateObj X{");
    }

    return stateObj;
}

bool PacketAnalyserTab::canReceiveData()
{
    return false;
}

void PacketAnalyserTab::onImport()
{
    ImportExportDialog * ied = new(std::nothrow) ImportExportDialog(GuiConst::IMPORT_OPERATION, false ,this);
    if (ied == nullptr) {
        qFatal("Cannot allocate memory for ImportExportDialog X{");
    }

    if (ied->exec() == QDialog::Accepted) {
        if (ied->opGuiConfSelected()) {

            // we need to clear stuff first
            packetModel->clearUserColumns();

            int tabcount = ui->viewTabWidget->count();
            QList<QWidget *> list;
            for (int i = 0; i < tabcount; i++) {
                if (ui->viewTabWidget->indexOf(hexView) != i)
                    list.append(ui->viewTabWidget->widget(i));
            }
            for (int i = 0; i < list.size(); i++) {
                delete list.at(i);
            }
            list.clear();

            // then load the conf
            quint64 flags = 0;

            StateOrchestrator *stateOrchestrator = new(std::nothrow) StateOrchestrator(ied->getFileName(), flags);
            if (stateOrchestrator == nullptr) {
                qFatal("Cannot allocate memory for StateOrchestrator X{");
            }

            connect(stateOrchestrator, SIGNAL(finished()), this, SLOT(onSaveLoadFinished()));
            connect(stateOrchestrator, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)),
                    logger, SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)));

            setEnabled(false); // disabling the tab temporarly
            if (!stateOrchestrator->initialize()) {
                delete stateOrchestrator;
                stateOrchestrator = nullptr;
                setEnabled(true);
                return;
            }

            PacketAnalyserTabStateObj *stateObj = new(std::nothrow) PacketAnalyserTabStateObj(this);
            if (stateObj == nullptr) {
                qFatal("Cannot allocate memory for PacketAnalyserTabStateObj X{");
            }

            stateOrchestrator->addState(stateObj);
            stateOrchestrator->start();
        } else {
            ImportExportWorker *worker = new(std::nothrow) ImportExportWorker(packetModel,ied->getFileName(),GuiConst::IMPORT_OPERATION,ied->getFormat(),this);
            if (worker == nullptr) {
                qFatal("Cannot allocate memory for ImportExportWorker X{");
            }

            QtConcurrent::run(worker, &ImportExportWorker::run);
            ui->packetTableView->hideColumn(PacketModelAbstract::COLUMN_DIRECTION);
            ui->packetTableView->hideColumn(PacketModelAbstract::COLUMN_COMMENT);
            ui->packetTableView->hideColumn(PacketModelAbstract::COLUMN_CID);
        }
    }
}

void PacketAnalyserTab::onExport()
{
    ImportExportDialog * ied = new(std::nothrow) ImportExportDialog(GuiConst::EXPORT_OPERATION ,ui->packetTableView->selectionModel()->hasSelection(), this);
    if (ied == nullptr) {
        qFatal("Cannot allocate memory for ImportExportDialog X{");
    }

    if (ied->exec() == QDialog::Accepted) {
        if (ied->opGuiConfSelected()) {
            quint64 flags = GuiConst::STATE_SAVE_REQUEST;

            StateOrchestrator *stateOrchestrator = new(std::nothrow) StateOrchestrator(ied->getFileName(), flags);
            if (stateOrchestrator == nullptr) {
                qFatal("Cannot allocate memory for StateOrchestrator X{");
            }

            connect(stateOrchestrator, SIGNAL(finished()), this, SLOT(onSaveLoadFinished()));
            connect(stateOrchestrator, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)),
                    logger, SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)));

            setEnabled(false); // disabling the tab temporarly
            if (!stateOrchestrator->initialize()) {
                delete stateOrchestrator;
                stateOrchestrator = nullptr;
                setEnabled(true);
                return;
            }

            PacketAnalyserTabStateObj *stateObj = new(std::nothrow) PacketAnalyserTabStateObj(this);
            if (stateObj == nullptr) {
                qFatal("Cannot allocate memory for PacketAnalyserTabStateObj X{");
            }

            stateOrchestrator->addState(stateObj);
            stateOrchestrator->start();
        } else {
            ImportExportWorker *worker = new(std::nothrow) ImportExportWorker(packetModel, ied->getFileName(),GuiConst::EXPORT_OPERATION,ied->getFormat(),this);
            if (worker == nullptr) {
                qFatal("Cannot allocate memory for ImportExportWorker X{");
            }

            if (ied->getSelectionOnly()) {
                QList<qint64> selected;
                QModelIndexList indexList = ui->packetTableView->selectionModel()->selectedRows();
                for (int i = 0; i < indexList.size(); i++) {
                    selected.append(packetModel->indexToPacketIndex(indexList.at(i)));
                }

                if (selected.isEmpty()){
                    QMessageBox::warning(this, tr("No selection"),tr("No packet selected!"),QMessageBox::Ok);
                    delete ied;
                    return;
                } else {
                    qSort(selected);
                    worker->setFilteredList(selected);
                }
            }

            worker->setExportFormattedXML(exportFormattedXML);
            worker->setExportFormattedJson(exportFormattedJson);
            worker->setPcapLinkType((PcapDef::Link_Type)pcapLinkType);

            QtConcurrent::run(worker, &ImportExportWorker::run);
        }
    }

    delete ied;
}

void PacketAnalyserTab::onModelReseted()
{
    clearCurrentPacket();
}

void PacketAnalyserTab::clearCurrentPacket()
{
    if (bytesource != nullptr) {
        disconnect(bytesource, SIGNAL(updated(quintptr)), this, SLOT(onCurrentPacketUpdated(quintptr)));
        bytesource->clearAllMarkings();
        bytesource->clear();
        bytesource->historyClear();
    }
    currentPacket = nullptr;
}

void PacketAnalyserTab::copyAsUpdate()
{
    guiHelper->updateCopyContextMenu(copyAsMenu);
}

void PacketAnalyserTab::onSelectionChanged(const QItemSelection &selection)
{
    if (!selection.isEmpty()) {
        int index = selection.last().bottomRight().row();
        currentPacket = packetModel->getPacket(index);
        if (currentPacket != nullptr) {
            bytesource->setData(currentPacket->getData());
            connect(bytesource, SIGNAL(updated(quintptr)), this, SLOT(onCurrentPacketUpdated(quintptr)));
            sortFilterProxyModel->setSelectedPacket(index);

            checkIfOriginalTabNeeded();
        } else {
            clearCurrentPacket();
        }
    } else {
        clearCurrentPacket();
    }
}

void PacketAnalyserTab::onRightClick(QPoint pos)
{
    if (globalContextMenu != nullptr) {
        globalContextMenu->exec(this->mapToGlobal(pos));
    }
}

void PacketAnalyserTab::updateStatus()
{
    QString status = tr("<b>Packets:</b> %1").arg(packetModel->size());
    if (packetQueue.size() > 0) {
        status.append(tr(" | <b>Interception Queue:</b> %1").arg(packetQueue.size()));
    }
    ui->statusLabel->setText(status);
}

void PacketAnalyserTab::onOptionsClicked()
{
    if (optionsDialog == nullptr) {
        optionsDialog = new(std::nothrow) PacketAnalyserOptionsDialog(guiHelper, packetModel, sortFilterProxyModel , ui->packetTableView, this);
        if (optionsDialog == nullptr) {
            qFatal("Cannot allocate memory for PacketAnalyserOptionsDialog X{");
        }
        connect(optionsDialog, SIGNAL(rejected()), SLOT(onOptionDialogClosed()));

        optionsDialog->show();
    } else {
        optionsDialog->raise();
    }
}

void PacketAnalyserTab::onOptionDialogClosed()
{
    optionsDialog = nullptr;
}

void PacketAnalyserTab::setOrchestrator(SourcesOrchestatorAbstract *orch)
{
    if (orchestrator != nullptr){
        SourcesOrchestatorAbstract *old = orchestrator;
        orchestrator = nullptr;
        guiHelper->unregisterOrchestrator(old);
        old->deleteLater();
    }
    orchestrator = orch;
    if (orchestrator != nullptr) {
         setForwarding(orchestrator->isForwarder());

        ui->forwardPushButton->setEnabled(true);

        connect(orchestrator, SIGNAL(forwardingChanged(bool)), this, SLOT(orchestratorForwardingChanged(bool)));
        connect(orchestrator, SIGNAL(newPacket(Packet*)), this, SLOT(receiveNewPacket(Packet*)));
        connect(orchestrator, SIGNAL(connectionsChanged()), this, SLOT(onOrchestratorConnectionsChanged()));
        connect(orchestrator, SIGNAL(packetInjected(Packet*)), packetModel, SLOT(addPacket(Packet*)));
        connect(orchestrator, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), this, SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)));

        QWidget * controlWid = orchestrator->getControlGui(this);
        if (controlWid != nullptr) {
            ui->toolBarLayout->insertWidget(ui->toolBarLayout->indexOf(orchCombo) + 1,controlWid);
            connect(orchestrator, SIGNAL(guiRequested()), orchCombo, SLOT(showConfPanel()));
        }

        for (int i = 0 ; i < orchestrator->blockSourceCount(); i++) {
            BlocksSource * bs = orchestrator->getBlockSource(i);
            if (bs != nullptr) {
                connect(bs, SIGNAL(inboundTranformSelectionRequested()), this, SLOT(onInboundTransformRequested()));
                connect(bs, SIGNAL(outboundTranformSelectionRequested()), this, SLOT(onOutboundTransformRequested()));
            }
        }

        guiHelper->registerOrchestrator(orchestrator);

        if (orchestrator->getHasDirection())
            ui->packetTableView->showColumn(PacketModelAbstract::COLUMN_DIRECTION);
        else
            ui->packetTableView->hideColumn(PacketModelAbstract::COLUMN_DIRECTION);

        QTimer::singleShot(0, orchestrator, SLOT(start()));
    } else {
        ui->forwardPushButton->setEnabled(false);
    }
}

void PacketAnalyserTab::receiveNewPacket(Packet *packet)
{
    if (intercepting) {

        if (packetQueue.size() == 0) {
            packetModel->addPacket(packet);
            selectLastPacket();
        }
        packetQueue.enqueue(packet);
        updateTimer.start();

     } else {
        packetModel->addPacket(packet);

        if (trackingLast) {
            selectLastPacket();
        }

        if (forwarding && orchestrator != nullptr) {
            orchestrator->postPacket(packet);
        }
    }
}

void PacketAnalyserTab::orchestratorForwardingChanged(bool val)
{
    setForwarding(val);
}

void PacketAnalyserTab::onForwardPressed()
{
    if (orchestrator != nullptr) {
        if (intercepting) {
            if (!packetQueue.isEmpty()) { // sending the last packet
                Packet * pa = packetQueue.dequeue();
                orchestrator->postPacket(pa);
                if (!packetQueue.isEmpty()) {
                    packetModel->addPacket(packetQueue.head());
                    selectLastPacket();
                }
            }
        } else if (currentPacket != nullptr) {
            orchestrator->postPacket(currentPacket);
        }
    }
}

void PacketAnalyserTab::onInterceptClicked()
{
    setIntercepting(!intercepting);
}

void PacketAnalyserTab::onOrchestratorConnectionsChanged()
{
    if (orchestrator != nullptr) {
        emit nameChanged();
    }
}

void PacketAnalyserTab::onNewTabRequested()
{
    SingleViewAbstract * sva = tabHeaderViewsContextMenu->getView(bytesource,ui->viewTabWidget);
    if (sva != nullptr) {
        ViewTab data = tabHeaderViewsContextMenu->getTabData();
        ui->viewTabWidget->addTab(sva,data.tabName);
        tabData.append(data);
    }
}

void PacketAnalyserTab::addViewTab(TabAbstract::ViewTab data)
{
    tabHeaderViewsContextMenu->setTabData(data);
    onNewTabRequested();
}

void PacketAnalyserTab::onAddNewColumn()
{
    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (itemConfig == nullptr) {
        qFatal("Cannot allocate memory for QuickViewItemConfig X{");
    }
    itemConfig->setWayBoxVisible(true);
    itemConfig->setFormatVisible(true);
    itemConfig->setOutputTypeVisible(false);
    int ret = itemConfig->exec();
    if (ret == QDialog::Accepted) {
        TransformAbstract * ta = itemConfig->getTransform();
        if (ta != nullptr) {
            packetModel->addUserColumn(itemConfig->getName(), ta, itemConfig->getFormat());
        }
    }

    delete itemConfig;
}

void PacketAnalyserTab::onDeleteTab(int index)
{
    if (ui->viewTabWidget->indexOf(hexView) == index) {
        logger->logError(tr("Cannot close the main hexadecimal tab"),metaObject()->className());
    } else { // here we trust the index as it comes from the QTabWidget
        SingleViewAbstract * sva = static_cast<SingleViewAbstract *>(ui->viewTabWidget->widget(index));
        ViewTab vt;
        index--; // need to reduce the index to match the tabData index
        if (index < 0 || index >= tabData.size()) {
            qCritical() << tr("The index for Tabdata is out-of-bound T_T");
            return;
        } else {
            vt = tabData.takeAt(index);
        }
        delete sva;
    }
}

void PacketAnalyserTab::onScrollBarChanged(int min, int max)
{
    Q_UNUSED(min);
    if (trackingLast)
        ui->packetTableView->verticalScrollBar()->setValue(max);
}

void PacketAnalyserTab::onCurrentPacketUpdated(quintptr source)
{
    if (currentPacket != nullptr && source != (quintptr)this) {
        currentPacket->setData(bytesource->getRawData());
        checkIfOriginalTabNeeded();
    }
}

bool PacketAnalyserTab::eventFilter(QObject *receiver, QEvent *event)
{
    bool result = QObject::eventFilter(receiver, event);
#if QT_VERSION >= 0x050000
    if (receiver == ui->viewTabWidget) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[MainTabs::eventFilter] nullptr MouseEvent";
                return true; // not supposed to happen anyway ..
            } else if (me->buttons() == Qt::RightButton){ // only in the tabbar area
                QRect clickable = tabBarRef->geometry();
                clickable.setRight(geometry().right());
                if (clickable.contains(me->pos())) {
                    tabHeaderViewsContextMenu->exec(ui->viewTabWidget->mapToGlobal(me->pos()));
                    return true;
                }
            }
        }
    }
#endif
    return result;
}
bool PacketAnalyserTab::getIntercepting() const
{
    return intercepting;
}

void PacketAnalyserTab::setIntercepting(bool value)
{
    intercepting = value;
    if (intercepting) {
        ui->interceptPushButton->setIcon(QIcon(":/Images/icons/system-switch-user-3.png"));
    } else {
        ui->interceptPushButton->setIcon(QIcon(":/Images/icons/process-stop.png"));
    }
}


void PacketAnalyserTab::onContextMenuAction(QAction *action)
{
    if (action == merge) {
        QList<qint64> selected;
        QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();
        if (modelList.size() < 2) {
            QMessageBox::warning(this, tr("Not enough selection"),tr("Need to select at least two packets when merging"),QMessageBox::Ok);
            return;
        }

        for (int i = 0; i < modelList.size(); i++) {
            selected.append(packetModel->indexToPacketIndex(modelList.at(i)));
        }

        int nindex = packetModel->merge(selected);
        if (nindex != PacketModelAbstract::INVALID_POS) {
            ui->packetTableView->selectRow(nindex);
            Packet * pac = packetModel->getPacket(nindex);
            if (pac != nullptr) {
                bytesource->setData(pac->getData());
                sortFilterProxyModel->setSelectedPacket(nindex);
            } else {
                clearCurrentPacket();
            }
        }
    } else if (action == split) {
        qDebug() << "split requested";
        QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();
        if (modelList.size() > 1) {
            QMessageBox::warning(this, tr("Too many selected"),tr("You an only select one packet when splitting"),QMessageBox::Ok);
            return;
        }

        if (modelList.size()  < 1) {
            QMessageBox::warning(this, tr("No selection"),tr("Need to select one packet when splitting"),QMessageBox::Ok);
            return;
        }

    } else if (action == deleteSelection) {
        QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();
        if (modelList.size() < 1) {
            QMessageBox::warning(this, tr("No Selection"),tr("Need to select at least one packet for deletion"),QMessageBox::Ok);
            return;
        }

        int res = QMessageBox::warning(this, tr("Deletion requested"),tr("Are you sure you want to delete the selected packets?"), QMessageBox::Yes, QMessageBox::No);
        if (res == QMessageBox::Yes) {
            QList<qint64> selected;

            for (int i = 0; i < modelList.size(); i++) {
                selected.append(packetModel->indexToPacketIndex(modelList.at(i)));
            }
            ui->packetTableView->selectionModel()->clearSelection();
            packetModel->removePackets(selected);
            if (packetModel->size() == 0)
                clearCurrentPacket();
        }
    }
}

void PacketAnalyserTab::buildContextMenu()
{
    globalContextMenu = new(std::nothrow) QMenu(this);
    if (globalContextMenu == nullptr) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
        return;
    }

    sendToMenu = new(std::nothrow) SendToMenu(guiHelper, tr("Send selection to"));
    if (sendToMenu == nullptr) {
        qFatal("Cannot allocate memory for sendToMenu X{");
        return;
    }
    connect(sendToMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSendToTriggered(QAction*)), Qt::UniqueConnection);
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addSeparator();

    copyAsMenu = new(std::nothrow) QMenu(tr("Copy as"));
    if (copyAsMenu == nullptr) {
        qFatal("Cannot allocate memory for copyMenu X{");
        return;
    }
    connect(copyAsMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCopyAs(QAction*)), Qt::UniqueConnection);
    globalContextMenu->addMenu(copyAsMenu);
    globalContextMenu->addSeparator();

    merge = new(std::nothrow) QAction(tr("merge"), globalContextMenu);
    if (merge == nullptr) {
        qFatal("Cannot allocate memory for Merge X{");
        return;
    }
    globalContextMenu->addAction(merge);


    split = new(std::nothrow) QAction(tr("Split"), globalContextMenu);
    if (split == nullptr) {
        qFatal("Cannot allocate memory for Split X{");
        return;
    }

    globalContextMenu->addAction(split);

    deleteSelection = new(std::nothrow) QAction(tr("Delete"), globalContextMenu);
    if (deleteSelection == nullptr) {
        qFatal("Cannot allocate memory for Delete X{");
        return;
    }

    globalContextMenu->addAction(deleteSelection);

    connect(globalContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(onContextMenuAction(QAction*)), Qt::UniqueConnection);
}
void PacketAnalyserTab::setTrackingLast(bool value)
{
    trackingLast = value;
}

void PacketAnalyserTab::onSendToTriggered(QAction *action)
{
    QByteArray selectedData;
    QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();

    for (int i = 0; i < modelList.size(); i++) {
        selectedData.append(packetModel->getPacket(packetModel->indexToPacketIndex(modelList.at(i)))->getData());
    }
    sendToMenu->processingAction(action, selectedData);
}

void PacketAnalyserTab::onClearListClicked()
{
    int res = QMessageBox::warning(this, tr("Clearing requested"),tr("Are you sure you want to clear all the packets?"), QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        packetModel->clear();
    }
}

void PacketAnalyserTab::onCopyAs(QAction *action)
{
    QByteArray selectedData;
    QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();

    for (int i = 0; i < modelList.size(); i++) {
        selectedData.append(packetModel->getPacket(packetModel->indexToPacketIndex(modelList.at(i)))->getData());
    }
    guiHelper->copyAction(action->text(), selectedData);
}

void PacketAnalyserTab::checkIfOriginalTabNeeded()
{
    if (currentPacket != nullptr && currentPacket->hasBeenModified()) {
        oriBytesource->setReadOnly(false);
        oriBytesource->setData(currentPacket->getOriginalData());
        oriBytesource->setReadOnly(true);
        int index = ui->viewTabWidget->indexOf(hexView) + 1;
        ui->viewTabWidget->insertTab(index,oriHexView, GuiConst::ORIGINAL_DATA_STR );
        removeTabButton(index);
    } else {
        int index = ui->viewTabWidget->indexOf(oriHexView);
        if (index != -1)
            ui->viewTabWidget->removeTab(index);
    }
}

void PacketAnalyserTab::removeTabButton(int index)
{
#if QT_VERSION >= 0x050000
        tabBarRef = ui->viewTabWidget->tabBar();
        tabBarRef->setTabButton(index, QTabBar::RightSide,nullptr);
        tabBarRef->setTabButton(index, QTabBar::LeftSide, nullptr);
#endif
}

bool PacketAnalyserTab::getForwarding() const
{
    return forwarding;
}

void PacketAnalyserTab::setForwarding(bool value)
{
    forwarding = value;
}

void PacketAnalyserTab::onMergeRequested()
{
    packetModel->mergeConsecutivePackets();
    selectLastPacket();
}

void PacketAnalyserTab::onInboundTransformRequested()
{
    BlocksSource * bs = static_cast<BlocksSource *>(sender());
    if (bs != nullptr) {
        QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
        if (itemConfig == nullptr) {
            qFatal("Cannot allocate memory for QuickViewItemConfig X{");
        }
        itemConfig->setWayBoxVisible(true);
        itemConfig->setFormatVisible(false);
        itemConfig->setOutputTypeVisible(false);
        int ret = itemConfig->exec();
        if (ret == QDialog::Accepted) {
            TransformAbstract * ta = itemConfig->getTransform();
            if (ta != nullptr) {
                bs->setInboundTranform(ta);
            }
        }

        delete itemConfig;
    } else {
        qCritical() << tr("[PacketAnalyserTab::inboundTransformRequested] cast is null pointer T_T");
    }
}

void PacketAnalyserTab::onOutboundTransformRequested()
{
    BlocksSource * bs = static_cast<BlocksSource *>(sender());
    if (bs != nullptr) {
        QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
        if (itemConfig == nullptr) {
            qFatal("Cannot allocate memory for QuickViewItemConfig X{");
        }
        itemConfig->setWayBoxVisible(true);
        itemConfig->setFormatVisible(false);
        itemConfig->setOutputTypeVisible(false);
        int ret = itemConfig->exec();
        if (ret == QDialog::Accepted) {
            TransformAbstract * ta = itemConfig->getTransform();
            if (ta != nullptr) {
                bs->setOutboundTranform(ta);
            }
        }

        delete itemConfig;
    } else {
        qCritical() << tr("[PacketAnalyserTab::inboundTransformRequested] cast is null pointer T_T");
    }
}

void PacketAnalyserTab::onSaveLoadFinished()
{
    setEnabled(true);
}

void PacketAnalyserTab::logMessage(const QString &message, const QString &, LOGLEVEL level)
{
    if (level == Pip3lineConst::LERROR) {
        QPalette palette = ui->messagesLabel->palette();
        palette.setColor(ui->messagesLabel->foregroundRole(), Qt::red);

        ui->messagesLabel->setPalette(palette);
        // print message in the GUI
        ui->messagesLabel->setText(message);
    }
}

void PacketAnalyserTab::selectLastPacket()
{
    ui->packetTableView->selectionModel()->select(packetModel->getLastPacketRow(),QItemSelectionModel::ClearAndSelect);
    if (ui->packetTableView->verticalScrollBar()->isVisible()) {
        ui->packetTableView->verticalScrollBar()->setValue( ui->packetTableView->verticalScrollBar()->maximum());
    }
}

PacketAnalyserTabStateObj::PacketAnalyserTabStateObj(PacketAnalyserTab *tab) :
    TabStateObj(tab)
{
    setName(metaObject()->className());
}

PacketAnalyserTabStateObj::~PacketAnalyserTabStateObj()
{

}

void PacketAnalyserTabStateObj::run()
{
    PacketAnalyserTab * gTab = dynamic_cast<PacketAnalyserTab *> (tab);
    TabStateObj::run();
    ImportExportWorker worker(gTab->packetModel);
    worker.setExportFormattedXML(false);
    QHash<QString, QString> conf;
    SourcesOrchestatorAbstract * orch = nullptr;
    BlocksSource * bs = nullptr;

    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        // saving orchestrator conf
        writer->writeStartElement(GuiConst::STATE_ORCHESTRATOR);
        orch = gTab->orchestrator;
        if (orch != nullptr) {
            writer->writeAttribute(GuiConst::STATE_TYPE, QString::number(orch->getType()));
            conf = orch->getConfiguration();
            QHashIterator<QString, QString> i(conf);
            while (i.hasNext()) {
                i.next();
                writer->writeAttribute(i.key(), i.value());
            }

            for (int i = 0; i < orch->blockSourceCount(); i++) {
                writer->writeStartElement(GuiConst::STATE_BLOCKSOURCE);
                bs = orch->getBlockSource(i);
                conf = bs->getConfiguration();
                QHashIterator<QString, QString> j(conf);
                while (j.hasNext()) {
                    j.next();
                    writer->writeAttribute(j.key(), j.value());
                }
                writer->writeEndElement(); // STATE_BLOCKSOURCE
            }
        } else {
            writer->writeAttribute(GuiConst::STATE_TYPE, QString::number(SourcesOrchestatorAbstract::INVALID_TYPE));
        }

        writer->writeEndElement(); // STATE_ORCHESTRATOR
        //saving column config
        int count = gTab->packetModel->columnCount();
        if ( count > 0) {
            writer->writeStartElement(GuiConst::STATE_COLUMNS_CONF);
            for (int i = 0; i < count; i++) {
                writer->writeStartElement(GuiConst::STATE_COLUMN);
                writer->writeAttribute(GuiConst::STATE_MODEL_INDEX, QString::number(i));
                writer->writeAttribute(GuiConst::STATE_IS_HIDDEN, QString::number(gTab->ui->packetTableView->isColumnHidden(i) ? 1 : 0));
                writer->writeAttribute(GuiConst::STATE_VIEW_INDEX, QString::number(gTab->ui->packetTableView->horizontalHeader()->visualIndex(i)));
                if (gTab->packetModel->isUserColumn(i)) {
                    TransformAbstract *ta = gTab->packetModel->getTransform(i);
                    if (ta != nullptr) {
                        QString confText;
                        TransformChain list;
                        list.append(ta);
                        list.setName(gTab->packetModel->getColumnName(i));
                        list.setFormat(gTab->packetModel->getColumnFormat(i));
                        QXmlStreamWriter streamin(&confText);
                        gTab->guiHelper->getTransformFactory()->saveConfToXML(list, &streamin);
                        writer->writeAttribute(GuiConst::STATE_NAME, gTab->packetModel->getColumnName(i));
                        writer->writeAttribute(GuiConst::STATE_CONF, QString::fromUtf8(confText.toUtf8().toBase64()));
                    }
                }
                writer->writeEndElement(); //STATE_COLUMN
            }
            writer->writeEndElement(); //STATE_COLUMNS_CONF
        }

        if (flags & GuiConst::STATE_LOADSAVE_DATA) // saving packets only if enabled
            worker.toXML(writer);

        // saving view tabs
        QList<TabAbstract::ViewTab> tabs = gTab->tabData;
        int size = tabs.size();
        writer->writeStartElement(GuiConst::STATE_TABVIEWLIST);
        writer->writeAttribute(GuiConst::STATE_SIZE, write(tabs.size()));
        for (int i = 0; i < size ; i++) {
            writer->writeStartElement(GuiConst::STATE_TABVIEW);
            writer->writeAttribute(GuiConst::STATE_TYPE, write((int)tabs.at(i).type));
            // saving configuration
            QString conf;
            TransformChain list;
            if (tabs.at(i).transform != nullptr) {
                list.append(tabs.at(i).transform);
            }
            list.setName(tabs.at(i).tabName);
            QXmlStreamWriter streamin(&conf);
            gTab->guiHelper->getTransformFactory()->saveConfToXML(list, &streamin);
            writer->writeAttribute(GuiConst::STATE_CONF, write(conf));
            writer->writeEndElement(); // STATE_TABVIEW
            list.clear(); // the transforms are not own by us, don't delete them
        }
        writer->writeEndElement(); // STATE_TABVIEWLIST

    } else {
        connect(&worker, SIGNAL(newPacket(Packet*)),gTab->packetModel, SLOT(addPacket(Packet*)));

        QMap<int, int> visualindexes;
        if (reader->readNextStartElement()) {
            qDebug() << "trying to read the orchestrator configuration" << reader->name().toString();
            if (reader->name() == GuiConst::STATE_ORCHESTRATOR) {
                // setting Orchestrator configuration
                QXmlStreamAttributes attributes = reader->attributes();
                conf.clear();
                for (int i = 0; i < attributes.size(); i++) {
                    QXmlStreamAttribute attr = attributes.at(i);
                    if (attr.name() == GuiConst::STATE_TYPE) {
                        bool ok = false;
                        int type = attr.value().toString().toInt(&ok);
                        if (!ok) {
                            qCritical() << tr("[PacketAnalyserTabStateObj] Cannot parse the type of the orchestrator: %1").arg(attr.name().toString());
                        } else {
                            gTab->orchCombo->setType(type);
                        }
                    } else {
                        conf.insert(attr.name().toString(), attr.value().toString());
                    }
                }
                orch = gTab->orchCombo->getOrchestrator();
                if (orch != nullptr) {
                    orch->setConfiguration(conf);
                    // settings the different blocksources configuration
                    int index = 0;
                    while (reader->readNext() != QXmlStreamReader::EndElement) {
                        if (reader->name() == GuiConst::STATE_BLOCKSOURCE) {
                            QXmlStreamAttributes attributes = reader->attributes();
                            conf.clear();
                            for (int i = 0; i < attributes.size(); i++) {
                                QXmlStreamAttribute attr = attributes.at(i);
                                conf.insert(attr.name().toString(), attr.value().toString());
                            }

                            bs = orch->getBlockSource(index);
                            if (bs != nullptr) {
                                bs->setConfiguration(conf);
                            } else {
                                qCritical() << tr("[PacketAnalyserTabStateObj] blocksource is nullptr T_T");
                            }

                            index++;

                            // read closing GuiConst::STATE_BLOCKSOURCE
                            if (!(reader->readNext() == QXmlStreamReader::EndElement && reader->name() == GuiConst::STATE_BLOCKSOURCE)) {
                                qCritical() << tr("[PacketAnalyserTabStateObj] Expected a closing tag for %1 T_T").arg(GuiConst::STATE_BLOCKSOURCE);
                            }

                        } else {
                            qCritical() << tr("[PacketAnalyserTabStateObj] Invalid token name, was expecting %1, got %2").arg(GuiConst::STATE_BLOCKSOURCE).arg(reader->name().toString());
                        }
                    }
                }
                else
                    qCritical() << tr("[PacketAnalyserTabStateObj] orchestrator is nullptr T_T");

                reader->readNext(); // read next token
            }

            if (reader->name() == GuiConst::STATE_COLUMNS_CONF) {
                while(reader->readNextStartElement()) {
                    QXmlStreamAttributes attributes = reader->attributes();
                    if (attributes.hasAttribute(GuiConst::STATE_MODEL_INDEX)) {
                        bool ok = false;
                        int index = attributes.value(GuiConst::STATE_MODEL_INDEX).toString().toInt(&ok);
                        if (!ok) {
                            qCritical() << tr("[PacketAnalyserTabStateObj] Cannot parse the index of the column: %1").arg(attributes.value(GuiConst::STATE_MODEL_INDEX).toString());
                        }
                        else {
                            if (!gTab->packetModel->isDefaultColumn(index)) {
                                QString name;
                                TransformAbstract *ta = nullptr;
                                Pip3lineConst::OutputFormat format = Pip3lineConst::TEXTFORMAT;
                                if (attributes.hasAttribute(GuiConst::STATE_NAME)) {
                                    name = attributes.value(GuiConst::STATE_NAME).toString();
                                }

                                if (attributes.hasAttribute(GuiConst::STATE_CONF)) {
                                    QString confText = attributes.value(GuiConst::STATE_CONF).toString();
                                    confText = QString::fromUtf8(QByteArray::fromBase64(confText.toUtf8()));
                                    if (confText.isEmpty()) {
                                        qCritical() << tr("[PacketAnalyserTabStateObj] Transform conf for index is empty: %1").arg(index);
                                    } else {
                                        QXmlStreamReader reader(confText);
                                        TransformChain talist = gTab->guiHelper->getTransformFactory()->loadConfFromXML(&reader);
                                        if (!talist.isEmpty()) {
                                            ta = talist.takeFirst();
                                            if (talist.size() > 0) {
                                                // dont'care about the rest
                                                while (!talist.isEmpty()) {
                                                    delete talist.takeLast();
                                                }
                                            }
                                            format = talist.getFormat();
                                        } else {

                                        }
                                    }
                                }
                                if (!name.isEmpty()) {
                                    gTab->packetModel->addUserColumn(name,ta,format);
                                } else { // otherwise we just ignore it
                                    delete ta;
                                    continue; //main while loop
                                }
                            }
                            // in any case we need the order
                            int tempint = 0;
                            if (attributes.hasAttribute(GuiConst::STATE_IS_HIDDEN)) {
                                tempint = attributes.value(GuiConst::STATE_IS_HIDDEN).toString().toInt(&ok);
                                if (ok) {
                                    gTab->ui->packetTableView->setColumnHidden(index, tempint == 1);
                                }
                            }

                            if (attributes.hasAttribute(GuiConst::STATE_VIEW_INDEX)) {
                                tempint = attributes.value(GuiConst::STATE_VIEW_INDEX).toString().toInt(&ok);
                                if (ok) {
                                    visualindexes.insert(index, tempint);
                                }
                            }
                        }
                    }
                    while (reader->readNext() != QXmlStreamReader::EndElement); // reading end element
                }
                reader->readNext();
            }
        }

        if (flags & GuiConst::STATE_LOADSAVE_DATA) // load data only if requested
            worker.loadFromXML(reader);

        QMapIterator<int, int> i(visualindexes);
         while (i.hasNext()) {
             i.next();
             int curVisualindex = gTab->ui->packetTableView->horizontalHeader()->visualIndex(i.key());
             if (i.value() != curVisualindex) {
                gTab->ui->packetTableView->horizontalHeader()->swapSections(i.value(),curVisualindex);
             }
         }

        // restoring view tabs
        if (reader->name() == GuiConst::STATE_TABVIEWLIST || readNextStart(GuiConst::STATE_TABVIEWLIST)) {
            QXmlStreamAttributes attrList = reader->attributes();
            if (attrList.hasAttribute(GuiConst::STATE_SIZE)) {
                bool ok = false;
                int size = readInt(attrList.value(GuiConst::STATE_SIZE),&ok);
                if (ok) {
                    for (int i = 0; i < size ; i++) {
                        TabAbstract::ViewTab vt;
                        vt.transform = nullptr; // just initialising in case of screw up
                        if (readNextStart(GuiConst::STATE_TABVIEW)) {
                            attrList = reader->attributes();
                            readEndElement(GuiConst::STATE_TABVIEW); // closing now, because there is no child defined anyway
                            if (attrList.hasAttribute(GuiConst::STATE_TYPE)) {
                                int type = readInt(attrList.value(GuiConst::STATE_TYPE),&ok);
                                if (ok && (type == 0 || type == 1 || type == 2 || type == 3)) {
                                    vt.type = (TabAbstract::ViewType) type;
                                } else {
                                    qWarning() << "Invalid state type for this view, skipping";
                                    continue;
                                }
                            } else {
                                qWarning() << "no state type for this view, skipping";
                                continue;
                            }
                            if (vt.type != TabAbstract::DEFAULTTEXT) {
                                if (attrList.hasAttribute(GuiConst::STATE_CONF)) {
                                    QString conf = readString(attrList.value(GuiConst::STATE_CONF));
                                    if (!conf.isEmpty()) {
                                        TransformChain list = gTab->guiHelper->getTransformFactory()->loadConfFromXML(conf);
                                        if (!list.isEmpty()) {
                                            vt.transform = list.at(0);
                                            vt.tabName = list.getName();
                                        } else {
                                            qWarning() << "Resulting transform list empty for this view, skipping";
                                            continue;
                                        }
                                    } else {
                                        qWarning() << "Empty conf for this view, skipping";
                                        continue;
                                    }
                                } else {
                                    qWarning() << "no conf for this view, skipping";
                                    continue;
                                }
                            } else {
                                vt.tabName = NewViewButton::TEXT_TEXT;
                            }
                            gTab->addViewTab(vt);
                            // no deleting vt.transform here, the pointer is now owned by the tab.
                        }
                    }
                } else {
                    qCritical() << "Size value is not a number, giving up";
                }
            } else {
                 qCritical() << "No size value for the view size, giving up";
            }
        }
        readEndElement(GuiConst::STATE_TABVIEWLIST);
    }

//    PacketAnalyserTabClosingStateObj *closingState = new(std::nothrow) PacketAnalyserTabClosingStateObj(gTab);
//    if (closingState == nullptr) {
//        qFatal("Cannot allocate memory for PacketAnalyserTabClosingStateObj X{");
//    };

//    emit addNewState(closingState);
}


PacketAnalyserTabClosingStateObj::PacketAnalyserTabClosingStateObj(PacketAnalyserTab *tab) :
    tab(tab)
{
    name = metaObject()->className();
}

PacketAnalyserTabClosingStateObj::~PacketAnalyserTabClosingStateObj()
{

}

void PacketAnalyserTabClosingStateObj::run()
{

}

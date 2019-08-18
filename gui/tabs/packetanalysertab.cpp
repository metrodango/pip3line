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
#include <QtConcurrent>
#include <QtConcurrentRun>
#include <transformchain.h>
#include <QItemSelectionModel>
#include <QPalette>
#include <QMessageBox>
#include <QHashIterator>
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
#include <QXmlStreamAttribute>
#include <QXmlStreamAttributes>
#include <threadedprocessor.h>
#include <transformmgmt.h>
#include <commonstrings.h>
#include <QScrollBar>
#include <QDialog>
#include <QAction>
#include <QTimer>
#include <QColorDialog>
#include "guihelper.h"
#include "pcapio/pcapdef.h"
#include "sources/blocksources/blockssource.h"
#include "packetanalyser/orchestratorchooser.h"
#include "shared/newviewbutton.h"
#include "shared/newviewmenu.h"
#include "packetanalysertab.h"
#include "quickviewitemconfig.h"
#include "shared/sendtomenu.h"
#include "textinputdialog.h"
#include "packetanalyser/filterdialog.h"
#include <state/stateorchestrator.h>
#include <sources/intermediatesource.h>

PacketAnalyserTab::PacketAnalyserTab(GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper, parent),
    ui(new Ui::PacketAnalyserTab),
    forwarding(false),
    trackingLast(true),
    intercepting(false),
    trackChanges(false)
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
    sendPacketsMenu = nullptr;
    sendPacketsToNew = nullptr;
    highlightMenu = nullptr;
    newHighlight = nullptr;

    optionsDialog = nullptr;
    filterDialog = nullptr;
    tabBarRef = nullptr;
    headerContextMenu = nullptr;
    deleteColumn = nullptr;
    tabHeaderViewsContextMenu = new(std::nothrow) NewViewMenu(guiHelper, this);
    if (tabHeaderViewsContextMenu == nullptr) {
        qFatal("Cannot allocate memory for NewViewMenu X{");
    }

    connect(tabHeaderViewsContextMenu, &NewViewMenu::newViewRequested, this, &PacketAnalyserTab::onNewTabRequested);

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
    buildHeadersContextMenu();
    updateHighlightMenu();

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
    ui->packetTableView->verticalHeader()->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    ui->packetTableView->horizontalHeader()->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_CID,packetModel->getDefaultWidthForColumn(PacketModelAbstract::COLUMN_CID));
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_LENGTH,packetModel->getDefaultWidthForColumn(PacketModelAbstract::COLUMN_LENGTH));
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_DIRECTION,packetModel->getDefaultWidthForColumn(PacketModelAbstract::COLUMN_DIRECTION));
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_TIMESPTAMP,packetModel->getDefaultWidthForColumn(PacketModelAbstract::COLUMN_TIMESPTAMP));
    ui->packetTableView->setColumnWidth(PacketModelAbstract::COLUMN_PAYLOAD,packetModel->getDefaultWidthForColumn(PacketModelAbstract::COLUMN_PAYLOAD));
    ui->packetTableView->verticalHeader()->setDefaultSectionSize(20);

    ui->packetTableView->horizontalHeader()->setSectionsMovable( true );
    ui->packetTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->packetTableView->setSortingEnabled(true);
    ui->packetTableView->verticalHeader()->setDefaultSectionSize(GlobalsValues::ROWSHEIGHT);

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
    ui->viewTabWidget->tabBar()->installEventFilter(this);

    orchCombo = new(std::nothrow) OrchestratorChooser(guiHelper);
    if (orchCombo == nullptr) {
        qFatal("Cannot allocate memory for OrchestratorChooser X{");
    }

    ui->toolBarLayout->insertWidget(ui->toolBarLayout->indexOf(ui->optionsPushButton) + 2, orchCombo);

    ui->trackPushButton->setChecked(trackingLast);

    setIntercepting(false);

    autoMergeMenu = new(std::nothrow) QMenu();
    if (autoMergeMenu == nullptr) {
        qFatal("Cannot allocate memory for QMenu X{");
    }

    ui->actionAuto_merge_packets->setChecked(packetModel->isAutoMergeConsecutivePackets());
    autoMergeMenu->addAction(ui->actionAuto_merge_packets);
    autoMergeMenu->addAction(ui->actionMergePacketsNow);

    ui->mergePacketsPushButton->setMenu(autoMergeMenu);

    // connecting stuff
    connect(bytesource, &ByteSourceAbstract::log, logger, &LoggerWidget::logMessage,Qt::QueuedConnection);
    connect(bytesource, &ByteSourceAbstract::updated, this, &PacketAnalyserTab::onByteSourceUpdated, Qt::UniqueConnection);
    connect(ui->packetTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PacketAnalyserTab::onSelectionChanged);
    connect(ui->packetTableView,&QTableView::customContextMenuRequested, this, &PacketAnalyserTab::onRightClick);
    connect(packetModel, &PacketModelAbstract::sendRequest, guiHelper->getCentralTransProc(), &ThreadedProcessor::processRequest, Qt::QueuedConnection);
    connect(packetModel, &PacketModelAbstract::log, guiHelper->getLogger(), &LoggerWidget::logMessage, Qt::QueuedConnection);
    connect(packetModel, &PacketModelAbstract::modelReset, this, &PacketAnalyserTab::onModelReseted);
    //connect(packetModel, &PacketModelAbstract::updated , &updateTimer, qOverload<>(&QTimer::start));
    connect(packetModel, SIGNAL(updated()), &updateTimer, SLOT(start()));
    connect(ui->actionMergePacketsNow, &QAction::triggered,this, &PacketAnalyserTab::onAutoMergeRequested);
    connect(ui->actionAuto_merge_packets, &QAction::toggled, packetModel, &PacketModelAbstract::setAutoMergeConsecutivePackets);
    connect(ui->clearPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onClearListClicked);
    connect(orchCombo, &OrchestratorChooser::newOrchestrator, this, &PacketAnalyserTab::setOrchestrator);

    connect(ui->viewTabWidget, &QTabWidget::tabCloseRequested,this, &PacketAnalyserTab::onDeleteTab);

    connect(ui->importPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onImport);
    connect(ui->exportPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onExport);
    connect(ui->optionsPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onOptionsClicked);
    connect(ui->refreshPushButton, &QPushButton::clicked, packetModel, &PacketModelAbstract::refreshAllColumn);
    connect(ui->forwardPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onForwardPressed);
    connect(ui->addColumnPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onAddNewColumn);
    ui->addViewPushButton->setMenu(tabHeaderViewsContextMenu);

    connect(ui->packetTableView->verticalScrollBar(), &QScrollBar::rangeChanged, this, &PacketAnalyserTab::onScrollBarChanged);
    connect(ui->packetTableView, &QTableView::clicked, this, &PacketAnalyserTab::onItemClicked);
    connect(&updateTimer, &QTimer::timeout, this, &PacketAnalyserTab::updateStatus);

    connect(ui->interceptPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onInterceptClicked);
    connect(ui->trackPushButton, &QPushButton::toggled, this, &PacketAnalyserTab::setTrackingLast);
    connect(ui->trackChangesPushButton, &QPushButton::toggled, this, &PacketAnalyserTab::onTrackingToggled);
    connect(guiHelper, &GuiHelper::importExportUpdated, this, &PacketAnalyserTab::updateCopyAsMenu);
    connect(ui->filterPushButton, &QPushButton::clicked, this, &PacketAnalyserTab::onFilter);
    connect(guiHelper, &GuiHelper::hexTableSizesUpdated, this, &PacketAnalyserTab::onFontUpdated);
    connect(guiHelper, &GuiHelper::registeredPacketsAnalysersUpdated, this, &PacketAnalyserTab::sendPacketsToUpdate, Qt::QueuedConnection);

    connect(guiHelper, &GuiHelper::importExportUpdated, this, &PacketAnalyserTab::updateCopyAsMenu);
    connect(guiHelper, &GuiHelper::markingsUpdated, this, &PacketAnalyserTab::updateHighlightMenu);
}

PacketAnalyserTab::~PacketAnalyserTab()
{
    delete autoMergeMenu;
    delete copyAsMenu;
    delete sendToMenu;
    delete highlightMenu;
    delete globalContextMenu;
    delete tabHeaderViewsContextMenu;
    delete orchCombo;
    delete sortFilterProxyModel;
    delete styleDelegate;
    delete packetModel;
    delete detachButton;
    delete sendPacketsMenu;

    delete ui;

    if (orchestrator != nullptr) {
        guiHelper->unregisterOrchestrator(orchestrator);
        delete orchestrator;
    }

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
        QSharedPointer<Packet> recPac = QSharedPointer<Packet> (new(std::nothrow) Packet(data));
        if (recPac == nullptr)
            qFatal("Cannot allocate memory for Packet X{");

        packetModel->addPacket(recPac);
        orchestrator->postPacket(recPac);
    }
}

void PacketAnalyserTab::addPackets(const QList<QSharedPointer<Packet> > &packets)
{
    packetModel->addPackets(packets);
    // not sure if we should post all the packets as well ... we'll see if the case comes up
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

            StateOrchestrator *stateOrchestrator = new(std::nothrow) StateOrchestrator(ied->getFileName(),
                                                                                       flags);
            if (stateOrchestrator == nullptr) {
                qFatal("Cannot allocate memory for StateOrchestrator X{");
            }

            connect(stateOrchestrator, &StateOrchestrator::finished, this, &PacketAnalyserTab::onSaveLoadFinished);
            connect(stateOrchestrator, &StateOrchestrator::log, logger, &LoggerWidget::logMessage);

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
            ImportExportWorker *worker = new(std::nothrow) ImportExportWorker(packetModel,
                                                                              ied->getFileName(),
                                                                              GuiConst::IMPORT_OPERATION,
                                                                              ied->getFormat(),
                                                                              ied->getEnableCompression()
                                                                              ,this);
            if (worker == nullptr) {
                qFatal("Cannot allocate memory for ImportExportWorker X{");
            }

            worker->setPlainBase64(ied->isPlainBase64());
            worker->setPlainToFile(ied->isPlainFile());

            connect(worker,&ImportExportWorker::log, this, &PacketAnalyserTab::logMessage, Qt::QueuedConnection);

            QtConcurrent::run(worker, &ImportExportWorker::run);
            // only hide for pcap format, as packets do not have the "direction" property in this case
            ui->packetTableView->setColumnHidden(PacketModelAbstract::COLUMN_DIRECTION, ied->getFormat() == GuiConst::PCAP_FORMAT);

            // timestamps are arbitrary in this mode, very little need to show them
            ui->packetTableView->setColumnHidden(PacketModelAbstract::COLUMN_TIMESPTAMP, ied->getFormat() == GuiConst::PLAIN_FORMAT);

            // should not be necessary, but just in case
            // ui->packetTableView->setColumnHidden(PacketModelAbstract::COLUMN_CID, true);
            // ui->packetTableView->setColumnHidden(PacketModelAbstract::COLUMN_COMMENT, true);
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

            connect(stateOrchestrator, &StateOrchestrator::finished, this, &PacketAnalyserTab::onSaveLoadFinished);
            connect(stateOrchestrator, &StateOrchestrator::log, logger, &LoggerWidget::logMessage);

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
            ImportExportWorker *worker = new(std::nothrow) ImportExportWorker(packetModel,
                                                                              ied->getFileName(),
                                                                              GuiConst::EXPORT_OPERATION,
                                                                              ied->getFormat(),
                                                                              ied->getEnableCompression(),
                                                                              this);
            if (worker == nullptr) {
                qFatal("Cannot allocate memory for ImportExportWorker X{");
            }

            worker->setPlainBase64(ied->isPlainBase64());
            worker->setPlainToFile(ied->isPlainFile());

            if (ied->getSelectionOnly()) {
                QList<qint64> selected;
                QModelIndexList indexList = ui->packetTableView->selectionModel()->selectedRows();
                for (int i = 0; i < indexList.size(); i++) {
                    selected.append(sortFilterProxyModel->indexToPacketIndex(indexList.at(i)));
                }

                if (selected.isEmpty()){
                    QMessageBox::warning(this, tr("No selection"),tr("No packet selected!"),QMessageBox::Ok);
                    delete ied;
                    return;
                } else {
                    std::sort(selected.begin(),selected.end(), std::greater<qint64>());
                    worker->setFilteredList(selected);
                }
            }

            worker->setExportFormattedXML(exportFormattedXML);
            worker->setExportFormattedJson(exportFormattedJson);
            worker->setPcapLinkType(static_cast<PcapDef::Link_Type>(pcapLinkType));

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
    currentPacket.clear(); // in case this was not already done
    bytesource->clearAllMarkings();
    bytesource->clear();
    bytesource->historyClear();
    sortFilterProxyModel->setSelectedPacket(PacketModelAbstract::INVALID_POS);
}

void PacketAnalyserTab::updateCopyAsMenu()
{
    guiHelper->updateCopyContextMenu(copyAsMenu);
}

void PacketAnalyserTab::sendPacketsToUpdate()
{
    sendPacketsMenu->clear();
    sendPacketsToNew = new(std::nothrow) QAction(tr("New Packets Analyzer"), sendPacketsMenu);
    if (sendPacketsToNew == nullptr) {
        qFatal("Cannot allocate memory for sendPacketsToNew X{");
    }

    sendPacketsMenu->addAction(sendPacketsToNew);
    sendPacketsMenu->addSeparator();

    QList<PacketAnalyserTab *> list = guiHelper->getRegisteredPacketsAnalysers();
    list.removeAll(this); // removing oneself
    for (int i = 0; i < list.size(); i++) {
        PacketAnalyserTab * tab = list.at(i);
        QAction * action = new(std::nothrow) QAction(tab->getName(), sendPacketsMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action sendPacketsToUpdate X{");
        }
        action->setData(QVariant(i));
        sendPacketsMenu->addAction(action);
    }
}

void PacketAnalyserTab::onSelectionChanged(const QItemSelection &selection)
{
   // qDebug() << tr("selection changed");
    if (!selection.isEmpty()) {
        QModelIndex last_i = selection.last().bottomRight();
        if (last_i.isValid()) {
            QModelIndex last = sortFilterProxyModel->mapToSource(last_i);
            if (last.isValid()) {
                int index = last.row();
                currentPacket = packetModel->getPacket(index);
                if (!currentPacket.isNull()) {
                    bytesource->setData(currentPacket->getData(),reinterpret_cast<quintptr>(this));
                    sortFilterProxyModel->setSelectedPacket(index);
                    checkIfOriginalTabNeeded();

                    if (trackChanges && index > 0) {
                        bytesource->clearAllMarkings();
                        QSharedPointer<Packet> previous = packetModel->getPreviousPacket(index, currentPacket->getDirection());
                        if (!previous.isNull()) {
                            BytesRangeList * ranges = new(std::nothrow) BytesRangeList();
                            if (ranges == nullptr) {
                                qFatal("Cannot allocate memory for BytesRangeList X{");
                            }
                            BytesRange::compareAndMark(bytesource->getRawData(), previous->getData(), ranges, GuiStyles::DEFAULT_MARKING_COLOR, QColor(), GuiConst::MODIFIED_STR);

                            if (!ranges->isEmpty()) {
                                bytesource->setNewMarkings(ranges);
                            } else {
                                delete ranges;
                            }
                        }
                    }
                    return; // nofail
                }
            }
        }
    }

    // if there is a fail
    clearCurrentPacket();
}

void PacketAnalyserTab::onRightClick(QPoint pos)
{
    if (globalContextMenu != nullptr) {
        QModelIndexList selectList = ui->packetTableView->selectionModel()->selectedRows();
        if (selectList.isEmpty()) { // 0
            merge->setEnabled(false);
            split->setEnabled(false);
            deleteSelection->setEnabled(false);
            copyAsMenu->setEnabled(false);
            sendToMenu->setEnabled(false);
        } else if (selectList.size() == 1) { //1
            merge->setEnabled(false);
            deleteSelection->setEnabled(true);
            split->setEnabled(true);
            copyAsMenu->setEnabled(true);
            sendToMenu->setEnabled(true);
        } else { // >1
            merge->setEnabled(true);
            split->setEnabled(false);
            deleteSelection->setEnabled(true);
            copyAsMenu->setEnabled(true);
            sendToMenu->setEnabled(true);
        }
        globalContextMenu->exec(ui->packetTableView->viewport()->mapToGlobal(pos));
    }
}

void PacketAnalyserTab::onHeaderContextMenu(QPoint pos)
{
    if (headerContextMenu != nullptr) {
        selectedHeader = ui->packetTableView->horizontalHeader()->logicalIndexAt(pos);
        if (packetModel->isUserColumn(selectedHeader))
            headerContextMenu->popup(ui->packetTableView->horizontalHeader()->viewport()->mapToGlobal(pos));
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
        connect(optionsDialog, &PacketAnalyserOptionsDialog::rejected, this,  &PacketAnalyserTab::onOptionDialogClosed);

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
    ui->messagesLabel->clear();
    orchestrator = orch;
    if (orchestrator != nullptr) {
         setForwarding(orchestrator->isForwarder());

        ui->forwardPushButton->setEnabled(true);

        connect(orchestrator, &SourcesOrchestatorAbstract::forwardingChanged, this, &PacketAnalyserTab::setForwarding);
        connect(orchestrator, &SourcesOrchestatorAbstract::newPacket, this, &PacketAnalyserTab::receiveNewPacket);
        connect(orchestrator, &SourcesOrchestatorAbstract::connectionsChanged, this, &PacketAnalyserTab::onOrchestratorConnectionsChanged);
        connect(orchestrator, &SourcesOrchestatorAbstract::packetInjected, packetModel, &PacketModelAbstract::addPacket);
        connect(orchestrator, &SourcesOrchestatorAbstract::log, this, &PacketAnalyserTab::logMessage);
        connect(orchestrator, &SourcesOrchestatorAbstract::stopped, ui->messagesLabel, &QLabel::clear);

        QWidget * controlWid = orchestrator->getControlGui(this);
        if (controlWid != nullptr) {
            ui->toolBarLayout->insertWidget(ui->toolBarLayout->indexOf(orchCombo) + 1,controlWid);
            connect(orchestrator, &SourcesOrchestatorAbstract::guiRequested, orchCombo, &OrchestratorChooser::onGuiRequested);
        }

        guiHelper->registerOrchestrator(orchestrator);

        if (orchestrator->getHasDirection())
            ui->packetTableView->showColumn(PacketModelAbstract::COLUMN_DIRECTION);
        else
            ui->packetTableView->hideColumn(PacketModelAbstract::COLUMN_DIRECTION);

        setTrackChanges(orchestrator->isTrackingChanges());

        QTimer::singleShot(0, orchestrator, SLOT(start()));
    } else {
        ui->forwardPushButton->setEnabled(false);
    }
}

void PacketAnalyserTab::receiveNewPacket(QSharedPointer<Packet> packet)
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

void PacketAnalyserTab::onForwardPressed()
{
    if (orchestrator != nullptr) {
        if (intercepting) {
            if (!packetQueue.isEmpty()) { // sending the last packet
                QSharedPointer<Packet> pa = packetQueue.dequeue();
                orchestrator->postPacket(pa);
                if (!packetQueue.isEmpty()) {
                    packetModel->addPacket(packetQueue.head());
                }
            }
        } else if (!currentPacket.isNull()) {
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
    SingleViewAbstract * sva = tabHeaderViewsContextMenu->getView(bytesource,nullptr);
    if (sva != nullptr) {
        ViewTab data = tabHeaderViewsContextMenu->getTabData();
        ui->viewTabWidget->addTab(sva,data.tabName);
//        QPushButton * configButton = sva->getConfigButton();
//        if (configButton != nullptr) {
//            ui->viewTabWidget->tabBar()->setTabButton(index,QTabBar::LeftSide, configButton);
//        }
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
        ui->viewTabWidget->removeTab(index);
        ViewTab vt;
        int tabListIndex = translateTabViewIndex(index);

        if (tabListIndex < 0 || tabListIndex >= tabData.size()) {
            qCritical() << tr("The index for Tabdata is out-of-bound T_T");
            return;
        } else {
            vt = tabData.takeAt(tabListIndex);
        }
        sva->deleteLater();
    }
}

void PacketAnalyserTab::onScrollBarChanged(int min, int max)
{
    Q_UNUSED(min);
    if (trackingLast)
        ui->packetTableView->verticalScrollBar()->setValue(max);
}

void PacketAnalyserTab::onByteSourceUpdated(quintptr source)
{
    if (!currentPacket.isNull()) { // current packet is null during cleaning operations, no need to update anything
        if (source != reinterpret_cast<quintptr>(this)) { // if the source is not the tab object (i.e. the source is the hex editor)
            // write the data back to the packet
            currentPacket->setData(bytesource->getRawData());
            checkIfOriginalTabNeeded();
        }
    }
}

bool PacketAnalyserTab::eventFilter(QObject *receiver, QEvent *event)
{
    bool result = QObject::eventFilter(receiver, event);

    if (receiver == ui->viewTabWidget) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[PacketAnalyserTab::eventFilter] nullptr MouseButtonPress X{";
                return true; // not supposed to happen anyway ..
            } else {
                QRect clickable = tabBarRef->geometry();
                clickable.setRight(geometry().right());
                if (me->buttons() == Qt::RightButton){
                    // only in the tabbar area
                    if (clickable.contains(me->pos())) {
                        tabHeaderViewsContextMenu->exec(ui->viewTabWidget->mapToGlobal(me->pos()));
                        return true;
                    }
                }
            }
        }
    } else if (receiver == ui->viewTabWidget->tabBar()) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[PacketAnalyserTab::eventFilter] nullptr MouseButtonDblClick X{";
                return true; // not supposed to happen anyway ..
            } else {
                // checking if we can locate the tab
                int clickedTabId = tabBarRef->tabAt(me->pos());
                if (clickedTabId < 0)
                    return false; // passing over ?

                if (clickedTabId >= tabBarRef->count()) {
                    qCritical() << "[PacketAnalyserTab::eventFilter] clickedTabId out-of-range X{";
                    return false;
                }
                int tabListIndex = translateTabViewIndex(clickedTabId);

                //renaming2
                QString currentName = tabBarRef->tabText(clickedTabId);
                TabAbstract::ViewTab vt = tabData.at(tabListIndex);
                SingleViewAbstract *sva = dynamic_cast<SingleViewAbstract *>(ui->viewTabWidget->widget(clickedTabId));
                if (sva != nullptr) {
                    IntermediateSource * is = dynamic_cast<IntermediateSource *>(sva->getByteSource());
                    if (is != nullptr) {
                        TransformAbstract * ta = is->getWrapperTransform();
                        QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
                        if (itemConfig == nullptr) {
                            qFatal("Cannot allocate memory for QuickViewItemConfig X{");
                        }

                        itemConfig->setTransform(ta);
                        itemConfig->setName(currentName);
                        itemConfig->setWayBoxVisible(ta->isTwoWays());
                        itemConfig->setReadonlyVisible(true);
                        itemConfig->setReadonly(vt.readonly);
                        int ret = itemConfig->exec();
                        if (ret == QDialog::Accepted) {
                            vt.tabName = itemConfig->getName();
                            vt.readonly = itemConfig->getReadonly();
                            TransformAbstract * ta = itemConfig->getTransform();
                            is->setWrapperTransform(ta);
                            is->setCustomReadonly(itemConfig->getReadonly());
                            vt.transform = ta; // don't touch that, ever !!
                            tabBarRef->setTabText(clickedTabId, vt.tabName);
                            tabData.replace(tabListIndex, vt);
                            delete itemConfig;
                        } else { // action cancelled
                            delete itemConfig;
                        }
                    } // if the cast fails no need to open the conf dialog
                } else {
                    qCritical() << "[PacketAnalyserTab::eventFilter] nullptr SingleViewAbstract X{";
                }

                return true;  //no further handling of this event is required
            }
        }
    }
    return result;
}

bool PacketAnalyserTab::getTrackChanges() const
{
    return trackChanges;
}

void PacketAnalyserTab::setTrackChanges(bool value)
{
    if (trackChanges != value) {
        trackChanges = value;
        ui->trackChangesPushButton->blockSignals(true);
        ui->trackChangesPushButton->setChecked(trackChanges);
        ui->trackChangesPushButton->blockSignals(false);
    }
}

bool PacketAnalyserTab::isTrackingLast() const
{
    return trackingLast;
}

void PacketAnalyserTab::setAutoMerge(bool value)
{
    ui->actionAuto_merge_packets->setChecked(value);
}

void PacketAnalyserTab::registerToGlobal()
{
    guiHelper->registerPacketsAnalyser(this);
}

void PacketAnalyserTab::unregisterFromGlobal()
{
    guiHelper->unregisterPacketsAnalyser(this);
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
            selected.append(sortFilterProxyModel->indexToPacketIndex(modelList.at(i)));
        }

        int nindex = static_cast<int>(packetModel->merge(selected));
        if (nindex != PacketModelAbstract::INVALID_POS) {
            QModelIndex index = packetModel->createIndex(nindex,0);
            index = sortFilterProxyModel->mapFromSource(index);
            QSharedPointer<Packet> pac = packetModel->getPacket(nindex);
            if (pac != nullptr) {
                currentPacket = pac;
                bytesource->setData(currentPacket->getData(), reinterpret_cast<quintptr>(this));
                ui->packetTableView->selectRow(index.row());
                sortFilterProxyModel->setSelectedPacket(nindex);
            } else {
                clearCurrentPacket();
            }
        }
    } else if (action == split) {
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
                selected.append(sortFilterProxyModel->indexToPacketIndex(modelList.at(i)));
            }
            ui->packetTableView->selectionModel()->clearSelection();
            packetModel->removePackets(selected);
            clearCurrentPacket();
        }
    } else if (action == clearHighlights) {
        QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();
        for (int i = 0; i < modelList.size(); i++) {
            QSharedPointer<Packet> pac = packetModel->getPacket(sortFilterProxyModel->indexToPacketIndex(modelList.at(i)));
            if (pac != nullptr) {
                pac->setBackground(QColor());
                pac->setComment(QString());
            }
            else
                qCritical() << tr("[PacketAnalyserTab::onContextMenuAction/clearHighlights] packet is nullptr T_T");
        }
    }
}

void PacketAnalyserTab::buildContextMenu()
{
    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == nullptr) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
    }

    highlightMenu = new(std::nothrow) QMenu(tr("Highlight as"));
    if (highlightMenu == nullptr) {
        qFatal("Cannot allocate memory for highlightMenu X{");
    }
    connect(highlightMenu, &QMenu::triggered, this, &PacketAnalyserTab::onHighlightMenu);

    globalContextMenu->addMenu(highlightMenu);

    newHighlight = new(std::nothrow) QAction(tr("New highlight"), this);
    if (newHighlight == nullptr) {
        qFatal("Cannot allocate memory for newHighlight X{");
    }

    clearHighlights = new(std::nothrow) QAction(tr("Clear highlights"), globalContextMenu);
    if (clearHighlights == nullptr) {
        qFatal("Cannot allocate memory for clearHighlights X{");
    }
    globalContextMenu->addAction(clearHighlights);

    sendToMenu = new(std::nothrow) SendToMenu(guiHelper, tr("Send selection to"));
    if (sendToMenu == nullptr) {
        qFatal("Cannot allocate memory for sendToMenu X{");
    }
    connect(sendToMenu, &QMenu::triggered, this, &PacketAnalyserTab::onSendToTriggered, Qt::UniqueConnection);
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addSeparator();

    copyAsMenu = new(std::nothrow) QMenu(tr("Copy as"));
    if (copyAsMenu == nullptr) {
        qFatal("Cannot allocate memory for copyMenu X{");
    }
    connect(copyAsMenu, &QMenu::triggered, this, &PacketAnalyserTab::onCopyAs, Qt::UniqueConnection);
    updateCopyAsMenu();
    globalContextMenu->addMenu(copyAsMenu);

    sendPacketsMenu = new(std::nothrow) QMenu(tr("Send packet(s)"));
    if (sendPacketsMenu == nullptr) {
        qFatal("Cannot allocate memory for sendPacketsMenu X{");
    }
    connect(sendPacketsMenu, &QMenu::triggered, this, &PacketAnalyserTab::onSendPackets, Qt::UniqueConnection);

    globalContextMenu->addMenu(sendPacketsMenu);

    globalContextMenu->addSeparator();

    merge = new(std::nothrow) QAction(tr("merge"), globalContextMenu);
    if (merge == nullptr) {
        qFatal("Cannot allocate memory for Merge X{");
    }
    globalContextMenu->addAction(merge);


    split = new(std::nothrow) QAction(tr("Split"), globalContextMenu);
    if (split == nullptr) {
        qFatal("Cannot allocate memory for Split X{");
    }

    globalContextMenu->addAction(split);

    deleteSelection = new(std::nothrow) QAction(tr("Delete"), globalContextMenu);
    if (deleteSelection == nullptr) {
        qFatal("Cannot allocate memory for Delete X{");
    }

    globalContextMenu->addAction(deleteSelection);

    connect(globalContextMenu, &QMenu::triggered, this, &PacketAnalyserTab::onContextMenuAction, Qt::UniqueConnection);
}

void PacketAnalyserTab::buildHeadersContextMenu()
{
    QHeaderView * headers = ui->packetTableView->horizontalHeader();
    headers->setContextMenuPolicy(Qt::CustomContextMenu);

    headerContextMenu = new(std::nothrow) QMenu(this);
    if (headerContextMenu == nullptr) {
        qFatal("Cannot allocate memory for headerContextMenu X{");
    }


    deleteColumn = new(std::nothrow) QAction(tr("delete"), headerContextMenu);
    if (deleteColumn == nullptr) {
        qFatal("Cannot allocate memory for deleteColumn X{");
    }
    headerContextMenu->addAction(deleteColumn);

    connect(deleteColumn, &QAction::triggered, this, &PacketAnalyserTab::onDeleteColumn);
    connect(headers, &QHeaderView::customContextMenuRequested, this, &PacketAnalyserTab::onHeaderContextMenu);
}

QList<TabAbstract::ViewTab> PacketAnalyserTab::getTabData()
{
    QList<ViewTab> ret;
    // updating the options, in case they have changed (very likely)
    for (int i = 0; i < tabData.size();i++) {
        ViewTab vt = tabData.at(i);
        QWidget *wview = ui->viewTabWidget->widget(i+1);
        if (wview != nullptr) {
            SingleViewAbstract * sva = static_cast<SingleViewAbstract *>(wview);
            vt.options = sva->getConfiguration();

        } else {
            qCritical() << tr("[PacketAnalyserTab::getTabData] Invalid index");
        }

        ret.append(vt);
    }

    return ret;
}

void PacketAnalyserTab::setTrackingLast(bool value)
{
    if (trackingLast != value) {
        trackingLast = value;
        ui->trackPushButton->blockSignals(true);
        ui->trackPushButton->setChecked(trackingLast);
        ui->trackPushButton->blockSignals(false);
        if (trackingLast) {
            ui->packetTableView->selectRow(sortFilterProxyModel->rowCount() - 1);
        }
    }
}

void PacketAnalyserTab::onHighlightMenu(QAction *action)
{
    QString name;
    QColor highlightColor;
    if (action == newHighlight) {
        if (ui->packetTableView->selectionModel()->hasSelection()) {
            highlightColor = QColorDialog::getColor(GuiStyles::DEFAULT_JSON_VALUE_COLOR, this);
            if (highlightColor.isValid()) {
                QPixmap pix(20,20);
                pix.fill(highlightColor);
                TextInputDialog *nameDialog = new(std::nothrow) TextInputDialog(this);
                if (nameDialog == nullptr) {
                    qFatal("Cannot allocate memory for textInputDialog X{");
                }
                nameDialog->setPixLabel(pix);
                int ret = nameDialog->exec();
                if (ret == QDialog::Accepted) {
                    name = nameDialog->getInputText();
                    if (!name.isEmpty())
                        guiHelper->addNewMarkingColor(name,highlightColor);
                }
            }
        }

    } else {
        name = action->text();
        QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
        if (colors.contains(name)) {
            highlightColor = colors.value(name);
        } else {
            qCritical("[PacketAnalyserTab::onHighlightMenu] Unknown marking color T_T");
        }
    }

    if (highlightColor.isValid()) {
        QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();
        for (int i = 0; i < modelList.size(); i++) {
            QSharedPointer<Packet> pac = packetModel->getPacket(sortFilterProxyModel->indexToPacketIndex(modelList.at(i)));
            if (pac != nullptr) {
                pac->setBackground(highlightColor);
                pac->setComment(name);
            }
            else
                qCritical() << tr("[PacketAnalyserTab::onHighlightMenu] packet is nullptr T_T");
        }
    }
}

void PacketAnalyserTab::updateHighlightMenu()
{
    highlightMenu->clear(); // action created on the fly should be automatically deleted
    highlightMenu->addAction(newHighlight);
    highlightMenu->addSeparator();
    QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
    QHashIterator<QString, QColor> i(colors);
    while (i.hasNext()) {
        i.next();
        QPixmap pix(48,48);
        pix.fill(i.value());
        QAction *  action = new(std::nothrow) QAction(QIcon(pix),i.key(), highlightMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateMarkMenu X{");
        }
        highlightMenu->addAction(action);
    }
}

int PacketAnalyserTab::translateTabViewIndex(int index)
{
    int ret = index - 1; // removing one as the initial view does not count
    int oriIndex = ui->viewTabWidget->indexOf(oriHexView); // if the original view is there, removing another one
    if (oriIndex != -1)
        ret--;

    return ret;
}

void PacketAnalyserTab::onSendToTriggered(QAction *action)
{
    QByteArray selectedData;
    QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();

    for (int i = 0; i < modelList.size(); i++) {
        QSharedPointer<Packet> pac = packetModel->getPacket(sortFilterProxyModel->indexToPacketIndex(modelList.at(i)));
        if (pac != nullptr)
            selectedData.append(pac->getData());
        else
            qCritical() << tr("[PacketAnalyserTab::onSendToTriggered] packet is nullptr T_T");
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
        QSharedPointer<Packet> pac = packetModel->getPacket(sortFilterProxyModel->indexToPacketIndex(modelList.at(i)));
        if (!pac.isNull())
            selectedData.append(pac->getData());
        else
            qCritical() << tr("[PacketAnalyserTab::onCopyAs] packet is nullptr T_T");
    }
    guiHelper->copyAction(action->text(), selectedData);
}

void PacketAnalyserTab::onSendPackets(QAction* action)
{
    QModelIndexList modelList = ui->packetTableView->selectionModel()->selectedRows();
    QList<QSharedPointer<Packet> > list;
    for (int i = 0; i < modelList.size(); i++) {
        QSharedPointer<Packet> pac = QSharedPointer<Packet>(packetModel->getPacket(sortFilterProxyModel->indexToPacketIndex(modelList.at(i)))->clone());
        if (!pac.isNull())
            list.append(pac);
        else
            qCritical() << tr("[PacketAnalyserTab::onSendToNewPacketAnalyser] packet is nullptr T_T");
    }

    if (!list.isEmpty()) {
        if (action == sendPacketsToNew) {
            emit guiHelper->newPacketAnalyserRequested(list);
        } else {
            QList<PacketAnalyserTab *> tablist = guiHelper->getRegisteredPacketsAnalysers();
            tablist.removeAll(this); // removing oneself
            bool ok = false;
            int index = action->data().toInt(&ok);
            if (ok && index < tablist.size()) {
                tablist.at(index)->addPackets(list);
            } else {
                qCritical() << tr("[PacketAnalyserTab::onSendPackets] Index is wrong T_T");
            }

        }
    }

}

void PacketAnalyserTab::checkIfOriginalTabNeeded()
{
    if (!currentPacket.isNull() && currentPacket->hasBeenModified()) {
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
        tabBarRef = ui->viewTabWidget->tabBar();
        tabBarRef->setTabButton(index, QTabBar::RightSide,nullptr);
        tabBarRef->setTabButton(index, QTabBar::LeftSide, nullptr);
}

bool PacketAnalyserTab::getForwarding() const
{
    return forwarding;
}

void PacketAnalyserTab::setForwarding(bool value)
{
    forwarding = value;
}

void PacketAnalyserTab::onAutoMergeRequested()
{
    ui->packetTableView->selectionModel()->clear();
    packetModel->mergeConsecutivePackets();

}

void PacketAnalyserTab::onSaveLoadFinished()
{
    setEnabled(true);
}

void PacketAnalyserTab::onItemClicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    setTrackingLast(false);
}

void PacketAnalyserTab::onFilter()
{
    qDebug() << "filter requested";
    if (filterDialog == nullptr) {
        filterDialog = new(std::nothrow) FilterDialog(sortFilterProxyModel, this);
        if (filterDialog == nullptr) {
            qFatal("Cannot allocate memory for FilterDialog X{");
        }
        connect(filterDialog, &FilterDialog::finished, this, &PacketAnalyserTab::onFilterDialogClosed);

        filterDialog->show();
    } else {
        filterDialog->raise();
    }
}

void PacketAnalyserTab::onFilterDialogClosed()
{
    filterDialog = nullptr; // dialog should delete itself
}

void PacketAnalyserTab::onDeleteColumn()
{
    if (packetModel->isUserColumn(selectedHeader)) {
        packetModel->removeUserColumn(selectedHeader);
    }
}

void PacketAnalyserTab::logMessage(const QString &message, const QString &source, LOGLEVEL level)
{
    if (level == Pip3lineConst::LERROR) {
        QPalette palette = ui->messagesLabel->palette();
        palette.setColor(ui->messagesLabel->foregroundRole(), Qt::red);

        ui->messagesLabel->setPalette(palette);
        // print message in the GUI
        ui->messagesLabel->setText(message);
    }
    logger->logMessage(message, source,level);
}

void PacketAnalyserTab::onFontUpdated()
{
    ui->packetTableView->verticalHeader()->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    ui->packetTableView->horizontalHeader()->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    ui->packetTableView->verticalHeader()->setDefaultSectionSize(GlobalsValues::ROWSHEIGHT);
}

void PacketAnalyserTab::onTrackingToggled(bool checked)
{
    if (trackChanges != checked) {
        trackChanges = checked;
        if (!trackChanges && bytesource != nullptr)
            bytesource->clearAllMarkings();
    }
}

void PacketAnalyserTab::selectLastPacket()
{
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
    QHash<QString, QString> confOptions;
    SourcesOrchestatorAbstract * orch = nullptr;
    BlocksSource * bs = nullptr;
    bool ok = false;

    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        // saving general tab conf
        writer->writeAttribute(GuiConst::STATE_TRACK_PACKETS, write(gTab->trackingLast));
        writer->writeAttribute(GuiConst::STATE_TRACK_CHANGES, write(gTab->getTrackChanges()));
        writer->writeAttribute(GuiConst::STATE_AUTO_MERGE, write(gTab->packetModel->isAutoMergeConsecutivePackets()));
        writer->writeAttribute(GuiConst::STATE_MAX_PAYLOAD_DISPLAY_SIZE, write(gTab->packetModel->getMaxPayloadDisplaySize()));
        // saving orchestrator conf
        writer->writeStartElement(GuiConst::STATE_ORCHESTRATOR);
        orch = gTab->orchestrator;
        if (orch != nullptr) {
            writer->writeAttribute(GuiConst::STATE_TYPE, QString::number(orch->getType()));
            confOptions = orch->getConfiguration();
            QHashIterator<QString, QString> i(confOptions);
            while (i.hasNext()) {
                i.next();
                writer->writeAttribute(i.key(), i.value());
            }

            for (int i = 0; i < orch->blockSourceCount(); i++) {
                writer->writeStartElement(GuiConst::STATE_BLOCKSOURCE);
                bs = orch->getBlockSource(i);
                confOptions = bs->getConfiguration();
                QHashIterator<QString, QString> j(confOptions);
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
                writer->writeAttribute(GuiConst::STATE_WIDTH, QString::number(gTab->ui->packetTableView->columnWidth(i)));
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
        QList<TabAbstract::ViewTab> tabs = gTab->getTabData();
        int size = tabs.size();
        writer->writeStartElement(GuiConst::STATE_TABVIEWLIST);
        writer->writeAttribute(GuiConst::STATE_SIZE, write(tabs.size()));
        for (int i = 0; i < size ; i++) {
            TabAbstract::ViewTab vt = tabs.at(i);
            writer->writeStartElement(GuiConst::STATE_TABVIEW);
            writer->writeAttribute(GuiConst::STATE_TYPE, write(static_cast<int>(vt.type)));
            // saving configuration
            QString conf;
            TransformChain list;
            if (vt.transform != nullptr) {
                list.append(vt.transform);
            }
            list.setName(vt.tabName);
            QXmlStreamWriter streamin(&conf);
            gTab->guiHelper->getTransformFactory()->saveConfToXML(list, &streamin);
            writer->writeAttribute(GuiConst::STATE_CONF, write(conf));
            writer->writeAttribute(GuiConst::STATE_READONLY, write(vt.readonly));
            confOptions = vt.options;
            QHashIterator<QString, QString> it(confOptions);
            while (it.hasNext()) {
                it.next();
                if (it.key() != GuiConst::STATE_CONF && it.key() != GuiConst::STATE_TYPE)
                    writer->writeAttribute(it.key(), it.value());
            }
            writer->writeEndElement(); // STATE_TABVIEW
            list.clear(); // the transforms are not own by us, don't delete them
        }
        writer->writeEndElement(); // STATE_TABVIEWLIST

        // stacking state Mngmt obj for sortFilterProxy
        emit addNewState(gTab->sortFilterProxyModel->getStateMngtObj());

    } else { // restore
        if (attributes.hasAttribute(GuiConst::STATE_TRACK_PACKETS)) {
            QString val = attributes.value(GuiConst::STATE_TRACK_PACKETS).toString();
            if (val == GuiConst::STATE_YES) {
                gTab->setTrackingLast(true);
            } else if (val == GuiConst::STATE_NO) {
                gTab->setTrackingLast(false);
            }
        }

        if (attributes.hasAttribute(GuiConst::STATE_TRACK_CHANGES)) {
            QString val = attributes.value(GuiConst::STATE_TRACK_CHANGES).toString();
            if (val == GuiConst::STATE_YES) {
                gTab->setTrackChanges(true);
            } else if (val == GuiConst::STATE_NO) {
                gTab->setTrackChanges(false);
            }
        }

        if (attributes.hasAttribute(GuiConst::STATE_AUTO_MERGE)) {
            QString val = attributes.value(GuiConst::STATE_AUTO_MERGE).toString();
            if (val == GuiConst::STATE_YES) {
                gTab->setAutoMerge(true);
            } else if (val == GuiConst::STATE_NO) {
                gTab->setAutoMerge(false);
            }
        }

        if (attributes.hasAttribute(GuiConst::STATE_MAX_PAYLOAD_DISPLAY_SIZE)) {
            int val = attributes.value(GuiConst::STATE_MAX_PAYLOAD_DISPLAY_SIZE).toInt(&ok);
            if (ok && val >= PacketModelAbstract::MAX_PAYLOAD_DISPLAY_SIZE_MIN_VAL && val < PacketModelAbstract::MAX_PAYLOAD_DISPLAY_SIZE_MAX_VAL) {
                gTab->packetModel->setMaxPayloadDisplaySize(val);
            }
        }

        QMap<int, int> visualindexes;
        if (reader->readNextStartElement()) {
            qDebug() << "trying to read the orchestrator configuration" << reader->name().toString();
            if (reader->name() == GuiConst::STATE_ORCHESTRATOR) {
                // setting Orchestrator configuration
                QXmlStreamAttributes attributes = reader->attributes();
                confOptions.clear();
                for (int i = 0; i < attributes.size(); i++) {
                    QXmlStreamAttribute attr = attributes.at(i);
                    if (attr.name() == GuiConst::STATE_TYPE) {
                        int type = attr.value().toString().toInt(&ok);
                        if (!ok) {
                            qCritical() << tr("[PacketAnalyserTabStateObj] Cannot parse the type of the orchestrator: %1").arg(attr.name().toString());
                        } else {
                            gTab->orchCombo->setType(type);
                        }
                    } else {
                        confOptions.insert(attr.name().toString(), attr.value().toString());
                    }
                }
                orch = gTab->orchCombo->getOrchestrator();
                if (orch != nullptr) {
                    orch->setConfiguration(confOptions);
                    // settings the different blocksources configuration
                    int index = 0;
                    while (reader->readNext() != QXmlStreamReader::EndElement) {
                        if (reader->name() == GuiConst::STATE_BLOCKSOURCE) {
                            QXmlStreamAttributes attributes = reader->attributes();
                            confOptions.clear();
                            for (int i = 0; i < attributes.size(); i++) {
                                QXmlStreamAttribute attr = attributes.at(i);
                                confOptions.insert(attr.name().toString(), attr.value().toString());
                            }

                            bs = orch->getBlockSource(index);
                            if (bs != nullptr) {
                                bs->setConfiguration(confOptions);
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
                else {
                    if (reader->readNext() != QXmlStreamReader::EndElement) {
                        qCritical() << tr("[PacketAnalyserTabStateObj] orchestrator is nullptr T_T");
                    } // otherwise this is expected if type = 0
                }

                reader->readNext(); // read next token
            }

            // columns configurations
            if (reader->name() == GuiConst::STATE_COLUMNS_CONF) {
                while(reader->readNextStartElement()) {
                    QXmlStreamAttributes attributes = reader->attributes();
                    if (attributes.hasAttribute(GuiConst::STATE_MODEL_INDEX)) {
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

                            if (attributes.hasAttribute(GuiConst::STATE_WIDTH)) {
                                tempint = attributes.value(GuiConst::STATE_WIDTH).toString().toInt(&ok);
                                if (ok) {
                                    gTab->ui->packetTableView->setColumnWidth(index,tempint);
                                }
                            }
                        }
                    }
                    while (reader->readNext() != QXmlStreamReader::EndElement); // reading end element
                }
                reader->readNext();
            }
        }

        if (flags & GuiConst::STATE_LOADSAVE_DATA) { // load data only if requested
            worker.loadFromXML(reader);
            gTab->packetModel->addPackets(worker.getLoadedPackets());
        }

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
                int size = readInt(attrList.value(GuiConst::STATE_SIZE),&ok);
                if (ok) {
                    for (int i = 0; i < size ; i++) {
                        TabAbstract::ViewTab vt;
                        vt.transform = nullptr; // just initialising in case of screw up
                        if (readNextStart(GuiConst::STATE_TABVIEW)) {
                            attrList = reader->attributes();
                            if (attrList.hasAttribute(GuiConst::STATE_READONLY)) {
                                vt.readonly = readBool(attrList.value(GuiConst::STATE_READONLY));
                            }
                            readEndElement(GuiConst::STATE_TABVIEW); // closing now, because there is no child defined anyway
                            if (attrList.hasAttribute(GuiConst::STATE_TYPE)) {
                                int type = readInt(attrList.value(GuiConst::STATE_TYPE),&ok);
                                if (ok && (type == 0 || type == 1 || type == 2 || type == 3 || type == 4)) {
                                    vt.type = static_cast<TabAbstract::ViewType>(type);
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

                            confOptions.clear();
                            for (int i = 0; i < attrList.size(); i++) {
                                QXmlStreamAttribute attr = attrList.at(i);
                                if (attr.name() != GuiConst::STATE_CONF && attr.name() != GuiConst::STATE_TYPE)
                                    confOptions.insert(attr.name().toString(), attr.value().toString());
                            }

                            vt.options = confOptions;

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

        // stacking state Mngmt obj for sortFilterProxy
        emit addNewState(gTab->sortFilterProxyModel->getStateMngtObj());
    }
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

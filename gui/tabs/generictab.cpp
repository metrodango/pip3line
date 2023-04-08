/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "generictab.h"
#include "ui_generictab.h"
#include <QDebug>
#include "shared/offsetgotowidget.h"
#include "shared/searchwidget.h"
#include "sources/bytesourceabstract.h"
#include "views/hexview.h"
#include "views/textview.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QKeyEvent>
#include <QAction>
#include <QTabWidget>
#include <QDropEvent>
#include <QMenu>
#include <QDragEnterEvent>
#include "quickviewitemconfig.h"
#include <QUrl>
#include "downloadmanager.h"
#include "loggerwidget.h"
#include "guihelper.h"
#include "shared/readonlybutton.h"
#include "shared/clearallmarkingsbutton.h"
#include "shared/bytesourceguibutton.h"
#include "shared/detachtabbutton.h"
#include "shared/messagepanelwidget.h"
#include "sources/intermediatesource.h"
#include "shared/universalreceiverbutton.h"
#include "shared/guiconst.h"
#include "shared/newviewmenu.h"
#include "shared/transformguibutton.h"
#include "views/bytetableview.h"
#include "views/singleviewabstract.h"
#include <QScrollBar>

using namespace GuiConst;

const QString GenericTab::TEXT_TEXT = "Text";

GenericTab::GenericTab(ByteSourceAbstract *nbytesource, GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper,parent),
    bytesource(nbytesource)
{
    ableToReceiveData = false;
    setName(bytesource->name());
    connect(bytesource, &ByteSourceAbstract::nameChanged, this, &GenericTab::setName);

    hexView = new(std::nothrow) HexView(bytesource, guiHelper,this);
    if (hexView == nullptr) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    ui = new(std::nothrow) Ui::GenericTab();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::FileTab X{");
    }
    ui->setupUi(this);
    ui->tabWidget->addTab(hexView, tr("Hex"));

    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &GenericTab::onDeleteTab);

    searchWidget = new(std::nothrow) SearchWidget(bytesource, guiHelper, this);
    if (searchWidget == nullptr) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    connect(searchWidget, &SearchWidget::jumpTo, hexView, &HexView::gotoSearch);

    ui->mainLayout->insertWidget(1,searchWidget);
    connect(searchWidget, &SearchWidget::searchRequest, this, &GenericTab::onSearch);

    hexView->installEventFilter(this);

    gotoWidget = new(std::nothrow) OffsetGotoWidget(guiHelper,this);
    if (gotoWidget == nullptr) {
        qFatal("Cannot allocate memory for OffsetGotoWidget X{");
    }
    gotoWidget->setMaximumWidth(150);
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(ui->loadPushButton) + 1, gotoWidget);
    connect(gotoWidget, &OffsetGotoWidget::gotoRequest, this, &GenericTab::onGotoOffset);

    roButton = new(std::nothrow) ReadOnlyButton(bytesource,this);
    if (roButton == nullptr) {
        qFatal("Cannot allocate memory for ReadOnlyButton X{");
    }
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(gotoWidget) + 2,roButton);

    clearAllMarksButton = new(std::nothrow) ClearAllMarkingsButton(bytesource,this);
    if (clearAllMarksButton == nullptr) {
        qFatal("Cannot allocate memory for clearAllMarkingsButton X{");
    }
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(roButton) + 1, clearAllMarksButton);

    guiButton = new(std::nothrow) ByteSourceGuiButton(bytesource,guiHelper,this);
    if (guiButton == nullptr) {
        qFatal("Cannot allocate memory for ByteSourceGuiButton X{");
    }

    ui->mainToolBarLayout->insertWidget(5, guiButton);

    integrateByteSource();

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == nullptr) {
        qFatal("Cannot allocate memory for detachButton X{");
    }

    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(ui->loadPushButton) + 1,detachButton);

    messagePanel = new(std::nothrow) MessagePanelWidget(this);
    if (messagePanel == nullptr) {
        qFatal("Cannot allocate memory for MessagePanelWidget X{");
    }


    ui->mainLayout->insertWidget(ui->mainLayout->indexOf(ui->tabWidget) + 1,messagePanel);
    connect(bytesource, &ByteSourceAbstract::log, messagePanel, &MessagePanelWidget::log);
    connect(bytesource, &ByteSourceAbstract::reset, messagePanel, &MessagePanelWidget::closeWidget);

    // Checking if there are some additional buttons
    QWidget *gui = bytesource->getGui(this, ByteSourceAbstract::GUI_BUTTONS);

    if (gui != nullptr) {
        ui->mainToolBarLayout->insertWidget(4, gui);
    }

    // Checking if there is an additional upper pane
    gui = bytesource->getGui(this, ByteSourceAbstract::GUI_UPPER_VIEW);

    if (gui != nullptr) {
        ui->mainLayout->insertWidget(1,gui);
    }

    //new view menu
    newViewsContextMenu = new(std::nothrow) NewViewMenu(guiHelper, this);
    if (newViewsContextMenu == nullptr) {
        qFatal("Cannot allocate memory for NewViewMenu X{");
    }

    connect(newViewsContextMenu, &NewViewMenu::newViewRequested, this, &GenericTab::onNewTabRequested);
    ui->tabWidget->installEventFilter(this);
    ui->tabWidget->tabBar()->installEventFilter(this);
    ui->addViewPushButton->setMenu(newViewsContextMenu);

    UniversalReceiverButton *urb = new(std::nothrow) UniversalReceiverButton(this, guiHelper);
    if (urb == nullptr) {
        qFatal("Cannot allocate memory for UniversalReceiverButton X{");
    }

    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(gotoWidget) + 1,urb);

    connect(hexView, &HexView::askForFileLoad, this, &GenericTab::fileLoadRequest);
    connect(ui->loadPushButton, &QPushButton::clicked, this, &GenericTab::fileLoadRequest);
    connect(ui->historyUndoPushButton, &QPushButton::clicked, this, &GenericTab::onHistoryBackward);
    connect(ui->historyRedoPushButton, &QPushButton::clicked, this, &GenericTab::onHistoryForward);

    setAcceptDrops(true);
}

GenericTab::~GenericTab()
{
    delete gotoWidget;
    delete searchWidget;
    delete detachButton;
    delete guiButton;
    delete ui;
    delete bytesource;
    bytesource = nullptr;
}

void GenericTab::loadFromFile(QString fileName)
{
    if (fileName.isEmpty()) {
        logger->logError("Empty file name, ignoring",metaObject()->className());
        return;
    }

    if (bytesource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        bytesource->fromLocalFile(fileName);
        integrateByteSource();
    }  else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg((static_cast<QObject *>(bytesource))->metaObject()->className()),QMessageBox::Ok);
    }
}

int GenericTab::getBlockCount() const
{
    return 1;
}

ByteSourceAbstract *GenericTab::getSource(int)
{
    return bytesource;
}

ByteTableView *GenericTab::getHexTableView(int)
{
    return hexView->getHexTableView();
}

void GenericTab::setData(const QByteArray &data)
{
    if (bytesource->isReadonly()) {
        QMessageBox::critical(this, tr("Read only"), tr("The byte source is readonly"),QMessageBox::Ok);
    } else if (bytesource->hasCapability(ByteSourceAbstract::CAP_RESET)) {
        bytesource->setData(data);
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_RESET capability").arg((static_cast<QObject *>(bytesource))->metaObject()->className()),QMessageBox::Ok);
    }
}

bool GenericTab::canReceiveData()
{
    return (bytesource != nullptr && bytesource->hasCapability(ByteSourceAbstract::CAP_RESET));
}

BaseStateAbstract *GenericTab::getStateMngtObj()
{
    GenericTabStateObj *stateObj = new(std::nothrow) GenericTabStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for GenericTabStateObj X{");
    }

    return stateObj;
}

void GenericTab::fileLoadRequest()
{
    QString fileName;
    if (bytesource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"),GuiConst::GLOBAL_LAST_PATH);
        if (!fileName.isEmpty()) {
            QFileInfo fi(fileName);
            GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
            bytesource->fromLocalFile(fileName);
            integrateByteSource();
            setName(QFileInfo(fileName).fileName());
        }
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg((static_cast<QObject *>(bytesource))->metaObject()->className()),QMessageBox::Ok);
    }
}

void GenericTab::onGotoOffset(quint64 offset, bool absolute, bool negative, bool select)
{
    if (!hexView->goTo(offset,absolute,negative, select)) {
        gotoWidget->setStyleSheet(GuiStyles::LineEditError);
    } else {
        gotoWidget->setStyleSheet(qApp->styleSheet());
    }
}

void GenericTab::onHistoryBackward()
{
    bytesource->historyBackward();
}

void GenericTab::onHistoryForward()
{
    bytesource->historyForward();
}

void GenericTab::onDeleteTab(int index)
{
    if (ui->tabWidget->indexOf(hexView) == index) {
        logger->logError(tr("Cannot close the main hexadecimal tab"),metaObject()->className());
    } else { // here we trust the index as it comes from the QTabWidget
        SingleViewAbstract * sva = static_cast<SingleViewAbstract *>(ui->tabWidget->widget(index));
        ViewTab vt;
        index--; // need to reduce the index to match the tabData index
        if (index < 0 || index >= tabData.size()) {
            qCritical() << tr("The index for Tabdata is out-of-bound T_T");
            return;
        } else {
            vt = tabData.takeAt(index);
            ui->tabWidget->removeTab(index + 1);
        }

        sva->deleteLater();
    }
}

void GenericTab::onNewTabRequested()
{
    SingleViewAbstract * sva = newViewsContextMenu->getView(bytesource, nullptr);
    if (sva != nullptr) {
        ViewTab data = newViewsContextMenu->getTabData();
        ui->tabWidget->addTab(sva,data.tabName);
        tabData.append(data);
    }
}

void GenericTab::onSearch(QByteArray item, QBitArray mask, bool)
{
    hexView->search(item, mask);
}

bool GenericTab::eventFilter(QObject *receiver, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) { // ctrl-[key]
            if (keyEvent->key() == Qt::Key_N)  {
                searchWidget->nextFind(hexView->getLowPos());
                return true;
            } else if (keyEvent->key() == Qt::Key_G) {
                gotoWidget->setFocus();
                return true;
            } else if (keyEvent->key() == Qt::Key_F) {
                searchWidget->setFocus();
                return true;
            }
        }
    }
    bool ret = QObject::eventFilter(receiver, event);
    if (receiver == ui->tabWidget) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[GenericTab::eventFilter] nullptr MouseEvent";
                return true; // not supposed to happen anyway ..
            } else if (me->buttons() == Qt::RightButton){
                // only in the tabbar area
                QRect clickable = ui->tabWidget->tabBar()->geometry();
                clickable.setRight(geometry().right());
                if (clickable.contains(me->pos())) {
                    newViewsContextMenu->exec(ui->tabWidget->mapToGlobal(me->pos()));
                    return true;
                }
            }
        }
    } else if (receiver == ui->tabWidget->tabBar()) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[GenericTab::eventFilter] nullptr MouseButtonDblClick X{";
                return true; // not supposed to happen anyway ..
            } else {
                // checking if we can locate the tab
                int clickedTabId = ui->tabWidget->tabBar()->tabAt(me->pos());
                if (clickedTabId < 0)
                    return false; // passing over ?

                if (clickedTabId >= ui->tabWidget->tabBar()->count()) {
                    qCritical() << "[GenericTab::eventFilter] clickedTabId out-of-range X{";
                    return false;
                }
                int tabListIndex = clickedTabId - 1; // removing one because initial tab

                //renaming2
                QString currentName = ui->tabWidget->tabBar()->tabText(clickedTabId);
                TabAbstract::ViewTab vt = tabData.at(tabListIndex);
                SingleViewAbstract *sva = dynamic_cast<SingleViewAbstract *>(ui->tabWidget->widget(clickedTabId));
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
                            ui->tabWidget->tabBar()->setTabText(clickedTabId, vt.tabName);
                            tabData.replace(tabListIndex, vt);
                            delete itemConfig;
                        } else { // action cancelled
                            delete itemConfig;
                        }
                    } // if the cast fails no need to open the conf dialog
                } else {
                    qCritical() << "[GenericTab::eventFilter] nullptr SingleViewAbstract X{";
                }

                return true;  //no further handling of this event is required
            }
        }
    }
    return ret;
}

void GenericTab::dragEnterEvent(QDragEnterEvent *event)
{
    guiHelper->processDragEnter(event, bytesource);
}

void GenericTab::dropEvent(QDropEvent *event)
{
    guiHelper->processDropEvent(event, bytesource);
}

void GenericTab::integrateByteSource()
{
    if (bytesource != nullptr) {
        roButton->refreshStateValue();

        ui->historyRedoPushButton->setEnabled(bytesource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
        ui->historyUndoPushButton->setEnabled(bytesource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
    }
}

void GenericTab::addViewTab(GenericTab::ViewTab data)
{
    newViewsContextMenu->setTabData(data);
    onNewTabRequested();
}

QList<TabAbstract::ViewTab> GenericTab::getTabData()
{
    QList<ViewTab> ret;
    // updating the options, in case they have changed (very likely)
    for (int i = 0; i < tabData.size();i++) {
        ViewTab vt = tabData.at(i);
        QWidget *wview = ui->tabWidget->widget(i+1);
        if (wview != nullptr) {
            SingleViewAbstract * sva = static_cast<SingleViewAbstract *>(wview);
            vt.options = sva->getConfiguration();

        } else {
            qCritical() << tr("[GenericTab::getTabData] Invalid index");
        }

        ret.append(vt);
    }

    return ret;
}

MessagePanelWidget *GenericTab::getMessagePanel() const
{
    return messagePanel;
}

SearchWidget *GenericTab::getSearchWidget() const
{
    return searchWidget;
}

OffsetGotoWidget *GenericTab::getGotoWidget() const
{
    return gotoWidget;
}

GenericTabStateObj::GenericTabStateObj(GenericTab *tab) :
    TabStateObj(tab)
{
    setName(metaObject()->className());
}

GenericTabStateObj::~GenericTabStateObj()
{

}

void GenericTabStateObj::run()
{
    GenericTab * gTab = dynamic_cast<GenericTab *> (tab);
    TabStateObj::run();
    QHash<QString, QString> options;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeAttribute(GuiConst::STATE_SEARCH_DATA, write(gTab->getSearchWidget()->text(),true));
        writer->writeAttribute(GuiConst::STATE_GOTOOFFSET_DATA, write(gTab->getGotoWidget()->text()));
        writer->writeAttribute(GuiConst::STATE_MESSAGE_PANEL, write(gTab->getMessagePanel()->toHTML(),true));
        writer->writeAttribute(GuiConst::STATE_MESSAGE_PANEL_VISIBLE, write(gTab->getMessagePanel()->isVisible()));
        writer->writeAttribute(GuiConst::STATE_SCROLL_INDEX, write(gTab->hexView->getHexTableView()->verticalScrollBar()->value()));
        writer->writeAttribute(GuiConst::STATE_CURRENT_INDEX, write(gTab->ui->tabWidget->currentIndex()));
        QList<TabAbstract::ViewTab> tabs = gTab->getTabData();
        int size = tabs.size();
        writer->writeStartElement(GuiConst::STATE_TABVIEWLIST);
        writer->writeAttribute(GuiConst::STATE_SIZE, write(tabs.size()));
        for (int i = 0; i < size ; i++) {
            writer->writeStartElement(GuiConst::STATE_TABVIEW);
            TabAbstract::ViewTab vt = tabs.at(i);
            if (vt.type == TabAbstract::JSONVIEW) {
                qDebug() << tr("generic");
            }
            writer->writeAttribute(GuiConst::STATE_TYPE, write(static_cast<int>(vt.type)));
            // saving configuration
            QString conf;
            TransformChain list;
            if (vt.transform != nullptr) {
                list.append(vt.transform);
            }
            list.setName(vt.tabName);
            QXmlStreamWriter streamin(&conf);
            tab->getHelper()->getTransformFactory()->saveConfToXML(list, &streamin);
            writer->writeAttribute(GuiConst::STATE_CONF, write(conf));
            writer->writeAttribute(GuiConst::STATE_READONLY, write(vt.readonly));
            options = vt.options;
            QHashIterator<QString, QString> it(options);
            while (it.hasNext()) {
                it.next();
                if (it.key() != GuiConst::STATE_CONF && it.key() != GuiConst::STATE_TYPE)
                    writer->writeAttribute(it.key(), it.value());
            }
            writer->writeEndElement();
            list.clear(); // the transforms are not own by us, don't delete them
        }
        writer->writeEndElement(); // STATE_TABVIEWLIST
    } else {
        QXmlStreamAttributes attrList = reader->attributes();
        if (attrList.hasAttribute(GuiConst::STATE_SEARCH_DATA)) {
            gTab->getSearchWidget()->setText(readString(attrList.value(GuiConst::STATE_SEARCH_DATA)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_GOTOOFFSET_DATA)) {
            gTab->getGotoWidget()->setText(readString(attrList.value(GuiConst::STATE_GOTOOFFSET_DATA)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_MESSAGE_PANEL)) {
            gTab->getMessagePanel()->setHTML(readString(attrList.value(GuiConst::STATE_MESSAGE_PANEL)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_MESSAGE_PANEL_VISIBLE)) {
            gTab->getMessagePanel()->setVisible(readBool(attrList.value(GuiConst::STATE_MESSAGE_PANEL_VISIBLE)));
        }

        bool ok = false;
        int index = -1;
        GenericTabClosingStateObj *tempState = nullptr;
        if (attrList.hasAttribute(GuiConst::STATE_SCROLL_INDEX)) {
            index = readInt(attrList.value(GuiConst::STATE_SCROLL_INDEX), &ok);
            if (ok) {
                tempState = new(std::nothrow) GenericTabClosingStateObj(gTab);
                if (tempState == nullptr) {
                    qFatal("Cannot allocate memory for GenericTabClosingStateObj X{");
                }
                tempState->setScrollIndex(index);
            }
        }

        if (attrList.hasAttribute(GuiConst::STATE_CURRENT_INDEX)) {
            index = readInt(attrList.value(GuiConst::STATE_CURRENT_INDEX), &ok);
            if (ok) {
                if (tempState == nullptr) {
                    tempState = new(std::nothrow) GenericTabClosingStateObj(gTab);
                    if (tempState == nullptr) {
                        qFatal("Cannot allocate memory for GenericTabClosingStateObj X{");
                    }
                }
                tempState->setCurrentIndex(index);
            }
        }

        if (tempState != nullptr)
            emit addNewState(tempState);

        // restoring view tabs
        if (readNextStart(GuiConst::STATE_TABVIEWLIST)) {
            attrList = reader->attributes();
            if (attrList.hasAttribute(GuiConst::STATE_SIZE)) {
                ok = false;
                int size = readInt(attrList.value(GuiConst::STATE_SIZE),&ok);
                if (ok) {
                    for (int i = 0; i < size ; i++) {
                        GenericTab::ViewTab vt;
                        vt.transform = nullptr; // just initialising in case of screw up
                        if (readNextStart(GuiConst::STATE_TABVIEW)) {
                            attrList = reader->attributes();
                            if (attrList.hasAttribute(GuiConst::STATE_READONLY)) {
                                vt.readonly = readBool(attrList.value(GuiConst::STATE_READONLY));
                            }
                            readEndElement(GuiConst::STATE_TABVIEW); // closing now, because there is no child defined anyway
                            if (attrList.hasAttribute(GuiConst::STATE_TYPE)) {
                                int type = readInt(attrList.value(GuiConst::STATE_TYPE),&ok);
                                if (ok && (type == TabAbstract::HEXVIEW ||
                                           type == TabAbstract::TEXTVIEW ||
                                           type == TabAbstract::DEFAULTTEXT ||
                                           type == TabAbstract::JSONVIEW)) {
                                    vt.type = static_cast<GenericTab::ViewType>(type);
                                } else {
                                    qWarning() << "Invalid state type for this view, skipping";
                                    continue;
                                }
                            } else {
                                qWarning() << "no state type for this view, skipping";
                                continue;
                            }

                            if (vt.type != GenericTab::DEFAULTTEXT) {
                                if (attrList.hasAttribute(GuiConst::STATE_CONF)) {
                                    QString conf = readString(attrList.value(GuiConst::STATE_CONF));
                                    if (!conf.isEmpty()) {
                                        TransformChain list = tab->getHelper()->getTransformFactory()->loadConfFromXML(conf);
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
                                vt.tabName = GenericTab::TEXT_TEXT;
                            }
                            options.clear();
                            for (int i = 0; i < attrList.size(); i++) {
                                QXmlStreamAttribute attr = attrList.at(i);
                                if (attr.name() != GuiConst::STATE_CONF && attr.name() != GuiConst::STATE_TYPE)
                                    options.insert(attr.name().toString(), attr.value().toString());
                            }

                            vt.options = options;

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

    BaseStateAbstract *state = gTab->getSource(0)->getStateMngtObj();
    emit addNewState(state);
}



GenericTabClosingStateObj::GenericTabClosingStateObj(GenericTab *tab) :
    tab(tab)
{
    scrollIndex = 0;
    currentIndex = 0;
    //name = metaObject()->className();
}

GenericTabClosingStateObj::~GenericTabClosingStateObj()
{

}

void GenericTabClosingStateObj::run()
{
    tab->hexView->getHexTableView()->verticalScrollBar()->setValue(scrollIndex);
    tab->ui->tabWidget->setCurrentIndex(currentIndex);
}

void GenericTabClosingStateObj::setScrollIndex(int value)
{
    scrollIndex = value;
}

void GenericTabClosingStateObj::setCurrentIndex(int value)
{
    currentIndex = value;
}


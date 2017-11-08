/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
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
    connect(bytesource,SIGNAL(nameChanged(QString)), SLOT(setName(QString)));

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

    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, 0);
    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::LeftSide, 0);

    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(onDeleteTab(int)));

    searchWidget = new(std::nothrow) SearchWidget(bytesource, guiHelper, this);
    if (searchWidget == nullptr) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    connect(searchWidget, SIGNAL(jumpTo(quint64,quint64)), hexView, SLOT(gotoSearch(quint64,quint64)));

    ui->mainLayout->insertWidget(1,searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,QBitArray,bool)), SLOT(onSearch(QByteArray,QBitArray,bool)));

    hexView->installEventFilter(this);

    gotoWidget = new(std::nothrow) OffsetGotoWidget(guiHelper,this);
    if (gotoWidget == nullptr) {
        qFatal("Cannot allocate memory for OffsetGotoWidget X{");
    }
    gotoWidget->setMaximumWidth(150);
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(ui->loadPushButton) + 1, gotoWidget);
    connect(gotoWidget,SIGNAL(gotoRequest(quint64,bool,bool,bool)), SLOT(onGotoOffset(quint64,bool,bool,bool)));

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
    connect(bytesource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), messagePanel, SLOT(log(QString,QString,Pip3lineConst::LOGLEVEL)));
    connect(bytesource, SIGNAL(reset()), messagePanel, SLOT(closeWidget()));
    //connect(byteSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger, SLOT(log(QString,QString,Pip3lineConst::LOGLEVEL)));

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

    connect(newViewsContextMenu, SIGNAL(newViewRequested()), SLOT(onNewTabRequested()));
    ui->tabWidget->installEventFilter(this);
    ui->addViewPushButton->setMenu(newViewsContextMenu);

    UniversalReceiverButton *urb = new(std::nothrow) UniversalReceiverButton(this, guiHelper);
    if (urb == nullptr) {
        qFatal("Cannot allocate memory for UniversalReceiverButton X{");
    }

    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(gotoWidget) + 1,urb);

    connect(hexView,SIGNAL(askForFileLoad()), SLOT(fileLoadRequest()));
    connect(ui->loadPushButton, SIGNAL(clicked()), SLOT(fileLoadRequest()));
    connect(ui->historyUndoPushButton, SIGNAL(clicked()), SLOT(onHistoryBackward()));
    connect(ui->historyRedoPushButton, SIGNAL(clicked()), SLOT(onHistoryForward()));

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
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
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
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_RESET capability").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
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
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
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
        }

        delete sva;
    }
}

void GenericTab::onNewTabRequested()
{
    SingleViewAbstract * sva = newViewsContextMenu->getView(bytesource,ui->tabWidget);
    if (sva != nullptr) {
        ViewTab data = newViewsContextMenu->getTabData();
        int index = ui->tabWidget->addTab(sva,data.tabName);
        QPushButton * configButton = sva->getConfigButton();
        if (configButton != nullptr) {
            ui->tabWidget->tabBar()->setTabButton(index,QTabBar::LeftSide, configButton);
        }
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
    SingleViewAbstract * newView = nullptr;
    QTabWidget * thetabwiget = ui->tabWidget;
    QPushButton * configButton = nullptr;

    switch (data.type) {
        case (GenericTab::HEXVIEW) : {
                IntermediateSource * is = new(std::nothrow) IntermediateSource(guiHelper,bytesource,data.transform);
                if (is == nullptr) {
                    qFatal("Cannot allocate memory for IntermediateSource X{");
                }

                newView = new(std::nothrow) HexView(is,guiHelper,thetabwiget,true);
                if (newView == nullptr) {
                    qFatal("Cannot allocate memory for HexView X{");
                }
                configButton = new(std::nothrow) TransformGuiButton(data.transform);
                if (configButton == nullptr) {
                    qFatal("Cannot allocate memory for TransformGuiButton X{");
                }
            }
            break;
        case (GenericTab::TEXTVIEW) : {
                IntermediateSource * is = new(std::nothrow) IntermediateSource(guiHelper,bytesource,data.transform);
                if (is == nullptr) {
                    qFatal("Cannot allocate memory for IntermediateSource X{");
                }

                newView = new(std::nothrow) TextView(is,guiHelper,thetabwiget,true);
                if (newView == nullptr) {
                    qFatal("Cannot allocate memory for TextView X{");
                }
                configButton = new(std::nothrow) TransformGuiButton(data.transform);
                if (configButton == nullptr) {
                    qFatal("Cannot allocate memory for TransformGuiButton X{");
                }
            }
            break;
        case (GenericTab::DEFAULTTEXT) : {
                newView = new(std::nothrow) TextView(bytesource,guiHelper,thetabwiget);
                if (newView == nullptr) {
                    qFatal("Cannot allocate memory for TextView X{");
                }
            }
            break;
        default: {
                logger->logError(tr("View Type undefined"));
            }
    }

    if (newView != nullptr) {
        newView->setConfiguration(data.options);
        int index = thetabwiget->addTab(newView,data.tabName);
        if (configButton != nullptr) {
            thetabwiget->tabBar()->setTabButton(index,QTabBar::LeftSide, configButton);
        }

        tabData.append(data);
    }
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
            writer->writeAttribute(GuiConst::STATE_TYPE, write((int)vt.type));
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
                            readEndElement(GuiConst::STATE_TABVIEW); // closing now, because there is no child defined anyway
                            if (attrList.hasAttribute(GuiConst::STATE_TYPE)) {
                                int type = readInt(attrList.value(GuiConst::STATE_TYPE),&ok);
                                if (ok && (type == GenericTab::HEXVIEW ||
                                           type == GenericTab::TEXTVIEW ||
                                           type == GenericTab::DEFAULTTEXT)) {
                                    vt.type = (GenericTab::ViewType) type;
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
    name = metaObject()->className();
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


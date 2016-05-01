/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "maintabs.h"
#include "textinputdialog.h"
#include "tabs/transformsgui.h"
#include "tabs/generictab.h"
#include "tabs/randomaccesstab.h"
#include "tabs/packetanalysertab.h"
#include <QMouseEvent>
#include <QFileInfo>
#include <QDebug>
#include <QFileDialog>
#include <QAction>
#include "shared/guiconst.h"
#include "state/closingstate.h"
#include "sources/largefile.h"
#include "sources/basicsource.h"
#include "sources/largefile.h"
#include "sources/currentmemorysource.h"

using namespace GuiConst;

const QString MainTabs::ID = "MainTabs";
const QString MainTabs::STATE_LOGGER_TAB = "LoggerTab";

MainTabs::MainTabs(GuiHelper *nguiHelper, QWidget *parent) :
    QTabWidget(parent)
{
    tabHeaderContextMenu = nullptr;
    tabBarRef = tabBar();
    tabBarRef->installEventFilter(this);
    installEventFilter(this);

    tabCount = 1;
    maxTabCount = GuiConst::DEFAULT_MAX_TAB_COUNT;

    setTabsClosable(true);
    setMovable(true);

    guiHelper = nguiHelper;
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onDeleteTab(int)));

    logger = guiHelper->getLogger();
    if (logger != nullptr) {
        connect(logger, SIGNAL(errorRaised()), this, SLOT(onLogError()));
        connect(logger,SIGNAL(cleanStatus()), this, SLOT(onLogCleanStatus()));
        int index = this->addTab(logger,tr("Logs"));
        if (logger->hasUncheckedError())
            tabBarRef->setTabTextColor(index, Qt::red);
    }

    tabHeaderContextMenu = new(std::nothrow) QMenu();
    if (tabHeaderContextMenu == nullptr) {
        qFatal("Cannot allocate memory for deletedTabContextMenu X{");
        return;
    }
    updateTabHeaderMenu();
    connect(guiHelper, SIGNAL(deletedTabsUpdated()), this, SLOT(updateTabHeaderMenu()));
    connect(tabHeaderContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(onDeletedTabSelected(QAction*)));
    connect(guiHelper, SIGNAL(tabRevived(TabAbstract*)), this, SLOT(integrateTab(TabAbstract*)));
}

MainTabs::~MainTabs()
{
    clearTabs();
    logger = nullptr;
    guiHelper = nullptr;
    tabBarRef = nullptr;
    delete tabHeaderContextMenu;
}

bool MainTabs::eventFilter(QObject *receiver, QEvent *event)
{
    bool result = QObject::eventFilter(receiver, event);

    if (receiver == tabBarRef) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[MainTabs::eventFilter] nullptr MouseEvent";
                return true; // not supposed to happen anyway ..
            } else {
                // checking if we can locate the tab
                int clickedTabId = tabBarRef->tabAt(me->pos());
                if (clickedTabId == -1)
                    return result;
                //renaming
                askForRenaming(clickedTabId);
                return true;  //no further handling of this event is required
            }
        }
    } else if (receiver == this) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QRect clickable = tabBarRef->geometry();
            clickable.setRight(geometry().right());
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[MainTabs::eventFilter] nullptr MouseEvent";
                return true; // not supposed to happen anyway ..
            } else if (clickable.contains(me->pos())){ // only in the tabbar area
                newPreTab(guiHelper->getDefaultNewTab());
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == nullptr) {
                qCritical() << "[MainTabs::eventFilter] nullptr MouseEvent";
                return true; // not supposed to happen anyway ..
            } else if (me->buttons() == Qt::RightButton){ // only in the tabbar area
                QRect clickable = tabBarRef->geometry();
                clickable.setRight(geometry().right());
                if (clickable.contains(me->pos())) {
                    tabHeaderContextMenu->exec(this->mapToGlobal(me->pos()));
                    return true;
                }
            }

        }
    }
    return result;
}

LoggerWidget *MainTabs::getLogger() const
{
    return logger;
}

void MainTabs::askForRenaming(int index)
{
    if (index != indexOf(logger)) {
        TextInputDialog * dia = guiHelper->getNameDialog(this, tabBarRef->tabText(index));
        if (dia != nullptr) {
            int res = dia->exec();
            QString newName = dia->getInputText();
            if (res == QDialog::Accepted && !newName.isEmpty()) {
                static_cast<TabAbstract *>(widget(index))->setName(newName);
            }
            delete dia;
        }
    }
}

int MainTabs::integrateTab(TabAbstract *newTab)
{
    int nextInsert = -1;
    if (tabCount > maxTabCount) {
        logger->logError(tr("Reached the maximum number of allowed tabs"));
        return nextInsert;
    }

    nextInsert = count() - ((indexOf(logger) == -1) ? 0 : 1);
    connect(newTab, SIGNAL(askWindowTabSwitch()), this, SLOT(receivedTabWindowSwitch()), Qt::UniqueConnection);

    tabList.insert(newTab,false);

    QString tabName = newTab->getName();
    if (tabName.isEmpty()) {
        // some people have a fat mouse pointer and have difficulties ... so we need to add some padding ...
        tabName = tr("   %1  ").arg(tabCount);
        newTab->setName(tabName);
    }

    connect(newTab, SIGNAL(nameChanged()), this, SLOT(receivedNameChanged()), Qt::UniqueConnection);
    connect(newTab, SIGNAL(askBringFront()), this, SLOT(receivedBringToFront()), Qt::UniqueConnection);

    insertTab(nextInsert, newTab, tabName);
    setCurrentIndex(nextInsert);
    guiHelper->addTab(newTab);
    tabCount++;
    return nextInsert;
}

void MainTabs::clearTabs()
{
    QHashIterator<TabAbstract *, bool> i(tabList);
    while (i.hasNext()) {
        i.next();
        delete i.key();
    }

    guiHelper->clearDeletedTabs();
    tabList.clear();
    clearFloatingWindows();
    hideLogs();
}

void MainTabs::clearFloatingWindows()
{
    QHashIterator<TabAbstract *, FloatingDialog *> i(activeWindows);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    activeWindows.clear();
}

void MainTabs::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange) {
        QList<FloatingDialog *> list = activeWindows.values();
        bool enableValue = isEnabled();
        for (int i = 0; i < list.size(); i++) {
            list.at(i)->setEnabled(enableValue);
        }
    }
}

BaseStateAbstract *MainTabs::getStateMngtObj()
{
    MainTabsStateObj *mainTabStateObj = new(std::nothrow) MainTabsStateObj(this);
    if (mainTabStateObj == nullptr) {
        qFatal("Cannot allocate memory for MainTabsStateObj X{");
    }

    return mainTabStateObj;
}

bool MainTabs::isLoggerVisible()
{
    int index = indexOf(logger);
    return index != -1;
}

TabAbstract * MainTabs::newTab(ByteSourceAbstract *bs,GuiConst::TAB_TYPES type, int *index)
{
    TabAbstract *newtab = nullptr;

    if (tabCount > maxTabCount) {
        logger->logError(tr("Reached the maximum number of allowed tabs"));
        return newtab;
    }

    switch (type) {
        case GuiConst::TRANSFORM_TAB_TYPE:
            newtab = new(std::nothrow) TransformsGui(guiHelper, this);
            if (newtab == nullptr) {
                qFatal("Cannot allocate memory for TransformsGui X{");
            }
            break;
        case GuiConst::RANDOM_ACCESS_TAB_TYPE:
            newtab= new(std::nothrow) RandomAccessTab(bs,guiHelper,this);
            if (newtab == nullptr) {
                qFatal("Cannot allocate memory for RandomAccessTab X{");
            }
            break;
        case GuiConst::GENERIC_TAB_TYPE:
            newtab = new(std::nothrow) GenericTab(bs,guiHelper,this);
            if (newtab == nullptr) {
                qFatal("Cannot allocate memory for GenericTab X{");
            }
            break;
        default:
            qCritical() << tr("[newTab]Invalid tab type %1").arg(type);
    }

    if (newtab != nullptr) {
        int ret = integrateTab(newtab);
        if (index != nullptr)
            (*index) = ret;
    }

    return newtab;
}

TabAbstract *MainTabs::newPreTab(GuiConst::AVAILABLE_PRETABS preType, QByteArray initialData)
{
    TabAbstract * ta = nullptr;
    switch(preType) {
        case (TRANSFORM_PRETAB):
            ta = newTabTransform(initialData);
            break;
        case (HEXAEDITOR_PRETAB):
            ta = newHexEditorTab(initialData);
            break;
        case (LARGE_FILE_PRETAB):
            ta = newFileTab();
            break;
        case (CURRENTMEM_PRETAB):
            ta = newCurrentMemTab();
            break;
        case (PACKET_ANALYSER_PRETAB):
            ta = newPacketAnalyserTab();
            break;
        default:
            qCritical() << tr("[MainTabs::newPreTab] Invalid pretype value %1").arg(preType);
    }
    return ta;
}

TabAbstract *MainTabs::newTabTransform(const QByteArray &initialValue, const QString &conf)
{
    int ret = -1;

    TransformsGui *nt = static_cast<TransformsGui *>(newTab(nullptr, GuiConst::TRANSFORM_TAB_TYPE, &ret));
    if (nt != nullptr) {
        nt->setPreTabType(GuiConst::TRANSFORM_PRETAB);
        nt->setCurrentChainConf(conf);
        if (!initialValue.isEmpty())
            nt->setData(initialValue);

    }

    return nt;
}

TabAbstract * MainTabs::newFileTab(QString fileName)
{
    LargeFile *fs = new(std::nothrow) LargeFile();
    if (fs == nullptr) {
        qFatal("Cannot allocate memory for FileSource X{");
    }

    if (!fileName.isEmpty())
        fs->fromLocalFile(fileName);
    connect(fs,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);

    TabAbstract *ft = newTab(fs, GuiConst::RANDOM_ACCESS_TAB_TYPE);
    if (ft != nullptr) {
        ft->setPreTabType(GuiConst::LARGE_FILE_PRETAB);
        ft->setName(QFileInfo(fileName).fileName());
    }

    return ft;
}

TabAbstract * MainTabs::newHexEditorTab(QByteArray data)
{
    BasicSource * bs = new(std::nothrow) BasicSource();
    if (bs == nullptr) {
        qFatal("Cannot allocate memory for BasicSource X{");
    }
    connect(bs,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);

    TabAbstract *tab = newTab(bs, GuiConst::GENERIC_TAB_TYPE);
    if (tab != nullptr) {
        tab->setPreTabType(GuiConst::HEXAEDITOR_PRETAB);
        tab->setName(GuiConst::BASEHEX_TAB_STRING);
        tab->setData(data);
    }

    return tab;
}

TabAbstract * MainTabs::newCurrentMemTab()
{
    CurrentMemorysource *cms = new(std::nothrow) CurrentMemorysource();
    if (cms == nullptr) {
        qFatal("Cannot allocate memory for CurrentMemorysource X{");
    }
    connect(cms,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);

    TabAbstract *raTab = newTab(cms, GuiConst::RANDOM_ACCESS_TAB_TYPE);
    if (raTab != nullptr) {
        raTab->setPreTabType(GuiConst::CURRENTMEM_PRETAB);
        raTab->setName("Current memory");
    }

    return raTab;
}

TabAbstract *MainTabs::newPacketAnalyserTab()
{
    PacketAnalyserTab *pat =new(std::nothrow) PacketAnalyserTab(guiHelper);
    if (pat == nullptr) {
        qFatal("Cannot allocate memory for PacketAnalyserTab X{");
    }

    integrateTab(pat);
    pat->setName(tr("Packet analyser"));

    return pat;
}

void MainTabs::onDeleteTab(int index)
{
    if (index != indexOf(logger)) {
        TabAbstract *tgui = static_cast<TabAbstract *>(widget(index));
        // index is set by Qt, no need to verify

        removeTab(index);
        tabList.remove(tgui);
        guiHelper->removeTab(tgui);
        guiHelper->addDeletedTab(tgui);
      //  delete tgui;
        setCurrentIndex(index > 0 ? index - 1 : 0);
    } else {
        removeTab(index);
    }
}

void MainTabs::showLogs()
{
    int index = indexOf(logger);
    if (index == -1) {
        index = addTab(logger, GuiConst::LOGS_STR);
    }
    setCurrentIndex(index);
}

void MainTabs::hideLogs()
{
    int index = indexOf(logger);
    if (index != -1) {
        removeTab(index);
    }
}

void MainTabs::onLogError()
{
    int index = indexOf(logger);
    if (index == -1 && logger != nullptr) {
        index = addTab(logger, tr("Logs"));
    }
    tabBarRef->setTabTextColor(index, Qt::red);
}

void MainTabs::onLogCleanStatus()
{
    int index = indexOf(logger);
    if (index != -1) {
        tabBarRef->setTabTextColor(index, Qt::black);
    }
}

void MainTabs::receivedNameChanged()
{
    TabAbstract *tab = dynamic_cast<TabAbstract *>(sender());
    if (tab == nullptr) {
        qCritical() << tr("[MainTabs::receivedNameChanged] tab is null after type casting T_T");
        return;
    }
    QString name = tab->getName();
    if (name.isEmpty()) {
        logger->logWarning(tr("New name empty during renaming process, ignoring."), ID);
        return;
    }
    int index = indexOf(tab);
    if (index == -1) {
        if (activeWindows.contains(tab)) {
            activeWindows.value(tab)->setWindowTitle(name);
        } else {
            qWarning("[MainTabs::receivedNameChanged] Tab %d not found when renaming T_T",index);
            return;
        }
    } else {
        tabBarRef->setTabText(index,name);
    }
    // if we are here, then the name was updated sucessfully
    guiHelper->tabNameUpdated(tab);
}

void MainTabs::receivedTabWindowSwitch()
{
    TabAbstract *tab = static_cast<TabAbstract *>(sender());
    if (!tabList.contains(tab)) {
        qWarning("[MainTabs] Tab not found when switching window <-> tab T_T");
        return;
    } else {
        if (tabList.value(tab)) { // window -> tab
            addTab(tab, tab->getName());
            setCurrentWidget(tab);
            if (!activeWindows.contains(tab)) {
                qWarning("[MainTabs] Widget not found in the active windows (window -> tab) T_T");
            } else {
               FloatingDialog *fd = activeWindows.value(tab);
               activeWindows.remove(tab);
               delete fd;
            }
            tabList.insert(tab,false);

        } else { // tab -> window
            detachTab(tab);
        }
    }
}

void MainTabs::detachTab(TabAbstract *tab, QByteArray windowState)
{
    int index = indexOf(tab);
    if (index == -1) {
        qWarning("[MainTabs] Tab not found when switching tab -> window T_T");
    } else {
        removeTab(index);
        if (count() != 0)
            setCurrentIndex(0);
        FloatingDialog *fd = new(std::nothrow) FloatingDialog(guiHelper, tab);
        if (fd != nullptr) {
            activeWindows.insert(tab, fd);
            fd->setWindowTitle(tab->getName());
            if (!windowState.isEmpty())
                fd->restoreGeometry(windowState);
            fd->raise();
            fd->show();
            tabList.insert(tab,true);
            connect(fd,SIGNAL(rejected()), this, SLOT(onFloatingWindowsReject()));
        } else {
            qFatal("Cannot allocate memory for FloatingDialog X{");
        }
    }
}

void MainTabs::receivedBringToFront()
{
    TabAbstract *tab = static_cast<TabAbstract *>(sender());
    if (!tabList.contains(tab)) {
        qWarning("[MainTabs] Tab not found when bringing to front T_T");
        return;
    } else {
        if (tabList.value(tab)) { // windows
            if (!activeWindows.contains(tab)) {
                qWarning("[MainTabs] Widget not found in the active windows (bringToFront) T_T");
            } else {
                FloatingDialog *fd = activeWindows.value(tab);
                fd->raise();
                fd->show();
            }
        } else { // tab
            setCurrentWidget(tab);
        }
    }
}

void MainTabs::onFloatingWindowsReject()
{
    FloatingDialog *fd = dynamic_cast<FloatingDialog *>(sender());
    if (fd != nullptr) {
        TabAbstract *tab = activeWindows.key(fd,nullptr);
        if (tab != nullptr) {
            addTab(tab, tab->getName());
            setCurrentWidget(tab);
            activeWindows.remove(tab);
            delete fd;
            tabList.insert(tab,false);
        } else {
           qWarning("[MainTabs] Tab not found  in the active windows tab (reject) T_T");
        }
    }
}

void MainTabs::updateTabHeaderMenu()
{
    tabHeaderContextMenu->clear();

    QMenu * newTabMenu = guiHelper->getNewTabMenu();
    if (newTabMenu != nullptr) {
        tabHeaderContextMenu->addMenu(newTabMenu);
    }

    QList<TabAbstract *> list = guiHelper->getDeletedTabs();
    if (list.size() > 0) {
        if (newTabMenu != nullptr) {
            tabHeaderContextMenu->addSeparator();
        }
        QAction * action = new(std::nothrow) QAction(tr("Deleted Tabs"), tabHeaderContextMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action for \"deleted tab\" X{");
            return;
        }
        action->setDisabled(true);
        tabHeaderContextMenu->addAction(action);

        for (int i = 0; i < list.size(); i++) {
            action = new(std::nothrow) QAction(list.at(i)->getName(), tabHeaderContextMenu);
            if (action == nullptr) {
                qFatal("Cannot allocate memory for action for deletedTabContextMenu X{");
                return;
            }
            tabHeaderContextMenu->addAction(action);
        }
    }
}

void MainTabs::onDeletedTabSelected(QAction *action)
{
    if (tabCount > maxTabCount) {
        logger->logError(tr("Reached the maximum number of allowed tabs, cannot revive the deleted tab."));
        return;
    }

    QList<QAction*> actions = tabHeaderContextMenu->actions();
    int index = actions.indexOf(action);
    if (index == -1) // need to do that because the actions from the submenu seem to be arriving here as well ???
        return;
    if (index > 2) {
        index = index - 3; // decreasing the index due to newMenu + title + separator
        guiHelper->reviveTab(index);
    } else {
        qCritical() << tr("[MainTabs::onDeletedTabSelected] index not found T_T");
    }
}


MainTabsStateObj::MainTabsStateObj(MainTabs *target) :
    mtabs(target)
{
    setName(mtabs->metaObject()->className());
}

MainTabsStateObj::~MainTabsStateObj()
{

}

void MainTabsStateObj::run()
{

    MainTabsClosingObj *closingState = new(std::nothrow) MainTabsClosingObj(mtabs);
    if (closingState == nullptr) {
        qFatal("Cannot allocate memory for MainTabsClosingObj X{");
    }

    emit addNewState(closingState);

    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeStartElement(GuiConst::STATE_TABS_ARRAY);
        writer->writeAttribute(GuiConst::STATE_MAINTABS_LOGGER, write(mtabs->isLoggerVisible()));
        writer->writeAttribute(GuiConst::STATE_CURRENT_INDEX, write(mtabs->currentIndex()));

        int listSize = mtabs->count();


        QWidget *logger = mtabs->getLogger();
        writer->writeAttribute(GuiConst::STATE_LOGGER_INDEX, write(mtabs->indexOf(logger)));
        int numberOfTabs = listSize - (mtabs->indexOf(logger) != -1 ? 1 : 0);
        QList<TabAbstract *> windowedTabs = mtabs->activeWindows.keys();
        numberOfTabs += windowedTabs.count();
        writer->writeAttribute(GuiConst::STATE_SIZE, write(numberOfTabs));


        for (int i = listSize - 1; i > -1; i--) { // need to run in reverse due to the stack structure
            QWidget * w = mtabs->widget(i);
            if (w == logger) {
                continue;
            }

            TabAbstract *tab = static_cast<TabAbstract *>(w);
            if (tab == nullptr) {
                qCritical() << tr("[MainTabs::saveState] Error while casting widget");
                continue;
            }

            BaseStateAbstract *tempS = tab->getStateMngtObj();
            if (tempS != nullptr) {
                emit addNewState(tempS);
            }
        }

        listSize = windowedTabs.count();

        for (int i = 0; i < listSize; i++) {

            TabAbstract *tab = windowedTabs.at(i);

            BaseStateAbstract *tempS = tab->getStateMngtObj();
            if (tempS != nullptr) {
                TabStateObj *tabState = static_cast<TabStateObj *>(tempS);

                tabState->setIsWindowed(true);
                tabState->setWindowState( mtabs->activeWindows.value(tab)->saveGeometry());
                emit addNewState(tempS);
            }
        }


    } else if (readNextStart(GuiConst::STATE_TABS_ARRAY)) {

        bool ok = false;
        mtabs->clearTabs();
        QXmlStreamAttributes attrList = reader->attributes();

        if (attrList.hasAttribute(GuiConst::STATE_MAINTABS_LOGGER)) {
            closingState->setShowlogs(readBool( attrList.value(GuiConst::STATE_MAINTABS_LOGGER)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_CURRENT_INDEX)) {
            int index = readInt(attrList.value(GuiConst::STATE_CURRENT_INDEX),&ok);
            if (ok)
                closingState->setCurrentIndex(index);
        }

        if (attrList.hasAttribute(GuiConst::STATE_LOGGER_INDEX)) {
            int index = readInt(attrList.value(GuiConst::STATE_LOGGER_INDEX),&ok);
            if (ok)
                closingState->setLogsIndex(index);
        }

        if (attrList.hasAttribute(GuiConst::STATE_SIZE)) {
            int listSize = readInt(attrList.value(GuiConst::STATE_SIZE), &ok);

            if (ok && (listSize > 0)) {
                for (int i = 0; i < listSize; i++) {
                    InitTabStateObj * initTab = new(std::nothrow) InitTabStateObj(mtabs);
                    if (initTab == nullptr) {
                        qFatal("Cannot allocate memory for InitTabStateObj X{");
                    }
                    connect(initTab, SIGNAL(detachTab(TabAbstract*,QByteArray)), mtabs, SLOT(detachTab(TabAbstract*,QByteArray)), Qt::QueuedConnection);
                    connect(initTab, SIGNAL(newTab(TabAbstract*)), mtabs, SLOT(integrateTab(TabAbstract*)), Qt::QueuedConnection);

                    emit addNewState(initTab);
                }
            }
        }

    }
}

InitTabStateObj::InitTabStateObj(MainTabs *target) :
    mtabs(target)
{
    name = metaObject()->className();
}

InitTabStateObj::~InitTabStateObj()
{

}

void InitTabStateObj::run()
{
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        qCritical() << tr("InitTabStateObj should not be used for saving T_T");
    } else {
        if (readNextStart(GuiConst::STATE_TAB)) {
            QXmlStreamAttributes attrList = reader->attributes();
            bool ok = false;
            int pretype = readInt(attrList.value(GuiConst::STATE_PRETAB_TYPE), &ok);
            if (ok && pretype >= 0 && pretype < GuiConst::AVAILABLE_TAB_STRINGS.size()) {
                TabAbstract *tab = mtabs->newPreTab((GuiConst::AVAILABLE_PRETABS)pretype);
                if (tab != nullptr) {
                    emit newTab(tab);
                    BaseStateAbstract *tabstate = tab->getStateMngtObj();
                    emit addNewState(tabstate);
                    if (attrList.hasAttribute(GuiConst::STATE_WINDOWED)) {
                        QString value = attrList.value(GuiConst::STATE_WINDOWED).toString();
                        bool ok = false;
                        int ivalue = value.toInt(&ok);
                        if (ok) {
                            if (ivalue == TabAbstract::WINDOWED_TAB) {
                                QByteArray geometry = readByteArray(attrList.value(GuiConst::STATE_WIDGET_GEOM));
                                emit detachTab(tab, geometry);
                            }
                        }
                    }
                } else {
                    qCritical() << tr("Returned tab is nullptr T_T");
                }
            } else {
                qCritical() << tr("Pretab value is invalid T_T");
            }
        }
    }
}


MainTabsClosingObj::MainTabsClosingObj(MainTabs *target) :
    ClosingState()
{
    mtabs = target;
    showlogs = false;
    currentIndex = 0;
    logsIndex = -1;
}

MainTabsClosingObj::~MainTabsClosingObj()
{

}

void MainTabsClosingObj::run()
{
    // closing the tabs array
    genCloseElement();
    // normal close for maintabs
    ClosingState::run();

    // last configuration bits for the maintabs
    if (!(flags & GuiConst::STATE_SAVE_REQUEST)) {
        if (showlogs) {
            if (logsIndex < 0 || logsIndex >= mtabs->count()) {
                mtabs->addTab(mtabs->logger, GuiConst::LOGS_STR);
            } else {
                mtabs->insertTab(logsIndex,mtabs->logger,GuiConst::LOGS_STR);
            }
        }
        else
            mtabs->hideLogs();

        if (currentIndex > 0)
            mtabs->setCurrentIndex(currentIndex);
        else
            mtabs->setCurrentIndex(0);
    }
}

void MainTabsClosingObj::setShowlogs(bool value)
{
    showlogs = value;
}

void MainTabsClosingObj::setCurrentIndex(int value)
{
    currentIndex = value;
}

void MainTabsClosingObj::setLogsIndex(int value)
{
    logsIndex = value;
}



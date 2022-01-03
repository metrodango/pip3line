/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MAINTABS_H
#define MAINTABS_H

#include <QTabWidget>
#include <QDialog>
#include <QTabBar>
#include <QHash>
#include <QNetworkAccessManager>
#include <transformmgmt.h>
#include "sources/bytesourceabstract.h"
#include "guihelper.h"
#include "tabs/tababstract.h"
#include "loggerwidget.h"
#include "floatingdialog.h"
#include <QDragMoveEvent>
#include <QPoint>
#include "shared/guiconst.h"
#include "state/basestateabstract.h"
#include "state/closingstate.h"
#include <pipelinecommon.h>
using namespace Pip3lineCommon;

class QAction;

class MainTabs : public QTabWidget
{
        Q_OBJECT
    public:
        explicit MainTabs(GuiHelper *guiHelper);
        ~MainTabs();
        void askForRenaming(int index);
        void clearTabs();
        BaseStateAbstract *getStateMngtObj();
        bool isLoggerVisible();
        LoggerWidget *getLogger() const;

    public slots:
        TabAbstract *newTab(ByteSourceAbstract *bytesource, GuiConst::TAB_TYPES type, int *index = nullptr);
        TabAbstract *newPreTab(GuiConst::AVAILABLE_PRETABS preType, QByteArray initialData = QByteArray());
        TabAbstract *newTabTransform(const QByteArray &initialValue = QByteArray(), const QString &conf = QString());
        TabAbstract *newFileTab(QString fileName = QString());
        TabAbstract *newHexEditorTab(QByteArray data = QByteArray());
        TabAbstract *newCurrentMemTab();
        TabAbstract *newPacketAnalyserTab(QList<QSharedPointer<Packet> > packets = QList<QSharedPointer<Packet> >());
        TabAbstract *newPacketAnalyserTab(const QByteArray config, const QString tabname = QString());
        int integrateTab(TabAbstract * newTab);
        void onDeleteTab(int index);
        void showLogs();
        void hideLogs();
    private slots:
        void onLogError();
        void onLogCleanStatus();
        void receivedNameChanged();
        void receivedTabWindowSwitch();
        void detachTab(TabAbstract * tab, QByteArray windowState = QByteArray());
        void receivedBringToFront();
        void onFloatingWindowsReject();
        void updateTabHeaderMenu();
        void onDeletedTabSelected(QAction * action);
    private:
        Q_DISABLE_COPY(MainTabs)
        static const QString ID;
        bool eventFilter(QObject * receiver, QEvent * event);
        void clearFloatingWindows();
        void changeEvent(QEvent * event);
        QString generateUniqueName(const QString & name);

        QTabBar * tabBarRef;
        GuiHelper *guiHelper;
        LoggerWidget * logger;
        QHash<TabAbstract *,bool> tabList;
        QHash<TabAbstract *, FloatingDialog *> activeWindows;
        uint tabCount;
        uint maxTabCount;
        QMenu * tabHeaderContextMenu;
        static const QString STATE_LOGGER_TAB;

        friend class MainTabsStateObj;
        friend class MainTabsClosingObj;
};

class MainTabsStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit MainTabsStateObj(MainTabs *target);
        ~MainTabsStateObj();
        void run();
    private:
        Q_DISABLE_COPY(MainTabsStateObj)
        MainTabs *mtabs;
};

class InitTabStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit InitTabStateObj(MainTabs *target);
        ~InitTabStateObj();
        void run();
    signals:
        void newTab(TabAbstract * tab);
        void detachTab(TabAbstract * tab, QByteArray state);
    private:
        Q_DISABLE_COPY(InitTabStateObj)
        MainTabs *mtabs;
};

class MainTabsClosingObj : public ClosingState
{
        Q_OBJECT
    public:
        explicit MainTabsClosingObj(MainTabs *target);
        ~MainTabsClosingObj();
        void run();

        void setShowlogs(bool value);
        void setCurrentIndex(int value);
        void setLogsIndex(int value);

    private:
        Q_DISABLE_COPY(MainTabsClosingObj)
        MainTabs *mtabs;
        bool showlogs;
        int currentIndex;
        int logsIndex;
};

#endif // MAINTABS_H

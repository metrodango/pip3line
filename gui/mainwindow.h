/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <transformmgmt.h>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPushButton>
#include <QPoint>
#include <QNetworkAccessManager>
#include "sources/blocksources/blockssource.h"
#include "shared/guiconst.h"
#include "state/basestateabstract.h"

namespace Ui {
class MainWindow;
}

class DebugDialog;
class ComparisonDialog;
class QuickViewDialog;
class SettingsDialog;
class AnalyseDialog;
class RegExpHelpDialog;
class MainTabs;
class LoggerWidget;
class TransformMgmt;
class GuiHelper;
class DownloadManager;
class BlocksSource;
class TabAbstract;
class StateOrchestrator;
class StateDialog;
class StateStatusWidget;

#ifdef Q_OS_LINUX
class QSocketNotifier;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:
        explicit MainWindow(bool debug = false, QWidget *parent = 0);
        ~MainWindow();
        void loadFile(QString fileName);

        SettingsDialog *getSettingsDialog() const;
        AnalyseDialog *getAnalyseDialog() const;
        RegExpHelpDialog *getRegexphelpDialog() const;
        QuickViewDialog *getQuickView() const;
        ComparisonDialog *getComparisonView() const;
        DebugDialog *getDebugDialog() const;
        MainTabs *getMainTabs() const;

#ifdef Q_OS_LINUX
        static int sigFd[2];
        static void exitSignalHandler(int signal);
#endif

    signals:
        void exiting();
    public slots:
        void onAboutPip3line();
        void onAnalyse(bool checked);
        void onHelpWithRegExp();
        void onQuickView(bool checked);
        void onCompare(bool checked);
        void onDebug();
        void onSettingsDialogOpen(bool checked);

    private slots:
        void checkForUpdates();
        void processingCheckForUpdate(QByteArray data);
        void processingUrlDownload(QByteArray data);
        void iconActivated(QSystemTrayIcon::ActivationReason reason);
        void onImport(QAction* action);
        void onDataFromURL();
        void on_actionLogs_triggered();
        void updateTrayIcon();
        void onDebugDestroyed();
        void onNewAction(QAction * action);
        void onNewDefault(QByteArray initialData = QByteArray());
        void showWindow();
        void onExternalBlockReceived(Block *block);
        void onSaveState();
        void onLoadState();
        void onSaveLoadFinished();
        void autoSave();
        void autoRestore();
        void onExit();
        void cleaningAndExit();
        void handleUnixSignal();
        void onClearMemory();
    private:
        Q_DISABLE_COPY(MainWindow)
        void buildToolBar();
        void initializeLibTransform();
        void closeEvent(QCloseEvent *event);
        void createTrayIcon();

        void hideEvent(QHideEvent * event);
        void showEvent(QShowEvent * event);

        void saveLoadState(QString filename, quint64 flags);

        Ui::MainWindow *ui;

        SettingsDialog *settingsDialog;
        AnalyseDialog *analyseDialog;
        RegExpHelpDialog *regexphelpDialog;
        QuickViewDialog * quickView;
        ComparisonDialog *comparisonView;
        DebugDialog * debugDialog;

        LoggerWidget *logger;
        MainTabs * mainTabs;

        GuiHelper *guiHelper;
        TransformMgmt *transformFactory;
        QNetworkAccessManager networkManager;
        QSettings *settings;
        StateOrchestrator * stateOrchestrator;
        StateDialog *stateDialog;
        StateStatusWidget * stateStatusWidget;

        QSystemTrayIcon *trayIcon;
        QMenu *trayIconMenu;
        QMenu *newMenu;
        QAction *trayIconLabel;

        QAction * newTransformTabAction;
        QAction * newLargeFileTabAction;
        QAction * newHexeditorTabAction;
        QAction * newCurrentMemTabAction;
        QAction * newPacketAnalyserTabAction;

        QAction * newAction;

        bool quickViewWasVisible;
        bool settingsWasVisible;
        bool compareWasVisible;
        QPoint savedPos;
        static bool appExiting;

#ifdef Q_OS_LINUX
        QSocketNotifier *snExit;
#endif

        friend class ClearAllStateObj;
};

class MainWinStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit MainWinStateObj(MainWindow *target);
        ~MainWinStateObj();
        void run();
    private:
        MainWindow *mwin;
        static const QString NAME;
};


class GlobalConfStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit GlobalConfStateObj(TransformMgmt *transformMgmt);
        ~GlobalConfStateObj();
        void run();
    signals:
        void settingsUpdated();
    private :
        TransformMgmt *transformMgmt;
};

class ClearAllStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit ClearAllStateObj(MainWindow *target);
        ~ClearAllStateObj();
        void run();
    private :
        MainWindow *mwin;
};

#endif // MAINWINDOW_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debugdialog.h"
#include <QTextStream>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QXmlStreamWriter>
#include "aboutdialog.h"
#include "messagedialog.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QRect>
#include <QRegExp>
#include <QSettings>
#include <QIcon>
#include <QClipboard>
#include <QRgb>
#include <QDebug>
#include <QColorDialog>
#include <transformabstract.h>
#include <QStatusBar>
#include <commonstrings.h>
#include <QFileDialog>
#include <QPalette>
#include <QTemporaryFile>
#include <QDebug>
#include "sources/currentmemorysource.h"
#include "sources/largefile.h"
#include "sources/basicsource.h"
#include "../version.h"
#include "analysedialog.h"
#include "massprocessingdialog.h"
#include "regexphelpdialog.h"
#include "loggerwidget.h"
#include "settingsdialog.h"
#include "downloadmanager.h"
#include "guihelper.h"
#include "maintabs.h"
#include "quickviewdialog.h"
#include "comparisondialog.h"
#include "shared/guiconst.h"
#include "state/stateorchestrator.h"
#include "state/closingstate.h"
#include "state/statedialog.h"
#include "state/statestatuswidget.h"
#include "sources/blocksources/connectiondetails.h"

#ifdef Q_OS_LINUX
#include <QSocketNotifier>
#include <sys/socket.h>
#include <csignal>
#include "unistd.h"
#endif


using namespace GuiConst;

#ifdef Q_OS_LINUX
int MainWindow::sigFd[2];
#endif

bool MainWindow::appExiting = false;

MainWindow::MainWindow(bool debug, QWidget *parent) :
    QMainWindow(parent)
{
    analyseDialog = nullptr;
    regexphelpDialog = nullptr;
    settingsDialog = nullptr;
    trayIcon = nullptr;
    quickView = nullptr;
    comparisonView = nullptr;
    debugDialog = nullptr;
    trayIconLabel = nullptr;
    newMenu = nullptr;
    trayIconMenu = nullptr;
    transformFactory = nullptr;

    newTransformTabAction = nullptr;
    newLargeFileTabAction = nullptr;
    newHexeditorTabAction = nullptr;
    newCurrentMemTabAction = nullptr;

    newAction = nullptr;

    stateOrchestrator = nullptr;
    stateDialog = nullptr;
    stateStatusWidget = nullptr;

    settingsWasVisible = false;
    quickViewWasVisible = false;
    compareWasVisible = false;

#ifdef Q_OS_LINUX
    // handling signals in linux
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, MainWindow::sigFd))
            qFatal("Couldn't create signal socketpair");
    snExit = new(std::nothrow) QSocketNotifier(MainWindow::sigFd[1],QSocketNotifier::Read, this);
    connect(snExit, SIGNAL(activated(int)), this, SLOT(handleUnixSignal()));

    if (snExit == nullptr) {
        qFatal("Cannot allocate memory for QSocketNotifier X{");
    }
#endif

    ui = new(std::nothrow) Ui::MainWindow();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::MainWindow X{");
    }

    qRegisterMetaType<Pip3lineConst::LOGLEVEL>("Pip3lineConst::LOGLEVEL");
    qRegisterMetaType<quintptr>("quintptr");
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<BytesRangeList>("BytesRangeList");
    qRegisterMetaType<QSslSocket::SslMode>("QSslSocket::SslMode");
    qRegisterMetaType<ConnectionDetails>("ConnectionDetails");

#if QT_VERSION >= 0x050000
    qRegisterMetaType<qintptr>("qintptr");
#endif

    qApp->setOrganizationName(APPNAME);
    qApp->setApplicationName(APPNAME);

    ui->setupUi(this);

    // change selection colors so that they appear the same whether they are enable/active or not
    QPalette palette = QApplication::palette();
    QColor originalb = palette.color(QPalette::Active,QPalette::Highlight);
    QColor originalf = palette.color(QPalette::Active,QPalette::HighlightedText);
    palette.setColor(QPalette::Disabled,QPalette::Highlight, originalb);
    palette.setColor(QPalette::Inactive,QPalette::Highlight, originalb);
    palette.setColor(QPalette::Disabled,QPalette::HighlightedText, originalf);
    palette.setColor(QPalette::Inactive,QPalette::HighlightedText, originalf);
    QApplication::setPalette(palette);

    // special stylesheet for the hexadecimal editor cells
//    qApp->setStyleSheet("TextCell {background: white}");

    logger = new(std::nothrow) LoggerWidget();
    if (logger == nullptr) {
        qFatal("Cannot allocate memory for LoggerWidget X{");
    }


#ifdef Q_OS_WIN // some pre-initialization on windows platform
    QByteArray newPATH;
    // looks for Python libraries path

    QStringList supportedPythonVersion;
    supportedPythonVersion << "2.7" << "3.4" << "3.5";

    // yet another Windows Python mess
#ifdef _WIN64
    supportedPythonVersion << "2.7-64" << "3.5-64";
#else
    supportedPythonVersion << "2.7-32" << "3.5-32";
#endif

    QStringList pythonRegistry;
    pythonRegistry << QString("HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\");
    pythonRegistry << QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\");
    for (int k = 0; k < pythonRegistry.size(); k++) {
        QSettings setting(pythonRegistry.at(k), QSettings::NativeFormat);
        QStringList list = setting.childGroups();
        for (int i = 0; i < list.size(); i++ ) {
            if (supportedPythonVersion.contains(list.at(i))) {
                setting.beginGroup(list.at(i));
                QStringList groups = setting.childGroups();
                for (int j = 0; j < groups.size(); j++ ) {
                    if (groups.at(j) == QString("InstallPath")) {
                        setting.beginGroup(groups.at(j));
                        qDebug() << "Found Python" << list.at(i);
                        newPATH.append(setting.value(".").toString().toUtf8()).append(";");
                        setting.endGroup();
                    }
                }
                setting.endGroup();
            }
        }
    }
    newPATH.chop(1); // removing potential semi-colon
    qputenv("PATH", newPATH);

#endif

    initializeLibTransform();

    guiHelper = new(std::nothrow) GuiHelper(transformFactory,&networkManager, logger);
    if (guiHelper == nullptr) {
        qFatal("Cannot allocate memory for GuiHelper X{");
    }
    guiHelper->setDebugMode(debug);

    buildToolBar();

    if (debug)
        ui->actionDebug_dialog->setVisible(true);
    else
        ui->actionDebug_dialog->setVisible(false);

    mainTabs = new(std::nothrow) MainTabs(guiHelper, this);
    if (mainTabs == nullptr) {
        qFatal("Cannot allocate memory for MainTabs X{");
    }
    ui->centralWidget->layout()->addWidget(mainTabs);

    quickViewWasVisible = false;
    settingsWasVisible = false;

    createTrayIcon();

    settings = transformFactory->getSettingsObj();
    if (settings->value(SETTINGS_AUTO_UPDATE, true).toBool()) {
        checkForUpdates();
    }

    onNewDefault();

    BlocksSource * blockListener = guiHelper->getIncomingBlockListener();

    connect(blockListener, SIGNAL(blockReceived(Block *)), SLOT(onExternalBlockReceived(Block *)),Qt::QueuedConnection);
    connect(blockListener,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)));
    QTimer::singleShot(0,blockListener,SLOT(startListening()));
    connect(this, SIGNAL(exiting()), blockListener, SLOT(stopListening()), Qt::QueuedConnection);

    connect(guiHelper,SIGNAL(raiseWindowRequest()), SLOT(showWindow()));
    connect(guiHelper, SIGNAL(newTabRequested(QByteArray)), this, SLOT(onNewDefault(QByteArray)));
    connect(ui->actionSave_State, SIGNAL(triggered()), SLOT(onSaveState()));
    connect(ui->actionLoad_State, SIGNAL(triggered()), SLOT(onLoadState()));

    connect(guiHelper, SIGNAL(requestSaveState()), this, SLOT(autoSave()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onExit()));

    connect(ui->actionSave_as_D_efault, SIGNAL(triggered(bool)), this, SLOT(autoSave()));

    if (guiHelper->getAutoRestoreOnStartup())
        QTimer::singleShot(0,this, SLOT(autoRestore()));

    connect(ui->actionClear_memory, SIGNAL(triggered()), SLOT(onClearMemory()));

//  qApp->setStyleSheet("* {color : green; background: black}");
}

MainWindow::~MainWindow()
{
    //qDebug() << "Destroying main window";

    guiHelper->blockSignals(true); // complicated things happen otherwise ...

    if (stateOrchestrator != nullptr) {
        delete stateOrchestrator;
        stateOrchestrator = nullptr;
    }
    delete debugDialog;
    delete newMenu;
    delete quickView;
    delete comparisonView;// these two first to avoid weird bugs
    delete mainTabs;
    delete trayIconMenu;
    logger = nullptr; // no need to delete, already done by the main tab gui
    delete analyseDialog;
    delete regexphelpDialog;
    delete trayIcon;
    delete settings;
    delete guiHelper;
    delete transformFactory;
    delete settingsDialog;

    delete ui;
}

void MainWindow::buildToolBar()
{
    QPushButton * newPushButton = new(std::nothrow) QPushButton(QIcon(":/Images/icons/document-new-3.png"),"",this);
    if (newPushButton == nullptr) {
        qFatal("Cannot allocate memory for newPushButton X{");
    }

    newPushButton->setToolTip(tr("All tabs type"));

    newMenu = new(std::nothrow) QMenu(NEW_TAB_STRING);
    if (newMenu == nullptr) {
        qFatal("Cannot allocate memory for newPushButton X{");
    }

    connect(ui->actionNewDefault, SIGNAL(triggered()), SLOT(onNewDefault()));

    newTransformTabAction = new(std::nothrow) QAction(TRANSFORM_TAB_STRING, newMenu);
    if (newTransformTabAction == nullptr) {
        qFatal("Cannot allocate memory for QAction NEW_DEFAULT X{");
    }
    newMenu->addAction(newTransformTabAction);

    newHexeditorTabAction = new(std::nothrow) QAction(BASEHEX_TAB_STRING, newMenu);
    if (newHexeditorTabAction == nullptr) {
        qFatal("Cannot allocate memory for QAction NEW_BASEHEX X{");
    }
    newMenu->addAction(newHexeditorTabAction);

    newLargeFileTabAction = new(std::nothrow) QAction(LARGE_FILE_TAB_STRING, newMenu);
    if (newLargeFileTabAction == nullptr) {
        qFatal("Cannot allocate memory for QAction NEW_FILE X{");
    }
    newMenu->addAction(newLargeFileTabAction);

    newPacketAnalyserTabAction = new(std::nothrow) QAction(PACKET_ANALYSER_TAB_STRING, newMenu);
    if (newPacketAnalyserTabAction == nullptr) {
        qFatal("Cannot allocate memory for QAction NEW_PACKET_ANALYSER X{");
    }
    newMenu->addAction(newPacketAnalyserTabAction);

#ifndef Q_OS_OSX // memory view not implemented for OS X
    if (guiHelper->getDebugMode()) {
        newCurrentMemTabAction = new(std::nothrow) QAction(CURRENTMEM_TAB_STRING, newMenu);
        if (newCurrentMemTabAction == nullptr) {
            qFatal("Cannot allocate memory for QAction NEW_CURRENTMEM X{");
        }
        newMenu->addAction(newCurrentMemTabAction);
    }
#endif

    newMenu->setIcon(QIcon(":/Images/icons/document-new-3.png"));

    newPushButton->setMenu(newMenu);
    newPushButton->setFlat(true);

    ui->menuFile->insertMenu(ui->actionNewDefault, newMenu);
    connect(newMenu, SIGNAL(triggered(QAction*)), SLOT(onNewAction(QAction*)));

    guiHelper->setNewTabMenu(newMenu);

    ui->mainToolBar->insertWidget(ui->mainToolBar->actions().at(1),newPushButton );
   // ui->mainToolBar->insertSeparator(ui->action_Analyse);

    connect(ui->actionHelp_with_RegExp, SIGNAL(triggered()), this, SLOT(onHelpWithRegExp()));
    connect(ui->actionAbout_Pip3line, SIGNAL(triggered()), this, SLOT(onAboutPip3line()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionPip3line_settings, SIGNAL(toggled(bool)), this, SLOT(onSettingsDialogOpen(bool)));
    connect(ui->actionMagic, SIGNAL(toggled(bool)), this, SLOT(onQuickView(bool)));
    connect(ui->actionDebug_dialog, SIGNAL(triggered()), this, SLOT(onDebug()));
    connect(ui->actionCompare, SIGNAL(toggled(bool)), this, SLOT(onCompare(bool)));
    connect(ui->action_Analyse, SIGNAL(toggled(bool)), SLOT(onAnalyse(bool)));

}

void MainWindow::initializeLibTransform()
{
    if (transformFactory == nullptr) {
        transformFactory = new(std::nothrow) TransformMgmt();
        if (transformFactory == nullptr) {
            qFatal("Cannot allocate memory for the Transform Factory X{");
        }
        connect(transformFactory,SIGNAL(error(QString, QString)),logger,SLOT(logError(QString,QString)), Qt::UniqueConnection);
        connect(transformFactory,SIGNAL(warning(QString,QString)),logger,SLOT(logWarning(QString,QString)), Qt::UniqueConnection);
        connect(transformFactory,SIGNAL(status(QString,QString)),logger,SLOT(logStatus(QString,QString)), Qt::UniqueConnection);
    }

    transformFactory->initialize(QCoreApplication::applicationDirPath());
}

void MainWindow::loadFile(QString fileName)
{
    TabAbstract * tab = mainTabs->newHexEditorTab();
    if (tab != nullptr)
        tab->loadFromFile(fileName);
}

void MainWindow::onAboutPip3line()
{
    AboutDialog dialog(guiHelper, this);
    dialog.exec();
}

void MainWindow::onHelpWithRegExp()
{
    if (!regexphelpDialog) {
        regexphelpDialog = new(std::nothrow) RegExpHelpDialog(guiHelper, this);
        if (regexphelpDialog == nullptr) {
            qFatal("Cannot allocate memory for regexphelpDialog X{");
            return;
        }
    }

    regexphelpDialog->show();
    regexphelpDialog->raise();
    regexphelpDialog->activateWindow();
}

void MainWindow::onSettingsDialogOpen(bool checked)
{
    if (checked) {
        if (settingsDialog == nullptr) {
            settingsDialog = new(std::nothrow) SettingsDialog(guiHelper, this);
            if (settingsDialog == nullptr) {
                qFatal("Cannot allocate memory for settingsDialog X{");
                return;
            }
            connect(settingsDialog, SIGNAL(updateCheckRequested()), this, SLOT(checkForUpdates()), Qt::QueuedConnection);
            connect(guiHelper, SIGNAL(globalUpdates()), settingsDialog, SLOT(initializeConf()), Qt::QueuedConnection);
            connect(settingsDialog, SIGNAL(forceAutoSave()), SLOT(autoSave()),Qt::QueuedConnection);
            settingsDialog->attachAction(ui->actionPip3line_settings);
        }

        settingsDialog->show();
        settingsDialog->raise();
    } else if (settingsDialog != nullptr) {
        settingsDialog->hide();
    }
}

void MainWindow::checkForUpdates()
{
    QUrl resource(UPDATE_URL);
    DownloadManager * dm = new(std::nothrow) DownloadManager(resource, guiHelper);
    if (dm == nullptr) {
        qFatal("Cannot allocate memory for DownloadManager (updates) X{");
        return;
    }

    connect(dm, SIGNAL(finished(QByteArray)), this, SLOT(processingCheckForUpdate(QByteArray)));
    guiHelper->requestDownload(resource, nullptr,dm);
}

void MainWindow::processingCheckForUpdate(QByteArray bdata)
{
    QString data = QString::fromUtf8(bdata.constData(),bdata.size());
    QString message = "Unknown";

    QRegExp versionRegExp("^([0-9]{1,2})\\.([0-9]{1,2})\\.?([0-9]{0,2})");
    if (data.isEmpty()) {
        message = tr("Empty network response, cannot check for new Pip3line version");
        logger->logWarning(message);
    } else {

        int pos = versionRegExp.indexIn(data,0);

        if (pos != -1) {
            uint major = versionRegExp.cap(1).toUInt();
            uint minor = versionRegExp.cap(2).toUInt();
            uint rev = 0;
            if (versionRegExp.captureCount() > 2)
                rev = versionRegExp.cap(3).toUInt();

            qDebug() << tr("Remote Version %1.%2.%3").arg(major).arg(minor).arg(rev);
            qDebug() << tr("Current Version %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_REV);

            if (major > VERSION_MAJOR ||
                    (major == VERSION_MAJOR && minor > VERSION_MINOR) ||
                    (major == VERSION_MAJOR && minor == VERSION_MINOR && rev > VERSION_REV)) {
                message = tr("Current: %1\nLatest: %2.%3.%4\n\nNew version of Pip3line available at\n \n%5")
                          .arg(QString(VERSION_STRING))
                          .arg(major)
                          .arg(minor)
                          .arg(rev)
                          .arg(RELEASES_URL);
                QMessageBox::warning(this,tr("New update"),message,QMessageBox::Ok);
            } else {
                message = tr("Latest version of Pip3line in use (%1), everything is fine").arg(QString(VERSION_STRING));
                logger->logStatus(message);

            }
        } else {
            message = tr("Got a network response but cannot find the version string.");
            logger->logWarning(message,"Version Check");
        }
    }

    if (settingsDialog != nullptr)
        settingsDialog->setVersionUpdateMessage(message);
}

void MainWindow::processingUrlDownload(QByteArray data)
{
    if (data.isEmpty()) {
        data = "No data from URL";
        mainTabs->newTabTransform(data);
    } else {
        mainTabs->newTabTransform(data);
    }
    showWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool minimize = settings->value(SETTINGS_MINIMIZE_TO_TRAY, true).toBool();
    if (minimize && trayIcon != nullptr) {
        guiHelper->goIntoHidding();
        hide();
        event->ignore();
    } else {
        qApp->quit();
        event->accept();
    }
}

void MainWindow::createTrayIcon()
{
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QApplication::setQuitOnLastWindowClosed(false);
        trayIconMenu = new(std::nothrow) QMenu(this);
        if (trayIconMenu == nullptr) {
            qFatal("Cannot allocate memory for trayIconMenu X{");
            return;
        }
        updateTrayIcon();
        trayIcon = new(std::nothrow) QSystemTrayIcon(this);
        if (trayIcon == nullptr) {
            qFatal("Cannot allocate memory for trayIcon X{");
            return;
        }
        trayIcon->setIcon(QIcon(":/Images/icons/pip3line.png"));
        trayIcon->setContextMenu(trayIconMenu);
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateTrayIcon()));
        trayIcon->show();
    } else {
        qCritical() << tr("No System tray seems to be available, disabling tray Icon menu functionality");
    }
}

void MainWindow::showWindow()
{
    if (!isActiveWindow()) {
        showNormal();
        activateWindow();
    }
}

void MainWindow::onExternalBlockReceived(Block *block)
{
    if (stateOrchestrator == nullptr) {
       guiHelper->routeExternalDataBlock(block->getData());
       guiHelper->raisePip3lineWindow();
    } else {
        logger->logError("Received external data packet during save/restore, dropping the packet");
    }

    delete block;
}

void MainWindow::saveLoadState(QString filename, quint64 flags)
{
    if (stateOrchestrator == nullptr) {

        if (!filename.isEmpty()) {

            stateOrchestrator = new(std::nothrow) StateOrchestrator(filename, flags);
            if (stateOrchestrator == nullptr) {
                qFatal("Cannot allocate memory for StateOrchestrator X{");
            }

            connect(stateOrchestrator, SIGNAL(finished()), SLOT(onSaveLoadFinished()));
            connect(stateOrchestrator, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)),
                    logger, SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)));
            mainTabs->setEnabled(false);

            if (!stateOrchestrator->initialize()) {
                delete stateOrchestrator;
                stateOrchestrator = nullptr;
                mainTabs->setEnabled(true);
                return;
            }

            if (flags & GuiConst::STATE_SAVE_REQUEST) { // if saving jsut show the progress in the status bar
                stateStatusWidget = stateOrchestrator->getStatusGui(this);
                statusBar()->addWidget(stateStatusWidget);
            } else { // otherwise dialog
                stateDialog = stateOrchestrator->getStatusDialog(this);
                stateDialog->show();
            }

            BaseStateAbstract *stateObj = new(std::nothrow) MainWinStateObj(this);
            if (stateObj == nullptr) {
                qFatal("Cannot allocate memory for MainWinStateObj X{");
            }

            stateOrchestrator->addState(stateObj);

            stateObj = new(std::nothrow) GlobalConfStateObj(transformFactory);
            if (stateObj == nullptr) {
                qFatal("Cannot allocate memory for GlobalConfStateObj X{");
            }

            connect(stateObj, SIGNAL(settingsUpdated()), guiHelper, SLOT(refreshAll()));

            stateOrchestrator->addState(stateObj);

            stateObj = new(std::nothrow) ClearAllStateObj(this);
            if (stateObj == nullptr) {
                qFatal("Cannot allocate memory for ClearAllStateObj X{");
            }

            stateOrchestrator->addState(stateObj);
            stateOrchestrator->start();
        }
    } else {
        qWarning() << "A Save/Load is already running, ignoring the request";
    }
}

MainTabs *MainWindow::getMainTabs() const
{
    return mainTabs;
}



void MainWindow::handleUnixSignal()
{
#ifdef Q_OS_LINUX
    snExit->setEnabled(false);
    char tmp;
    ssize_t rb = ::read(sigFd[1], &tmp, sizeof(tmp));
    if (rb != 1)
        qCritical() << "Reading from the signal handler pipe failed T_T";
    qApp->exit();
    snExit->setEnabled(true);
#else
    qWarning() << "UNIX Signals not handle on this platform";
#endif
}

void MainWindow::onClearMemory()
{
    int res = QMessageBox::warning(this,tr("Clear memory"),tr("This will clear misc non-visibles objects from memory"),QMessageBox::Cancel,QMessageBox::Ok);
    if (res == QMessageBox::Ok) {
        qDebug() << "Clearing memory";
        guiHelper->clearDeletedTabs();
        if (analyseDialog != nullptr && analyseDialog->isHidden()) {
            delete analyseDialog;
            analyseDialog = nullptr;
            qDebug() << "Clearing memory: analyseDialog";
        }
        if (settingsDialog != nullptr && settingsDialog->isHidden()) {
            delete settingsDialog;
            settingsDialog = nullptr;
            qDebug() << "Clearing memory: settingsDialog";
        }
        if (regexphelpDialog != nullptr && regexphelpDialog->isHidden()) {
            delete regexphelpDialog;
            regexphelpDialog = nullptr;
            qDebug() << "Clearing memory: regexphelpDialog";
        }
        if (quickView != nullptr && quickView->isHidden()) {
            delete quickView;
            quickView = nullptr;
            qDebug() << "Clearing memory: quickView";
        }
        if (comparisonView != nullptr && comparisonView->isHidden()) {
            delete comparisonView;
            comparisonView = nullptr;
            qDebug() << "Clearing memory: comparisonView";
        }
        if (debugDialog != nullptr && debugDialog->isHidden()) {
            delete debugDialog;
            debugDialog = nullptr;
            qDebug() << "Clearing memory: debugDialog";
        }
    }
}

#ifdef Q_OS_LINUX

void MainWindow::exitSignalHandler(int signal)
{
    if (appExiting) { // just ignoring the signal
        return;
    }

    qWarning() << "Received signal " << signal << "Quitting";
    if (signal == SIGTERM || signal == SIGINT) {
        char a = '1';
        ssize_t rb = ::write(MainWindow::sigFd[0], &a, sizeof(a));
        if (rb != 1)
            qCritical() << "Writing into the signal handler pipe failed T_T";
    } else {
        fprintf(stdout, "Received signal %d, force exit the app without saving\n",signal);
        exit(1);
    }
}

#endif

DebugDialog *MainWindow::getDebugDialog() const
{
    return debugDialog;
}

ComparisonDialog *MainWindow::getComparisonView() const
{
    return comparisonView;
}

QuickViewDialog *MainWindow::getQuickView() const
{
    return quickView;
}

RegExpHelpDialog *MainWindow::getRegexphelpDialog() const
{
    return regexphelpDialog;
}

AnalyseDialog *MainWindow::getAnalyseDialog() const
{
    return analyseDialog;
}

SettingsDialog *MainWindow::getSettingsDialog() const
{
    return settingsDialog;
}

void MainWindow::onSaveState()
{
    QString conffileName = QFileDialog::getSaveFileName(this,tr("Choose file to save to"),QDir::home().absolutePath(), tr("all (*)"));
    if (!conffileName.isEmpty()) {
        QFileInfo finfo(conffileName);
        if (finfo.exists()) // resetting the file if exist
            QFile::resize(conffileName,0);

        saveLoadState(conffileName, guiHelper->getDefaultSaveStateFlags());
    }

}

void MainWindow::onLoadState()
{
    QString conffileName = QFileDialog::getOpenFileName(this,tr("Choose state file to load from"),QDir::home().absolutePath(), tr("all (*)"));
    if (!conffileName.isEmpty()) {
        saveLoadState(conffileName, guiHelper->getDefaultLoadStateFlags());
    }
}

void MainWindow::onSaveLoadFinished()
{
    if (stateOrchestrator != nullptr) {
        QString actions;
        if (stateOrchestrator->isSaving()) {
            QTimer::singleShot(100, this, SLOT(repaint()));
            actions.append("Save");
        } else {
           actions.append("Load");
        }

        delete stateOrchestrator;
        stateOrchestrator = nullptr;
        QTimer::singleShot(100, this, SLOT(repaint()));
        logger->logStatus(tr("%1 state finished").arg(actions));
    }

    mainTabs->setEnabled(true);

    if (stateDialog != nullptr) {
        stateDialog->hide();
        delete stateDialog;
        stateDialog = nullptr;
    }

    if (stateStatusWidget != nullptr) {
        delete stateStatusWidget;
        stateStatusWidget = nullptr;
    }

    if (appExiting)
        cleaningAndExit();
}

void MainWindow::autoSave()
{
    QString conffileName = guiHelper->getAutoSaveFileName();
    if (conffileName.isEmpty()) {
        logger->logError(tr("The filename for saving is empty, abording action."), tr("AutoSave"));
    } else {
        qDebug() << "autoSave called" << conffileName;
        QFileInfo finfo(conffileName);
        if (finfo.exists()) // resetting the file if exist
            QFile::resize(conffileName,0);
        saveLoadState(conffileName, guiHelper->getDefaultSaveStateFlags());
    }
}

void MainWindow::autoRestore()
{
    QString conffileName = guiHelper->getAutoSaveFileName();
    if (conffileName.isEmpty()) {
        logger->logError(tr("The filename for restoring is empty, abording action."), tr("AutoRestore"));
    } else {
        qDebug() << "autoRestore called" << conffileName;

        QFileInfo fi = QFileInfo(conffileName);
        if (fi.exists()) {
            saveLoadState(conffileName, guiHelper->getDefaultLoadStateFlags());
        } else {
            logger->logError(tr("The state file [%1] does not exist, nothing to restore.").arg(conffileName), tr("AutoRestore"));
        }
    }
}

void MainWindow::onExit()
{
    if (guiHelper->getAutoSaveState() && guiHelper->getAutoSaveOnExit()) {
        MainWindow::appExiting = true;
        autoSave();
    } else {
        cleaningAndExit();
    }
}

void MainWindow::cleaningAndExit()
{
    emit exiting();
    qApp->exit();
}

void MainWindow::hideEvent(QHideEvent *event)
{
    savedPos = pos();
    guiHelper->goIntoHidding();
    if (stateDialog != nullptr) {
        stateDialog->hide();
    }
    QMainWindow::hideEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (!savedPos.isNull())
        move(savedPos);
    guiHelper->isRising();
    if (stateDialog != nullptr) {
        stateDialog->show();
    }
    QMainWindow::showEvent(event);
}

void MainWindow::updateTrayIcon()
{
    if (trayIconMenu != nullptr) {
        trayIconMenu->clear();
        trayIconLabel = new(std::nothrow) QAction(tr("Import from clipboard as"), trayIconMenu);
        if (trayIconLabel == nullptr) {
            qFatal("Cannot allocate memory for trayIconLabel X{");
            return;
        }
        trayIconLabel->setDisabled(true);
        trayIconMenu->addAction(trayIconLabel);
        QAction * action = new(std::nothrow) QAction(UTF8_STRING_ACTION, trayIconMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for QAction in mainwindow 1 X{");
            return;
        }

        trayIconMenu->addAction(action);
        QStringList list = guiHelper->getImportExportFunctions();
        qSort(list);
        for (int i = 0; i < list.size(); i++) {
            action = new(std::nothrow) QAction(list.at(i), trayIconMenu);
            if (action == nullptr) {
                qFatal("Cannot allocate memory for QAction in mainwindow 2 X{");
                return;
            }
            trayIconMenu->addAction(action);
        }
        connect(trayIconMenu, SIGNAL(triggered(QAction*)), this, SLOT(onImport(QAction*)), Qt::UniqueConnection);

        trayIconMenu->addSeparator();
        trayIconMenu->addAction(ui->actionGet_data_from_URL);
        trayIconMenu->addSeparator();
        trayIconMenu->addAction(ui->actionExit);

    }
}

void MainWindow::onDebug()
{
    qDebug() << "Debugging";
    if (debugDialog == nullptr) {
        debugDialog = new(std::nothrow) DebugDialog(guiHelper, nullptr);
        if (debugDialog == nullptr) {
            qFatal("Cannot allocate memory for DebugDialog X{");
            return;
        }
        connect(debugDialog,SIGNAL(destroyed()), SLOT(onDebugDestroyed()));
    }

    debugDialog->show();
}

void MainWindow::onDebugDestroyed()
{
    debugDialog = nullptr;
}

void MainWindow::onNewAction(QAction *action)
{
    if (action == newLargeFileTabAction) {
        TabAbstract *ftab = mainTabs->newFileTab();
        QString fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
        if (!fileName.isEmpty()) {
            ftab->loadFromFile(fileName);
        }
    } else if (action == newTransformTabAction) {
        mainTabs->newTabTransform();
    } else if (action == newCurrentMemTabAction) {
        mainTabs->newCurrentMemTab();
    } else if (action == newHexeditorTabAction) {
        mainTabs->newHexEditorTab();
    } else if (action == newPacketAnalyserTabAction) {
        mainTabs->newPacketAnalyserTab();
    }
}

void MainWindow::onNewDefault(QByteArray initialData)
{
    mainTabs->newPreTab(guiHelper->getDefaultNewTab(),initialData);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::MiddleClick:
    case QSystemTrayIcon::DoubleClick:
        showWindow();
        break;
    case QSystemTrayIcon::Context:
        {
            QString input = QApplication::clipboard()->text();
            QList<QAction *> actions = trayIconMenu->actions();
            actions.removeAll(trayIconLabel);
            actions.removeAll(ui->actionExit);
            for (int i = 0; i < actions.size(); i++) {
                actions.at(i)->setEnabled(!input.isEmpty());
            }
        }
        break;
    default:
        qWarning() << tr("Unmanaged Sytem tray activation reason: %1").arg(reason);
    }
}

void MainWindow::onImport(QAction *action)
{
    if (action == ui->actionGet_data_from_URL) {
        onDataFromURL();
    } else if (action != ui->actionExit) {
        QClipboard *clipboard = QApplication::clipboard();
        QString input = clipboard->text();
        if (action->text() == UTF8_STRING_ACTION) {
            mainTabs->newTabTransform(input.toUtf8());
        } else {
            TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
            if (ta != nullptr) {
                ta->setWay(TransformAbstract::OUTBOUND);
                mainTabs->newTabTransform(ta->transform(input.toUtf8()));
            }
        }
        showWindow();
    }
}

void MainWindow::onDataFromURL()
{
    QString input = QApplication::clipboard()->text();
    QUrl resource(input);
    if (resource.isValid()) {
        DownloadManager * dm = new(std::nothrow) DownloadManager(resource, guiHelper);
        if (dm == nullptr) {
            qFatal("Cannot allocate memory for DownloadManager (url) X{");
            return;
        }
        connect(dm, SIGNAL(finished(QByteArray)), this, SLOT(processingUrlDownload(QByteArray)));
        guiHelper->requestDownload(resource, nullptr,dm);
    }
}

void MainWindow::onQuickView(bool checked)
{
    if (checked) {
        if (quickView == nullptr) {
            quickView = new(std::nothrow) QuickViewDialog(guiHelper, this);
            if (quickView == nullptr) {
                qFatal("Cannot allocate memory for QuickViewDialog X{");
                return;
            }
            connect(guiHelper, SIGNAL(newSelection(QByteArray)), quickView, SLOT(receivingData(QByteArray)));
            quickView->attachAction(ui->actionMagic);
        }

        quickView->show();
        quickView->raise();
    } else if (quickView != nullptr) {
        quickView->hide();
    }
}

void MainWindow::onCompare(bool checked)
{
    if (checked) {
        if (comparisonView == nullptr) {
            comparisonView= new(std::nothrow) ComparisonDialog(guiHelper, this);
            if (comparisonView == nullptr) {
                qFatal("Cannot allocate memory for ComparisonDialog X{");
                return;
            }
            comparisonView->attachAction(ui->actionCompare);
            comparisonView->adjustSize();
        }
        comparisonView->show();
        comparisonView->raise();
    } else if (comparisonView != nullptr) {
        comparisonView->hide();
    }
}

void MainWindow::onAnalyse(bool checked)
{
    if (checked) {
        if (!analyseDialog) {
            analyseDialog = new(std::nothrow) AnalyseDialog(guiHelper,this);
            if (analyseDialog == nullptr) {
                qFatal("Cannot allocate memory for analyseDialog X{");
                return;
            }
            analyseDialog->attachAction(ui->action_Analyse);
        }

        analyseDialog->show();
        analyseDialog->raise();
    } else if (analyseDialog != nullptr) {
        analyseDialog->hide();
    }
}

void MainWindow::on_actionLogs_triggered()
{
    mainTabs->showLogs();
}

const QString MainWinStateObj::NAME = QObject::tr("Main Window");

MainWinStateObj::MainWinStateObj(MainWindow *target) :
    mwin(target)
{
    setName(NAME);
}

MainWinStateObj::~MainWinStateObj()
{

}

void MainWinStateObj::run()
{
    QByteArray sdata;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {

        if (flags & GuiConst::STATE_LOADSAVE_DIALOG_POS) {
            writer->writeStartElement(GuiConst::STATE_MAIN_WINDOW);

            writer->writeAttribute(GuiConst::STATE_WIDGET_GEOM, write(mwin->saveGeometry()));

            writer->writeAttribute(GuiConst::STATE_WIDGET_STATE, write(mwin->saveState()));
            if (!genCloseElement()) return;
        }

        writer->writeStartElement(GuiConst::STATE_DIALOGS_RUNNING);

        writer->writeAttribute(GuiConst::STATE_ANALYSE_DIALOG, write(mwin->getAnalyseDialog() != nullptr));
        writer->writeAttribute(GuiConst::STATE_REGEXPHELP_DIALOG, write(mwin->getRegexphelpDialog() != nullptr));
        writer->writeAttribute(GuiConst::STATE_QUICKVIEW_DIALOG, write(mwin->getQuickView() != nullptr));
        writer->writeAttribute(GuiConst::STATE_COMPARISON_DIALOG, write(mwin->getComparisonView() != nullptr));
        writer->writeAttribute(GuiConst::STATE_DEBUG_DIALOG, write(mwin->getDebugDialog() != nullptr));
        writer->writeAttribute(GuiConst::STATE_SETTINGS_DIALOG, write(mwin->getSettingsDialog() != nullptr));

    } else { // loading
        if (reader->name() != GuiConst::STATE_MAIN_WINDOW) {
            if (!reader->readNextStartElement()) {
                emit log(tr("Cannot read next element"), metaObject()->className(), Pip3lineConst::LERROR);
                return;
            }
        }

        if (flags & GuiConst::STATE_LOADSAVE_DIALOG_POS &&
                reader->name() == GuiConst::STATE_MAIN_WINDOW) { // loading mainwindow state

            QXmlStreamAttributes attrList = reader->attributes();
            sdata = readByteArray(attrList.value(GuiConst::STATE_WIDGET_GEOM));
            if (!sdata.isEmpty())
                mwin->restoreGeometry(sdata);

            sdata = readByteArray(attrList.value(GuiConst::STATE_WIDGET_STATE));
            if (!sdata.isEmpty())
                mwin->restoreState(sdata);

            genCloseElement();
            if (!reader->readNextStartElement()) {
                emit log(tr("Cannot read next element %1").arg(reader->errorString()), metaObject()->className(), Pip3lineConst::LERROR);
                return;
            }

        } else {
            qDebug() << GuiConst::STATE_MAIN_WINDOW << "not loaded";
        }

        if (reader->name() == GuiConst::STATE_DIALOGS_RUNNING) {
            QXmlStreamAttributes attrList = reader->attributes();
            if (attrList.hasAttribute(GuiConst::STATE_ANALYSE_DIALOG) &&
                    readBool(attrList.value(GuiConst::STATE_ANALYSE_DIALOG)))
                mwin->onAnalyse(true);

            if (attrList.hasAttribute(GuiConst::STATE_REGEXPHELP_DIALOG) &&
                    readBool(attrList.value(GuiConst::STATE_REGEXPHELP_DIALOG)))
                mwin->onHelpWithRegExp();

            if (attrList.hasAttribute(GuiConst::STATE_QUICKVIEW_DIALOG) &&
                    readBool(attrList.value(GuiConst::STATE_QUICKVIEW_DIALOG)))
                mwin->onQuickView(true);

            if (attrList.hasAttribute(GuiConst::STATE_COMPARISON_DIALOG) &&
                    readBool(attrList.value(GuiConst::STATE_COMPARISON_DIALOG)))
                mwin->onCompare(true);

            if (attrList.hasAttribute(GuiConst::STATE_DEBUG_DIALOG) &&
                    readBool(attrList.value(GuiConst::STATE_DEBUG_DIALOG)))
                mwin->onDebug();

            if (attrList.hasAttribute(GuiConst::STATE_SETTINGS_DIALOG) &&
                    readBool(attrList.value(GuiConst::STATE_SETTINGS_DIALOG)))
                mwin->onSettingsDialogOpen(true);
        }
    }

    BaseStateAbstract *temp = mwin->getMainTabs()->getStateMngtObj(); // need to be first to be executed after the closingstate
    emit addNewState(temp);

    temp = new(std::nothrow) ClosingState();
    if (temp == nullptr) {
        qFatal("Cannot allocate memory for ClosingState X{");
    }
    emit addNewState(temp);


    AppDialog * dialog = nullptr;

    dialog = mwin->getSettingsDialog();
    if (dialog != nullptr) {
        temp = dialog->getStateMngtObj();
        emit addNewState(temp);
    }

    dialog = mwin->getDebugDialog();
    if (dialog != nullptr) {
        temp = dialog->getStateMngtObj();
        emit addNewState(temp);
    }

    dialog = mwin->getComparisonView();
    if (dialog != nullptr) {
        temp = dialog->getStateMngtObj();
        emit addNewState(temp);
    }

    dialog = mwin->getQuickView();
    if (dialog != nullptr) {
        temp = dialog->getStateMngtObj();
        emit addNewState(temp);
    }

    dialog = mwin->getRegexphelpDialog();
    if (dialog != nullptr) {
        temp = dialog->getStateMngtObj();
        emit addNewState(temp);
    }

    dialog = mwin->getAnalyseDialog();
    if (dialog != nullptr) {
        temp = dialog->getStateMngtObj();
        emit addNewState(temp);
    }
}


GlobalConfStateObj::GlobalConfStateObj(TransformMgmt *transformMgmt) :
    transformMgmt(transformMgmt)
{

}

GlobalConfStateObj::~GlobalConfStateObj()
{

}

void GlobalConfStateObj::run()
{
    QSettings *globalConf = transformMgmt->getSettingsObj();
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        if (flags & GuiConst::STATE_LOADSAVE_GLOBAL_CONF) {

            QTemporaryFile tempfile;
            if (!tempfile.open()) {
                emit log(tr("Cannot open temporary file to save the global conf"), metaObject()->className(), Pip3lineConst::LERROR);
                return;
            }
            QSettings * tempSettings = new(std::nothrow) QSettings(tempfile.fileName(),QSettings::IniFormat);
            if (tempSettings == nullptr) {
                qFatal("Cannot allocate memory for QSettings when saving state X{");
            }

            QStringList keys = globalConf->allKeys();

            foreach (const QString &key, keys) {
                    tempSettings->setValue(key, globalConf->value(key));
            }

            keys.clear();

            tempSettings->sync();
            delete tempSettings; // just deleting the object refered to, not the file
            tempSettings = nullptr;

            tempfile.seek(0);
            writer->writeTextElement(GuiConst::STATE_GLOBAL_CONF, write(tempfile.readAll()));
            tempfile.close();
        }
    } else {
        if (reader->name() != GuiConst::STATE_GLOBAL_CONF) {
            if (!reader->readNextStartElement()) {
                emit log(tr("Cannot read next element"), metaObject()->className(), Pip3lineConst::LERROR);
                return;
            }
        }

        if ((flags & GuiConst::STATE_LOADSAVE_GLOBAL_CONF) &&
                reader->name() == GuiConst::STATE_GLOBAL_CONF) { // got a globalconf

            QByteArray sdata = readByteArray(reader->readElementText());
            QTemporaryFile tempfile;
            if (!tempfile.open()) {
                emit log(tr("Cannot open temporary file to load the global conf"), metaObject()->className(), Pip3lineConst::LERROR);
                return;
            }

            tempfile.write(sdata);
            tempfile.flush();
            QSettings * tempSettings = new(std::nothrow) QSettings(tempfile.fileName(),QSettings::IniFormat);
            if (tempSettings == nullptr) {
                qFatal("Cannot allocate memory for QSettings when loading state X{");
            }

            QStringList keys = tempSettings->allKeys();

            foreach (const QString &key, keys) {
                globalConf->setValue( key, tempSettings->value( key ) );
            }

            delete tempSettings;
            transformMgmt->reset();

            emit settingsUpdated();

            if (!reader->readNextStartElement()) {
                emit log(tr("Cannot read next element %1").arg(reader->errorString()), metaObject()->className(), Pip3lineConst::LERROR);
                return;
            }

        } else {
            qDebug() << GuiConst::STATE_GLOBAL_CONF << "not loaded";
        }
    }
    delete globalConf;
}


ClearAllStateObj::ClearAllStateObj(MainWindow *target) :
    mwin(target)
{

}

ClearAllStateObj::~ClearAllStateObj()
{

}

void ClearAllStateObj::run()
{
    if (!(flags & GuiConst::STATE_SAVE_REQUEST)) {
        //clearing everything out
        delete mwin->settingsDialog;
        mwin->settingsDialog = nullptr;
        delete mwin->analyseDialog;
        mwin->analyseDialog = nullptr;
        delete mwin->regexphelpDialog;
        mwin->regexphelpDialog = nullptr;
        delete mwin->quickView;
        mwin->quickView = nullptr;
        delete mwin->comparisonView;
        mwin->comparisonView = nullptr;
        delete mwin->debugDialog;
        mwin->debugDialog = nullptr;

        mwin->mainTabs->clearTabs();
        mwin->guiHelper->deleteImportExportFuncs();
    }
}

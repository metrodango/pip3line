/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QApplication>
#include "mainwindow.h"
#include <QtPlugin>
#include <QDebug>
#include <QStyleFactory>
#include <QObject>
#include <QStyle>

#define DEBUG_CMD "--debug"
#define FILE_CMD "--file"

#include <stdio.h>
#include <stdlib.h>
#include <QDateTime>
#include <commonstrings.h>
#include <QDir>
#include <QByteArray>
FILE * _logFile = nullptr; // logFile
MainWindow * w = nullptr;

#if QT_VERSION >= 0x050000
void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    QByteArray fmess = msg.toUtf8();
    const char *localMsg = fmess.constData();
#else
void myMessageOutput(QtMsgType type, const char *localMsg)
{
#endif
    QByteArray ftime = QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8();
    const char * currentTime = ftime.constData();

    if (type == QtDebugMsg) {
        fprintf(stderr, "%s Debug: %s\n",currentTime, localMsg);
        if (_logFile != nullptr)
            fprintf(_logFile, "%s Debug: %s\n",currentTime, localMsg);
    }
#if QT_VERSION >= 0x050500
    else if (type == QtInfoMsg) {
        fprintf(stderr, "%s Info: %s\n",currentTime, localMsg);
        if (_logFile != nullptr)
            fprintf(_logFile, "%s Info: %s\n",currentTime, localMsg);
    }
#endif
    else if (type == QtWarningMsg) {
        fprintf(stderr, "%s Warning: %s\n",currentTime, localMsg);
        if (_logFile != nullptr)
            fprintf(_logFile, "%s Warning: %s\n",currentTime, localMsg);
    } else if (type == QtCriticalMsg) {
        fprintf(stderr, "%s Critical: %s\n",currentTime, localMsg);
        if (_logFile != nullptr)
            fprintf(_logFile, "%s Critical: %s\n",currentTime, localMsg);
    } else if (type == QtFatalMsg) {
        fprintf(stderr, "%s Fatal: %s\n",currentTime, localMsg);
        if (_logFile != nullptr) {
            fprintf(_logFile, "%s Fatal: %s\n",currentTime, localMsg);
            fclose (_logFile);
            _logFile = nullptr;
        }
        abort();
    }
	if (_logFile != nullptr) {
		fflush(_logFile);
	}
}

#ifdef Q_OS_LINUX
#include <csignal>

static void setup_unix_signal_handlers()
{
    struct sigaction term;

    term.sa_handler = MainWindow::exitSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) == -1)
        qWarning("Could not set the SIGTERM signal handler");
    if (sigaction(SIGINT, &term, 0) == -1)
        qWarning("Could not set the SIGINT signal handler");
    if (sigaction(SIGQUIT, &term, 0) == -1)
        qWarning("Could not set the SIGQUIT signal handler");
    if (sigaction(SIGABRT, &term, 0) == -1)
        qWarning("Could not set the SIGABRT signal handler");
}
#endif

int main(int argc, char *argv[])
{
    // Qt initialisation
    QApplication a(argc, argv);

#ifdef Q_OS_LINUX
    setup_unix_signal_handlers();
#endif

    QStringList pathlist = QCoreApplication::libraryPaths();
    qDebug() << pathlist;
#if QT_VERSION >= 0x050000
    // Fixing the qt5 plugin mess
    pathlist << ".";
    QCoreApplication::setLibraryPaths(pathlist);
#endif

    // Creating the QT log file

    QDir homeDir = QDir::home();
    if (!homeDir.cd(Pip3lineConst::USER_DIRECTORY)) {
        if (!homeDir.mkpath(Pip3lineConst::USER_DIRECTORY)) {
            qWarning("Cannot create user directory for log file");
        }
    }

    homeDir = QDir::home();
    if (homeDir.cd(Pip3lineConst::USER_DIRECTORY)) {
        QByteArray logdir = homeDir.absolutePath().append(QDir::separator()).append("pip3line_gui.log").toLocal8Bit();
#ifdef Q_CC_MSVC
#define BUFFERSIZE 200

        char buffer[BUFFERSIZE];
        memset((void *)buffer, 0, BUFFERSIZE);

        errno_t err = fopen_s (&_logFile, logdir.constData(),"w");
        if ( err != 0) {
            strerror_s(buffer, BUFFERSIZE, err);
            qWarning( buffer );
#else
        _logFile = fopen (logdir.constData(),"w");

        if (_logFile == nullptr) {
            qWarning("Cannot open log file for writing");
#endif
        } else {
#if QT_VERSION >= 0x050000
            qInstallMessageHandler(myMessageOutput);
#else
            qInstallMsgHandler(myMessageOutput);
#endif
        }
    }


#if QT_VERSION >= 0x050000
    // forcing style for Qt5. can be overwritten at runtime
    // with the option -style [style name]
    // An indicative list of available themes is given in the Help->info dialog
    QStringList stylelist = QStyleFactory::keys();
    if (stylelist.contains("Fusion"))
        QApplication::setStyle("Fusion");
#ifdef Q_OS_WIN
    else if (stylelist.contains("WindowsVista"))
        QApplication::setStyle("WindowsVista");
    else if (stylelist.contains("WindowsXP"))
        QApplication::setStyle("WindowsXP");
#endif

#endif

 //   a.setStyleSheet("QWidget{ selection-background-color: blue}");
    qDebug() << "App started";

    // Clearing the PATH to avoid library corruption whenever loading plugins (mostly a Windows issue, but it does not hurt the other platforms)
    qputenv("PATH", QByteArray());

#ifdef Q_OS_UNIX
    qputenv("LD_PRELOAD", QByteArray());
#endif

    QStringList list = QApplication::arguments();

    bool debugging =  false;
    if (list.contains(DEBUG_CMD)) {
        debugging = true;
        list.removeAll(DEBUG_CMD);
    }

    QString fileName;
    if (list.contains(FILE_CMD)) {
        int index = list.indexOf(FILE_CMD);
        if (list.size() > index + 1) {
            fileName = list.at(index + 1);
            list.removeAt(index);
            list.removeAt(index);
        } else {
            qCritical() << QObject::tr("Missing file name for --file, ignoring.");
        }
    }

    // initializing the default random generator
    qsrand((uint)QDateTime::currentDateTime().time().msec());

    int ret = 0;
    w = new(std::nothrow) MainWindow(debugging);
    if (w != nullptr) {
        if (!fileName.isEmpty())
            w->loadFile(fileName);

        w->show();

        ret = a.exec();
        //qWarning() << "App ending";

        MainWindow * wt = w;
        w = nullptr;
        delete wt;
    }

    if (_logFile != nullptr) {
        fclose (_logFile);
        _logFile = nullptr;
    }

    return ret;
}

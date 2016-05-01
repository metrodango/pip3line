/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QtCore/QCoreApplication>
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include "masterthread.h"
#include "../version.h"

#define NOERRORPARAM "-n"
#define VERBOSEPARAM "-v"
#define BINARY_INPUT_PARAM "-b"
#define TRANSFORM_NAME_PARAM "-t"
#define FILE_PARAM "-f"
#define INBOUND_PARAM "-i"
#define OUTBOUND_PARAM "-o"

void usage() {
    QTextStream cout(stdout);

    cout << APPNAME << QString(" %1.%2").arg(VERSION_MAJOR).arg(VERSION_MINOR) << QObject::tr(" Using libtransform v%1").arg(LIB_TRANSFORM_VERSION) << endl;
    cout << "Usage: " << APPNAME << " [file]" << endl;
    cout << "       " << TRANSFORM_NAME_PARAM << QObject::tr(" [Transform name] name of an existing Transform to use") << endl;
    cout << "       " << FILE_PARAM << QObject::tr(" [configuration file] name of the configuration file to use (generated from the GUI)") << endl;
    cout << "       " << BINARY_INPUT_PARAM << QObject::tr(" Treat the input as one binary block (as opposed to text file with lines)") << endl;
    cout << "       " << NOERRORPARAM << QObject::tr(" do not show errors") << endl;
    cout << "       " << VERBOSEPARAM << QObject::tr(" verbose") << endl;
}

#ifdef Q_OS_LINUX
#include <signal.h>

void termSignalHandler(int) {
    QTimer::singleShot(0,QCoreApplication::instance(),SLOT(quit()));
}

static void setup_unix_signal_handlers()
{
    struct sigaction term;

    term.sa_handler = termSignalHandler;
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
#ifdef Q_OS_LINUX
    setup_unix_signal_handlers();
#endif

    QCoreApplication a(argc, argv);
    QTextStream cerr(stderr);
    QStringList args = a.arguments();
    QString name;
    QString fileName;
    bool inbound = true;

    bool noError = false;
    bool verbose = false;
    bool binaryInput = false;
    int ret = 0;

    args.takeFirst();



    if (args.contains(NOERRORPARAM)) {
        noError = true;
        args.removeAll(NOERRORPARAM);
    }

    if (args.contains(VERBOSEPARAM)) {
        verbose = true;
        args.removeAll(VERBOSEPARAM);
    }

    if (args.contains(BINARY_INPUT_PARAM)) {
        binaryInput = true;
        args.removeAll(BINARY_INPUT_PARAM);
    }

    if (args.contains(INBOUND_PARAM) && args.contains(OUTBOUND_PARAM)) {
        cerr << QObject::tr("Cannot have %1 and %2 at the same time").arg(INBOUND_PARAM).arg(OUTBOUND_PARAM) << endl;
        usage();
        return -1;
    }

    if (args.contains(INBOUND_PARAM)) {
        inbound = true;
        args.removeAll(INBOUND_PARAM);
    }

    if (args.contains(OUTBOUND_PARAM)) {
        inbound = false;
        args.removeAll(OUTBOUND_PARAM);
    }

    if (args.contains(TRANSFORM_NAME_PARAM) && args.contains(FILE_PARAM)) {
        cerr << QObject::tr("Cannot have %1 and %2 at the same time").arg(TRANSFORM_NAME_PARAM).arg(FILE_PARAM) << endl;
        usage();
        return -1;
    } else if (!args.contains(TRANSFORM_NAME_PARAM) && !args.contains(FILE_PARAM)) {
        cerr << QObject::tr("Need either a configuration file or a transform name to run") << endl;
        usage();
        return -1;
    }

    if (args.contains(FILE_PARAM)) {
        int index = args.indexOf(FILE_PARAM);
        if (index >= args.size() - 1 || index < 0) {
            cerr << QObject::tr("Need a file name for parameter %1").arg(FILE_PARAM) << endl;
            usage();
            return -1;
        }
        fileName = args.at(index + 1).trimmed();
        args.removeAll(FILE_PARAM);
        args.removeAt(index);
        args.removeAt(index);
    }

    if (args.contains(TRANSFORM_NAME_PARAM)) {
        int index = args.indexOf(TRANSFORM_NAME_PARAM);
        if (index >= args.size() - 1 || index < 0) {
            cerr << QObject::tr("Need a name for parameter %1").arg(TRANSFORM_NAME_PARAM) << endl;
            usage();
            return -1;
        }
        name = args.at(index + 1).trimmed();
        args.removeAll(TRANSFORM_NAME_PARAM);
        args.removeAt(index);
        args.removeAt(index);
    }

    MasterThread *pt = NULL;

    pt = new(std::nothrow) MasterThread(binaryInput, noError, verbose);
    if (pt == NULL) {
        qFatal("Cannot allocate memory for MasterThread X{");
    }
    if (fileName.isEmpty()) {
        pt->setTransformName(name, inbound);
    } else {
        pt->setConfigurationFile(fileName, inbound);
    }

    pt->start();
    ret = a.exec();

    delete pt;
    return ret;
}

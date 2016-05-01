/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QUrl>
#include <QMutex>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QUrl>
#include "guihelper.h"
#include "sources/bytesourceabstract.h"

class QNetworkRequest;
class GuiHelper;

class DownloadManager : public QObject
{
        Q_OBJECT
    public:
        explicit DownloadManager(QUrl &url, GuiHelper *guiHelper, QObject *parent = 0);
        ~DownloadManager();
        bool launch();
        QByteArray getData();
    signals:
        void finished(QByteArray data);
    public slots:
        void requestFinished();
        void networkSSLError(QList<QSslError> sslError);
    private:
        Q_DISABLE_COPY(DownloadManager)
        static const QString ID;
        QNetworkAccessManager * networkManager;
        QByteArray data;
        QUrl resource;
        QUrl previousRedirect;
        void createRequest(QUrl url);
        GuiHelper *guiHelper;
        int redirects;
};

#endif // DOWNLOADMANAGER_H

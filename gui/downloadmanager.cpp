/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "downloadmanager.h"
#include <QMutexLocker>
#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QVariant>
#include <QUrl>
#include <QDebug>
#include "loggerwidget.h"

const QString DownloadManager::ID = "DownloadManager";

DownloadManager::DownloadManager(QUrl &url, GuiHelper *guiHelper,QObject *parent) :
    QObject(parent),
    guiHelper(guiHelper)
{
    resource = url;
    networkManager = guiHelper->getNetworkManager();
    redirects = 0;
}

DownloadManager::~DownloadManager()
{
    qDebug() << "Destroying " << this;
}

bool DownloadManager::launch()
{
    if (networkManager != nullptr) {

        createRequest(resource);

    } else {
        guiHelper->getLogger()->logError(tr("No network manager, ignoring download request"),ID);
        return false;
    }
    return true;
}

QByteArray DownloadManager::getData()
{
    // there should not be any need for a mutex here, as getData and requestFisnihed should never execute concurrently
    if (data.size() == 0) {
        guiHelper->getLogger()->logWarning(tr("No data for collection"),ID);
    }
    return data;
}

void DownloadManager::requestFinished()
{
    QNetworkReply * reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr) {
        qFatal("DownloadManager::requestFinished reply is nullptr");
    }

    if (reply->error()) {
        guiHelper->getLogger()->logError(tr("[Failed to load \"%1\"] %2").arg(resource.toString()).arg(reply->errorString()),ID);
    } else {
        QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (!previousRedirect.isEmpty() && previousRedirect == possibleRedirectUrl) {
            guiHelper->getLogger()->logError(tr("The URL %1 is redirected to the itself giving up...").arg(previousRedirect.toString()));
        } else if (redirects > 10) {
            guiHelper->getLogger()->logError(tr("There was more than 10 redirections when following the URL, giving up..."));
        } else if (possibleRedirectUrl.isEmpty()) {
            // there should not be any need for a mutex here, as getData and requestFisnihed should never execute concurrently
            data = reply->readAll();
            qDebug() << tr("%1 successfully loaded (%2 bytes) ").arg(resource.toEncoded().constData()).arg(data.size());
            emit finished(data);
            deleteLater();
        } else { // following redirection
            redirects++;
            guiHelper->getLogger()->logWarning(tr("URL redirected to %1").arg(possibleRedirectUrl.toString()));
            createRequest(possibleRedirectUrl);
        }

    }
    reply->deleteLater();
}

void DownloadManager::networkSSLError(QList<QSslError> sslErrors)
{
    for (int i = 0; i < sslErrors.size(); i++) {
        guiHelper->getLogger()->logError(sslErrors.at(i).errorString(),ID);
    }
}

void DownloadManager::createRequest(QUrl url)
{
    QNetworkRequest request(url);
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    QNetworkReply * reply = networkManager->get(request);
    connect(reply, &QNetworkReply::sslErrors, this, &DownloadManager::networkSSLError);
    connect(reply, &QNetworkReply::finished, this, &DownloadManager::requestFinished);
    if (QSslConfiguration::defaultConfiguration().peerVerifyMode() == QSslSocket::VerifyNone)
        reply->ignoreSslErrors();
}

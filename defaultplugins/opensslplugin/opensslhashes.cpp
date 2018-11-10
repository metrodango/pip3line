/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "opensslhashes.h"

#include <stdio.h>
#include <QTextStream>
#include <QStringList>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/lhash.h>
#include <openssl/conf.h>
#include <openssl/opensslv.h>

const QString OpenSSLHashes::id = "OpenSSL hashes";
QStringList OpenSSLHashes::hashList = QStringList();


OpenSSLHashes::OpenSSLHashes(const QString &nhashName)
{
    QTextStream cerr(stderr);
    if (!nhashName.isEmpty())
        hashName = nhashName;
    else if (!hashList.isEmpty()) {
        hashName = hashList.at(0);
    } else {
        cerr << "Error in Openssl hashes plugins: no hash function loaded." << endl;
    }
}

OpenSSLHashes::~OpenSSLHashes()
{
}

QString OpenSSLHashes::name() const {
    return hashName;
}

QString OpenSSLHashes::description() const {
    return tr("Hashes functions from the openssl library");
}

bool OpenSSLHashes::isTwoWays() {
    return false;
}

QString OpenSSLHashes::help() const
{
    QString help;
    help.append("<p>Hashes functions from the Openssl library.</p><p>Currently available Openssl hashes: <ul>");
    for (int i = 0; i < hashList.size(); i++)
    {
        help.append("<li>");
        help.append(hashList.at(i));
        help.append("</li>");
    }
    help.append("</ul></p><p>Note: Openssl library files need to be available for this plugin to work properly.</p>");
    help.append(QString("<p>Plugin compiled against %1<br>").arg(OPENSSL_VERSION_TEXT));
    help.append("Plugin currently running with: <ul>");
    help.append(QString("<li>Version: %1</li>").arg(SSLeay_version(SSLEAY_VERSION)));
    help.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_PLATFORM)));
    help.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_CFLAGS)));
    help.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_BUILT_ON)));

    help.append("</ul></p>");
    return help;
}

QHash<QString, QString> OpenSSLHashes::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLHASHNAME,hashName);

    return properties;
}

bool OpenSSLHashes::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    res = setHashName(propertiesList.value(XMLHASHNAME)) && res;

    return res;
}

void OpenSSLHashes::transform(const QByteArray &input, QByteArray &output)
{
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;
    output.clear();

    const EVP_MD *md = EVP_get_digestbyname(hashName.toUtf8().data());
    if (!md) {
        emit error(tr("Unknown hash %1").arg(hashName),id);
        return;
    }
    EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, nullptr);
    EVP_DigestUpdate(mdctx, input.data(), static_cast<size_t>(input.size()));
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    if (md_len >= INT_MAX) {
        emit error(tr("Invalid return size for hash %1").arg(hashName),id);
        md_len = 0;
    }
    output.append(reinterpret_cast<char *>(md_value),static_cast<int>(md_len));
    EVP_MD_CTX_destroy(mdctx);
}

QString OpenSSLHashes::getHashName()
{
    return hashName;
}

bool OpenSSLHashes::setHashName(QString name)
{
    if (!hashList.contains(name)) {
        emit error(tr("Unknown hash %1").arg(name),id);
        return false;
    }
    hashName = name;
    emit confUpdated();
    return true;
}



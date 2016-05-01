/**
pip3line: easy encoder/decoder and more
Copyright (C) 2012  Gabriel Caudrelier<gabriel.caudrelier@gmail.com>

Pip3line is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Pip3line is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Pip3line.  If not, see <http://www.gnu.org/licenses/>.
**/

#include "opensslhashes.h"
#include "xmlcommons.h"
#include <stdio.h>
#include <QTextStream>
#include <QStringList>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/lhash.h>
#include <openssl/conf.h>

const QString OpenSSLHashes::id = "OpenSSL hashes";
QMutex OpenSSLHashes::initlocker;
QStringList OpenSSLHashes::hashList = QStringList();
const QStringList OpenSSLHashes::blacklistHash = QStringList() << "ecdsa-with-SHA1" << "DSA" << "DSA-SHA";

extern "C" {
void static list_md_fn_OpenSSLHashes(const EVP_MD *m, const char */* unused */, const char */* unused */, void */* unused */)
{

    if (m) {
        QString name(EVP_MD_name(m));

        // blacklisting some messages digest returned by openssl list function.
        // If someone can explain to me why the list of hashes in here is different
        // from the one in the openssl command line tool that would be great.
        // Not only it returns a different list, but the list itself is mostly duplicated ??

        if (OpenSSLHashes::blacklistHash.contains(name)) {
            return;
        }
        if (!OpenSSLHashes::hashList.contains(name)) {
#ifndef QT_NO_DEBUG
            QTextStream cout(stdout);
            cout << name << " " << m << " " << EVP_MD_type(m) << endl;
#endif
            OpenSSLHashes::hashList.append(name);
        }
    }
}
}

OpenSSLHashes::OpenSSLHashes()
{
    initlocker.lock();
    if (hashList.isEmpty()) {
        OpenSSL_add_all_digests();
        EVP_MD_do_all_sorted(list_md_fn_OpenSSLHashes, 0);
    }
    initlocker.unlock();

    if (hashList.size() > 0)
        hashName = hashList.at(0);
}

OpenSSLHashes::~OpenSSLHashes()
{
    //EVP_cleanup();
}


QString OpenSSLHashes::name() const {
    return id;
}

QString OpenSSLHashes::description() const {
    return tr("Hashes functions from the openssl library");
}

TransformAbstract::Type OpenSSLHashes::getType() const {
    return TransformAbstract::HASH;
}

bool OpenSSLHashes::isTwoWays() {
    return false;
}

QDomElement OpenSSLHashes::getConf(QDomDocument *xmlDoc)
{
    QDomElement conf;
    conf = TransformAbstract::getConf(xmlDoc);

    conf.setAttribute(XMLHASHNAME,hashName);

    return conf;
}

bool OpenSSLHashes::setConf(QDomElement *conf)
{
    bool res = TransformAbstract::setConf(conf);

    res = setHashName(conf->attribute(XMLHASHNAME)) && res;

    return res;
}

QByteArray OpenSSLHashes::transform(const QByteArray &input) {
    QByteArray ret;

    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    md = EVP_get_digestbyname(hashName.toAscii().data());
    if (!md) {
        emit error(tr("OpenSSL: Unknown hash"),id);
        return ret;
    }

    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input.data(), input.size());
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_destroy(mdctx);
    ret.append((char *)md_value,md_len);

    return ret;
}

QString OpenSSLHashes::getHashName()
{
    return hashName;
}

bool OpenSSLHashes::setHashName(QString name)
{
    if (!hashList.contains(name)) {
        emit error(tr("OpenSSL: Unknown hash %1").arg(name),id);
        return false;
    }
    hashName = name;
    emit confUpdated();
    return true;
}



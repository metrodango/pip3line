/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "opensslplugin.h"
#include "opensslhashes.h"
#include <QTextStream>
#include <QLabel>
#include <QDebug>
#include <pip3linecallback.h>
#include "../../version.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>

#if QT_VERSION >= 0x050000
const QStringList OpensslPlugin::blacklistHash = QStringList() << "ecdsa-with-SHA1" << "DSA" << "DSA-SHA" << "MD5" << "MD4" << "SHA1" << "SHA"
                                                               << "SHA224" << "SHA256" << "SHA384" << "SHA512";
#else
const QStringList OpensslPlugin::blacklistHash = QStringList() << "ecdsa-with-SHA1" << "DSA" << "DSA-SHA" << "MD5" << "MD4" << "SHA1" << "SHA";
#endif
QMutex OpensslPlugin::hashListLocker;

extern "C" {
void static list_md_fn_OpenSSLHashes(const EVP_MD *m, const char *, const char *, void *)
{

    if (m) {
        QString name(EVP_MD_name(m));

        // blacklisting some messages digest returned by openssl list function.
        // If someone can explain to me why the list of hashes in here is different
        // from the one given by the openssl command line tool that would be great.
        // Not only it returns a different list, but the list itself is mostly duplicated ??
        // Blacklist also includes hashes already present by default in the Qt API

        if (OpensslPlugin::blacklistHash.contains(name)) {
            return;
        }
        if (!OpenSSLHashes::hashList.contains(name)) {
            qDebug() << name << " " << m << " " << EVP_MD_type(m);
            OpenSSLHashes::hashList.append(name);
        }
    }
}
}

OpensslPlugin::OpensslPlugin()
{
    hashListLocker.lock();
    if (OpenSSLHashes::hashList.isEmpty()) {
        OpenSSL_add_all_digests();
        EVP_MD_do_all_sorted(list_md_fn_OpenSSLHashes, 0);
    }
    hashListLocker.unlock();
    gui = nullptr;
}

OpensslPlugin::~OpensslPlugin()
{
    qDebug() << "Destroying Openssl Hashes Plugins" << this;
    hashListLocker.lock();
    OpenSSLHashes::hashList.clear();
    hashListLocker.unlock();
    EVP_cleanup();
    delete gui;

}

QString OpensslPlugin::pluginName() const
{
    return "Openssl Hashes";
}

TransformAbstract *OpensslPlugin::getTransform(QString name)
{
    TransformAbstract *ta = nullptr;

    if (OpenSSLHashes::hashList.contains(name)) {
        ta = new(std::nothrow) OpenSSLHashes(name);
        if (ta == nullptr) {
           qFatal("Cannot allocate memory for OpenSSLHashes (openssl 1) X{");
        }
    }

    return ta;
}

const QStringList OpensslPlugin::getTransformList(QString type)
{
    QStringList ret;
    if (type == DEFAULT_TYPE_HASHES) {
        ret.append(OpenSSLHashes::hashList);
    }
    return ret;
}

const QStringList OpensslPlugin::getTypesList()
{
    return QStringList() << DEFAULT_TYPE_HASHES;
}

QWidget *OpensslPlugin::getConfGui(QWidget * /* parent */)
{
    if (gui == nullptr) {
        QString info;
        info.append(QString("<p>Plugin compiled against %1<br>").arg(OPENSSL_VERSION_TEXT));
        info.append("Plugin currently running with: <ul>");
        info.append(QString("<li>Version: %1</li>").arg(SSLeay_version(SSLEAY_VERSION)));
        info.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_PLATFORM)));
        info.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_CFLAGS)));
        info.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_BUILT_ON)));

        info.append("</ul></p>");
        QLabel * label = new(std::nothrow) QLabel(info);
        if (label == nullptr) {
            qFatal("Cannot allocate memory for QLabel (openssl gui) X{");
            return nullptr;
        }
        label->setWordWrap(true);
        gui = label;
        connect(gui,SIGNAL(destroyed()), SLOT(onGuiDelete()));
    }
    return gui;
}

QString OpensslPlugin::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

int OpensslPlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString OpensslPlugin::pluginVersion() const
{
    return VERSION_STRING;
}

void OpensslPlugin::onGuiDelete()
{
    gui = nullptr;
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(opensslplugin, OpensslPlugin)
#endif
QT_END_NAMESPACE

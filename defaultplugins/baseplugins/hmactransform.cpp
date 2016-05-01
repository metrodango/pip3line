/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hmactransform.h"
#include "confgui/hmactransformwidget.h"
#include <QCryptographicHash>

const QString HMACTransform::id = "HMAC";
const QMap<int, QString> HMACTransform::hashes = HMACTransform::initHash();

HMACTransform::HMACTransform()
{
    selectedHash = QCryptographicHash::Sha1;
}

HMACTransform::~HMACTransform()
{

}

QString HMACTransform::name() const
{
    return id;
}

QString HMACTransform::description() const
{
    return tr("HMAC calculation for common hash algorithms");
}

void HMACTransform::transform(const QByteArray &input, QByteArray &output)
{
    if (key.isEmpty()) {
        emit error(tr("No key specified, using null key"),id);
    }

    int blocksize = getBlocksize(selectedHash);
    if (key.length() > blocksize)
        key = QCryptographicHash::hash(key, (QCryptographicHash::Algorithm)selectedHash); // keys longer than blocksize are shortened

    if (key.length() < blocksize)
        key = key.append(QByteArray(blocksize - key.length(),'\x00')); // keys shorter than blocksize are zero-padded (where ∥ is concatenation)

    QByteArray o_key_pad = QByteArray(blocksize,'\x5c');
    QByteArray i_key_pad = QByteArray(blocksize,'\x36');
    for (int i = 0 ; i < o_key_pad.size(); i++) {
        o_key_pad[i] = o_key_pad[i] ^ key[i];
        i_key_pad[i] = i_key_pad[i] ^ key[i];
    }
    i_key_pad.append(input);
    o_key_pad.append(QCryptographicHash::hash(i_key_pad,(QCryptographicHash::Algorithm)selectedHash));
    output = QCryptographicHash::hash(o_key_pad,(QCryptographicHash::Algorithm)selectedHash);
}

bool HMACTransform::isTwoWays()
{
    return false;
}

QWidget *HMACTransform::requestGui(QWidget *parent)
{
    return new(std::nothrow) HMACTransformWidget(this, parent);
}

QHash<QString, QString> HMACTransform::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLHASHNAME,QString::number(selectedHash));
    properties.insert(XMLKEY,QString::fromUtf8(key.toHex()));
    return properties;
}

bool HMACTransform::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    bool ok = true;
    int val = 0;

    val = propertiesList.value(XMLHASHNAME).toInt(&ok);
    if (!ok || !hashes.contains(val)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLHASHNAME),id);
    } else {
        setSelectedHash(val);
    }

    QString sval = propertiesList.value(XMLKEY);
    setKey(QByteArray::fromHex(sval.toUtf8()));

    return res;
}
int HMACTransform::getSelectedHash() const
{
    return selectedHash;
}

void HMACTransform::setSelectedHash(int value)
{
    if (selectedHash != value) {
        if (hashes.contains(value)) {
            selectedHash = value;
            emit confUpdated();
        } else {
            emit error(tr("Unknown hash identifier :%1").arg(value),id);
        }
    }
}
QByteArray HMACTransform::getKey() const
{
    return key;
}

void HMACTransform::setKey(const QByteArray &value)
{
    if (key != value) {
        key = value;
        emit confUpdated();
    }
}

const QMap<int, QString> HMACTransform::initHash()
{
    QMap<int, QString> list;
    list.insert(QCryptographicHash::Md4,"MD4");
    list.insert(QCryptographicHash::Md5,"MD5");
    list.insert(QCryptographicHash::Sha1,"SHA-1");
#if QT_VERSION >= 0x050000
    list.insert(QCryptographicHash::Sha224,"SHA-224");
    list.insert(QCryptographicHash::Sha256,"SHA-256");
    list.insert(QCryptographicHash::Sha384,"SHA-384");
    list.insert(QCryptographicHash::Sha512,"SHA-512");
#endif
    return list;
}

int HMACTransform::getBlocksize(int hash)
{
    switch (hash) {
    case QCryptographicHash::Md4:
    case QCryptographicHash::Md5:
    case QCryptographicHash::Sha1:
        return 64;
        break;
#if QT_VERSION >= 0x050000
    case QCryptographicHash::Sha224:
    case QCryptographicHash::Sha256:
        return 64;
        break;
    case QCryptographicHash::Sha384:
    case QCryptographicHash::Sha512:
        return 128;
        break;
#endif
    default:
        emit error(tr("Unknown hash identifier for block size:%1").arg(hash),id);
        return 0;
    }
}

QString HMACTransform::help() const
{
    QString help;
    help.append("<p>HMAC calculation algorithm based on different hashes</p><p><b>Note:</b> This transformation is using the internal Qt hashes functions.</p><p>This means that the SHA-224/256/384/512 hashes are only available when using Qt5</p>");
    return help;
}



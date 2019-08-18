/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "sha3_224.h"
#include <QCryptographicHash>

const QString Sha3_224::id = "SHA3-224";

Sha3_224::Sha3_224()
{

}

Sha3_224::~Sha3_224()
{

}

QString Sha3_224::name() const
{
    return id;
}

QString Sha3_224::description() const {
    return tr("Sha3-224 hash");
}

bool Sha3_224::isTwoWays() {
    return false;
}

QString Sha3_224::help() const
{
    QString help;
    help.append("<p>Sha3-224 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

void Sha3_224::transform(const QByteArray &input, QByteArray &output) {
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha3_224);
}

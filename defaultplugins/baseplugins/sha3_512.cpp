/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "sha3_512.h"
#include <QCryptographicHash>

const QString Sha3_512::id = "SHA3-512";

Sha3_512::Sha3_512()
{

}

Sha3_512::~Sha3_512()
{

}

QString Sha3_512::name() const
{
    return id;
}

QString Sha3_512::description() const {
    return tr("Sha3-512 hash");
}

bool Sha3_512::isTwoWays() {
    return false;
}

QString Sha3_512::help() const
{
    QString help;
    help.append("<p>Sha3-512 hash (Keccak)</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

void Sha3_512::transform(const QByteArray &input, QByteArray &output) {
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha3_512);
}

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "sha3_384.h"
#include <QCryptographicHash>

const QString Sha3_384::id = "SHA3-384";

Sha3_384::Sha3_384()
{

}

Sha3_384::~Sha3_384()
{

}

QString Sha3_384::name() const
{
    return id;
}

QString Sha3_384::description() const {
    return tr("Sha3-384 hash");
}

bool Sha3_384::isTwoWays() {
    return false;
}

QString Sha3_384::help() const
{
    QString help;
    help.append("<p>Sha3-384 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

void Sha3_384::transform(const QByteArray &input, QByteArray &output) {
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha3_384);
}


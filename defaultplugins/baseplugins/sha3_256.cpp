/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "sha3_256.h"
#include <QCryptographicHash>

const QString Sha3_256::id = "SHA3-256";

Sha3_256::Sha3_256()
{

}

Sha3_256::~Sha3_256()
{

}

QString Sha3_256::name() const
{
    return id;
}

QString Sha3_256::description() const {
    return tr("Sha3-256 hash");
}

bool Sha3_256::isTwoWays() {
    return false;
}

QString Sha3_256::help() const
{
    QString help;
    help.append("<p>Sha3-256 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

void Sha3_256::transform(const QByteArray &input, QByteArray &output) {
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha3_256);
}


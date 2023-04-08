/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "sha256.h"
#include <QCryptographicHash>

const QString Sha256::id = "SHA256";

Sha256::Sha256()
{
}

Sha256::~Sha256()
{
}

QString Sha256::name() const
{
    return id;
}

QString Sha256::description() const
{
    return tr("Classic Sha256 hash");
}

void Sha256::transform(const QByteArray &input, QByteArray &output)
{
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha256);
}

bool Sha256::isTwoWays()
{
    return false;
}

QString Sha256::help() const
{
    QString help;
    help.append("<p>Classic Sha256 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

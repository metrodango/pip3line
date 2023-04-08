/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "sha512.h"
#include <QCryptographicHash>

const QString Sha512::id = "SHA512";

Sha512::Sha512()
{
}

Sha512::~Sha512()
{
}

QString Sha512::name() const
{
    return id;
}

QString Sha512::description() const
{
    return tr("Classic Sha512 hash");
}

void Sha512::transform(const QByteArray &input, QByteArray &output)
{
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha512);
}

bool Sha512::isTwoWays()
{
    return false;
}

QString Sha512::help() const
{
    QString help;
    help.append("<p>Classic Sha512 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

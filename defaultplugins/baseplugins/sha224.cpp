/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "sha224.h"
#include <QCryptographicHash>

const QString Sha224::id = "SHA224";

Sha224::Sha224()
{
}

Sha224::~Sha224()
{
}

QString Sha224::name() const
{
    return id;
}

QString Sha224::description() const
{
    return tr("Classic Sha224 hash");
}

void Sha224::transform(const QByteArray &input, QByteArray &output)
{
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha224);
}

bool Sha224::isTwoWays()
{
    return false;
}

QString Sha224::help() const
{
    QString help;
    help.append("<p>Classic Sha224 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

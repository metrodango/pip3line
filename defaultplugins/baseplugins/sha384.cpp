/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "sha384.h"
#include <QCryptographicHash>

const QString Sha384::id = "SHA384";

Sha384::Sha384()
{
}

Sha384::~Sha384()
{
}

QString Sha384::name() const
{
    return id;
}

QString Sha384::description() const
{
    return tr("Classic Sha384 hash");
}

void Sha384::transform(const QByteArray &input, QByteArray &output)
{
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha384);
}

bool Sha384::isTwoWays()
{
    return false;
}

QString Sha384::help() const
{
    QString help;
    help.append("<p>Classic Sha384 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

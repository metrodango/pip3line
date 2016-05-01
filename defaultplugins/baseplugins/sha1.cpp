/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "sha1.h"
#include <QCryptographicHash>

const QString Sha1::id = "SHA1";

Sha1::Sha1()
{

}

Sha1::~Sha1()
{
}

QString Sha1::name() const {
    return id;
}

QString Sha1::description() const {
    return tr("Classic Sha1 hash");
}

bool Sha1::isTwoWays() {
    return false;
}

QString Sha1::help() const
{
    QString help;
    help.append("<p>Classic Sha1 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

void Sha1::transform(const QByteArray &input, QByteArray &output) {
    output = QCryptographicHash::hash(input,QCryptographicHash::Sha1);
}

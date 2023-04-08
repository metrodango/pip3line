/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "md5.h"
#include <QCryptographicHash>

const QString Md5::id = "MD5";

QString Md5::name() const {
    return id;
}

QString Md5::description() const {
    return tr("Classic MD5 hash function");
}

bool Md5::isTwoWays() {
    return false;
}

QString Md5::help() const
{
    QString help;
    help.append("<p>Classic MD5 hash</p><p>This transformation is using the QT internal hash function.</p>");
    return help;
}

void Md5::transform(const QByteArray &input, QByteArray &output) {
    output = QCryptographicHash::hash(input,QCryptographicHash::Md5);
}

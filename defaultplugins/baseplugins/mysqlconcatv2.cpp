/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "mysqlconcatv2.h"

const QString MySqlConcatv2::id = "MySql concat v2";

QString MySqlConcatv2::name() const {
    return id;
}

QString MySqlConcatv2::description() const {
    return tr("Produce an MySql concatenated string (2nd version)");
}

void MySqlConcatv2::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    if (input.isEmpty())
        return;

    output.append("0x").append(input.toHex());

}

bool MySqlConcatv2::isTwoWays() {
    return false;
}

QString MySqlConcatv2::help() const
{
    QString help;
    help.append("<p>Produce an MySql concatenated string from the input (UTF-8)</p><p>This version uses the hexadecimal representation</p>");
    return help;
}

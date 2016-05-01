/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "mysqlconcat.h"

const QString MysqlConcat::id = "MySql concat";

QString MysqlConcat::name() const {
    return id;
}

QString MysqlConcat::description() const {
    return tr("Produce an MySql concatenated string");
}

bool MysqlConcat::isTwoWays() {
    return false;
}

QString MysqlConcat::help() const
{
    QString help;
    help.append("<p>Produce an MySql concatenated string from the input (UTF-8)</p><p>This version uses char()</p>");
    return help;
}

void MysqlConcat::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    output.append("concat(");
    for (int i = 0; i < input.size(); i++) {
        output.append("char(").append(QByteArray::number(uchar(input.at(i)), 10)).append("),");
    }
    output.chop(1);
    output.append(")");
}

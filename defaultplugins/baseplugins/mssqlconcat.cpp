/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "mssqlconcat.h"

const QString MSSqlConcat::id = "MSSql concat";

QString MSSqlConcat::name() const {
    return id;
}

QString MSSqlConcat::description() const {
    return tr("Produce an MSSql concatenated string");
}

bool MSSqlConcat::isTwoWays() {
    return false;
}

QString MSSqlConcat::help() const
{
    QString help;
    help.append("<p>Produce an MSSql concatenated string from the input (UTF-8)</p>");
    return help;
}

void MSSqlConcat::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    for (int i = 0; i < input.size(); i++) {
        output.append("char(").append(QByteArray::number(uchar(input.at(i)), 10)).append(")+");
    }
    output.chop(1);
}

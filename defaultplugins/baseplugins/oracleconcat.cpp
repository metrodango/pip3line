/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "oracleconcat.h"


const QString OracleConcat::id = "Oracle concat";

QString OracleConcat::name() const {
    return id;
}

QString OracleConcat::description() const {
    return tr("Produce an Oracle concatenated string");
}

bool OracleConcat::isTwoWays() {
    return false;
}

void OracleConcat::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    for (int i = 0; i < input.size(); i++) {
        output.append("chr(").append(QByteArray::number(uchar(input.at(i)), 10)).append(")||");
    }
    output.chop(2);
}

QString OracleConcat::help() const
{
    QString help;
    help.append("<p>Produce an Oracle concatenated string from the input (UTF-8)</p>");
    return help;
}

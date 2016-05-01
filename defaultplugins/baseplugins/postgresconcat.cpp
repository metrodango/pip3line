/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "postgresconcat.h"

const QString PostgresConcat::id = "Postgres concat";

QString PostgresConcat::name() const {
    return id;
}

QString PostgresConcat::description() const {
    return tr("Produce an Postgres concatenated string");
}

bool PostgresConcat::isTwoWays() {
    return false;
}

QString PostgresConcat::help() const
{
    QString help;
    help.append("<p>Produce an Postgres concatenated string from the input (UTF-8)</p>");
    return help;
}

void PostgresConcat::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    for (int i = 0; i < input.size(); i++) {
        output.append("CHR(").append(QByteArray::number(uchar(input.at(i)), 10)).append(")||");
    }
    output.chop(2);
}


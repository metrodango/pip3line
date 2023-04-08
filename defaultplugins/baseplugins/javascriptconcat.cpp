/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "javascriptconcat.h"

const QString JavaScriptConcat::id = "JavaScript concat";

QString JavaScriptConcat::name() const {
    return id;
}

QString JavaScriptConcat::description() const {
    return tr("Produce an JavaScript concatenated string");
}

bool JavaScriptConcat::isTwoWays() {
    return false;
}

QString JavaScriptConcat::help() const
{
    QString help;
    help.append("<p>Produce an JavaScript concatenated string from the input (UTF-8)</p><p>This version is using String.fromCharCode()</p>");
    return help;
}

void JavaScriptConcat::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    output.append("String.fromCharCode(");
    for (int i = 0; i < input.size(); i++) {
        output.append(QByteArray::number(uchar(input.at(i)), 10)).append(",");
    }
    output.chop(1);
    output.append(")");
}

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "numbertochar.h"
#include "confgui/numbertocharwidget.h"
#include <QDebug>

const QString NumberToChar::id = "short to Char";

const char NumberToChar::DEFAULT_SEPARATOR = '_';

NumberToChar::NumberToChar()
{
    separator = DEFAULT_SEPARATOR;
    signedShort = true;
}

NumberToChar::~NumberToChar()
{
}

QString NumberToChar::name() const
{
    return id;
}

QString NumberToChar::description() const
{
    return tr("Convert list of signed short to an array of char.");
}

void NumberToChar::transform(const QByteArray &input, QByteArray &output)
{
    bool ok;
    if (wayValue == TransformAbstract::INBOUND) {

        QByteArray num;
        for (int i = 0; i < input.size(); i++) {
            if ((signedShort && input.at(i) ==  '-') || (input.at(i) > 47 && input.at(i) < 58)) {
                num.append(input.at(i));
            }
            else {
                if (signedShort) {
                int val;
                val = num.toInt(&ok);

                if (ok && !(val <  SCHAR_MIN || val > SCHAR_MAX )) {
                    output.append((char)val);
                } else {
                    emit error(tr("Invalid number"),id);
                }

                }
                num.clear();
            }
        }
    } else {
        for (int i = 0; i < input.size(); i++) {
            if (signedShort)
                output.append(QByteArray::number((qint8)input.at(i)));
            else
                output.append(QByteArray::number((quint8)input.at(i)));
            output.append(separator);
        }
        output.chop(1);
    }
}

bool NumberToChar::isTwoWays()
{
    return true;
}

QString NumberToChar::help() const
{
    QString help;
    help.append("<p>Convert list of signed short to an array of char.</p><p> Input delimiters can be anything apart from the sign character '-' and numbers chararacters.</p><p>Any other characters will be silently ignored</p>");
    return help;
}

QWidget *NumberToChar::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) NumberToCharWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for NumberToCharWidget X{");
    }
    return widget;
}

QHash<QString, QString> NumberToChar::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLSEPARATOR,saveChar(separator));
    return properties;
}

bool NumberToChar::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    QString tmp = propertiesList.value(XMLSEPARATOR);
    char tmpChar = '\x00';
    if (!loadChar(tmp,&tmpChar)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLSEPARATOR),id);
    } else {
        res = setSeparator(tmpChar) && res;
    }

    return res;
}

QString NumberToChar::inboundString() const
{
    return "Signed short to char";
}

QString NumberToChar::outboundString() const
{
    return "Char to signed short";
}

char NumberToChar::getSeparator() const
{
    return separator;
}

bool NumberToChar::setSeparator(char c)
{
    if (c == '-') {
        emit error("Cannot use the negative sign as separator",id);
        return false;
    }

    if ((c > 47 && c < 58) ) {
        emit error("Cannot use a number as separator",id);
        return false;
    }
    if (separator != c) {
        separator = c;
        emit confUpdated();
    }

    return true;
}


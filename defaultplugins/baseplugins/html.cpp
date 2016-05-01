/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "html.h"
#include "confgui/htmlwidget.h"

#include <QTextDocument>

const QString Html::id = "Html";
const QHash<char,QString> Html::reserverdCharNames = _initializeReservedChar();

Html::Html()
{
    useName = false;
    useHexadecimal = true;
    encodeAll = false;
}

Html::~Html()
{
}


QString Html::name() const {
    return id;
}

QString Html::description() const {
    return tr("Html encode");
}

void Html::transform(const QByteArray &input, QByteArray &output) {
    output.clear();

    if (wayValue == TransformAbstract::INBOUND) {
        for (int i = 0; i < input.size(); i++) {

            if (encodeAll) {
                output.append(encode(input.at(i)));
            } else {
                if (reserverdCharNames.contains(input.at(i))) {
                    output.append(encode(input.at(i)));
                } else {
                    output.append(input.at(i));
                }
            }
        }
    } else {
        QTextDocument text;
        text.setHtml(QString::fromUtf8(input));
        output = text.toPlainText().toUtf8();
    }
}

bool Html::isTwoWays() {
    return true;
}

QHash<QString, QString> Html::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLUSEENTITYNAMES,QString::number(useName ? 1 : 0));
    properties.insert(XMLUSEHEXA,QString::number(useHexadecimal ? 1 : 0));
    properties.insert(XMLENCODEALL,QString::number(encodeAll ? 1 : 0));

    return properties;
}

bool Html::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLUSEENTITYNAMES).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLUSEENTITYNAMES),id);
    } else {
        setUseName(val == 1);
    }

    val = propertiesList.value(XMLENCODEALL).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLENCODEALL),id);
    } else {
        setEncodeAll(val == 1);
    }
    val = propertiesList.value(XMLUSEHEXA).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLUSEHEXA),id);
    } else {
        setUseHexadecimal(val == 1);
    }

    return res;
}

QWidget *Html::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) HtmlWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for HtmlWidget X{");
    }
    return widget;
}

QString Html::help() const
{
    QString help;
    help.append("<p>HTML entities encoding</p><p>HTML encode the input, following certain rules.<br>You can choose to encode only HTML \"reserved\" characters or just everything.<br>Different formats are available: decimal, hexadecimal, entities names (This only apply to encoding, this transformation decode from any format).</p>");
    return help;
}

QByteArray Html::encode(char c)
{
    QByteArray ret;
    ret.append('&');
    if (useName && reserverdCharNames.contains(c)) {
        ret.append(reserverdCharNames[c]);
    } else if (useHexadecimal) {
        ret.append('#').append('x');
        QByteArray byt;
        byt.append(c);
        ret.append(byt.toHex());
    } else {
        ret.append('#').append(QByteArray::number(int(c), 10));
    }
    ret.append(';');
    return ret;
}

QHash<char,QString> Html::_initializeReservedChar()
{
    QHash<char,QString> ret;

    ret['<'] = "lt";
    ret['>'] = "gt";
    ret['&'] = "amp";
    ret['\''] = "apos";
    ret['"'] = "quot";

    return ret;
}

bool Html::doWeUseName()
{
    return useName;
}

bool Html::doWeUseHexadecimal()
{
    return useHexadecimal;
}

bool Html::doWeEncodeAll()
{
    return encodeAll;
}

void Html::setUseName(bool val)
{
    useName = val;
    emit confUpdated();
}

void Html::setUseHexadecimal(bool val)
{
    useHexadecimal = val;
    emit confUpdated();
}

void Html::setEncodeAll(bool val)
{
    encodeAll = val;
    emit confUpdated();
}




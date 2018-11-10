/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "xor.h"
#include <QDebug>
#include "confgui/xorwidget.h"

const QString Xor::id = "Xor";
const QString Xor::XMLXORALGORITHM = "Algorithm";

Xor::Xor()
{
    hexDecode = false;
    xortype = Basic;
}

Xor::~Xor()
{
}


QString Xor::name() const {
    return id;
}

QString Xor::description() const {
    return tr("Basic xor cipher");
}

bool Xor::isTwoWays() {
    return true;
}

QHash<QString, QString> Xor::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLKEY,QString(key.toBase64()));
    properties.insert(XMLFROMHEX,QString::number(hexDecode ? 1 : 0));
    properties.insert(XMLXORALGORITHM, QString::number(xortype));
    return properties;
}

bool Xor::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLFROMHEX).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLFROMHEX),id);
    } else {
        setFromHex(val == 1);
    }

    val = propertiesList.value(XMLXORALGORITHM).toInt(&ok);
    if (!ok || (val != Basic && val != PREVIOUSINPUT && val != PREVIOUSOUTPUT)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLXORALGORITHM),id);
    } else {
        setType(static_cast<Xor::Type>(val));
    }

    setKey(QByteArray::fromBase64(propertiesList.value(XMLKEY).toUtf8()));
    return res;
}

QWidget *Xor::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) XorWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for XorWidget X{");
    }
    return widget;
}

QString Xor::help() const
{
    QString help;
    help.append("<p>Basic xor cipher</p><p>Xor the given key with the input.</p>");
    return help;
}

void Xor::transform(const QByteArray &input, QByteArray &output){

    output.clear();

    QByteArray finalKey;
    if (hexDecode)
        finalKey = fromHex(key);
    else
        finalKey = key;

    if (finalKey.size() < 1) {
        emit error(tr("Key size is zero, nothing done."),id);
        return;
    }

    switch (xortype) {
        case Basic:
            {
                if (finalKey.size() < input.size()) {
                    emit warning(tr("Key length (%1) is inferior to the data length (%2). Reusing the key multiple time.").arg(finalKey.size()).arg(input.size()),id);
                }
                for (int i = 0 ; i < input.size(); i++) {
                    output.append(input.at(i) ^ finalKey.at(i % finalKey.size()));
                }
            }
            break;
        case PREVIOUSINPUT:
            {
                if (wayValue == INBOUND) {
                    output = applyPreviousInput(input,finalKey);
                } else {
                    output = applyPreviousOutput(input,finalKey);
                }

            }
            break;
        case PREVIOUSOUTPUT:
            {
                if (wayValue == INBOUND) {
                    output = applyPreviousOutput(input,finalKey);
                } else {
                    output = applyPreviousInput(input,finalKey);
                }
            }
            break;
        default:
            emit error(tr("Unknown Xor type T_T"),id);
    }
}

QByteArray Xor::getKey() const
{
    return key;
}

void Xor::setKey(QByteArray val)
{
    if (key != val) {
        key = val;
        emit confUpdated();
    }
}

bool Xor::isFromHex() const
{
    return hexDecode;
}

void Xor::setFromHex(bool val)
{
    if (hexDecode != val) {
        hexDecode = val;
        emit confUpdated();
    }
}

void Xor::setType(Xor::Type val)
{
    if (xortype != val) {
        xortype = val;
        emit confUpdated();
    }
}

Xor::Type Xor::getType() const
{
    return xortype;
}

QByteArray Xor::applyPreviousInput(const QByteArray &data, QByteArray & key)
{
    QByteArray output;
    QByteArray temp = data;
    temp.prepend(key);

    for (int i = key.size() ; i < temp.size(); i++) {
        output.append(temp.at(i) ^ temp.at(i - key.size()));
    }

    return output;
}

QByteArray Xor::applyPreviousOutput(const QByteArray &data, QByteArray &key)
{
    QByteArray output = key;

    for (int i = 0 ; i < data.size(); i++) {
        output.append(data.at(i) ^ output.at(i));
    }
    output = output.mid(key.size());

    return output;
}

QString Xor::inboundString() const
{
    return tr("Encrypt");
}

QString Xor::outboundString() const
{
    return tr("Decrypt");
}

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "rotx.h"
#include "confgui/rotxwidget.h"

const QString Rotx::id = "ROTx";

Rotx::Rotx()
{
    choosenVariant = ROT13;
}

Rotx::~Rotx()
{
}

QString Rotx::name() const {
    return id;
}

QString Rotx::description() const {
    return tr("Character rotation Cipher. Also known as the Caesar Cipher");
}

void Rotx::transform(const QByteArray &input, QByteArray &output) {
    output = input;
    if (choosenVariant == ROT13) {
        for (int i = 0; i < output.size(); i++) {
            if (output[i] >= 'a' && output[i] <= 'z') {
                if (output[i] > 'm')
                    output[i] = output[i] - 13;
                else
                    output[i] = output[i] + 13;
            } else if (output[i] >= 'A' && output[i] <= 'Z') {
                if (output[i] > 'M')
                    output[i] = output[i] - 13;
                else
                    output[i] = output[i] + 13;
            }
        }
    } else if (choosenVariant == ROT5) {
        for (int i = 0; i < output.size(); i++) {
            if (output[i] >= '0' && output[i] <= '9') {
                if (output[i] > '4')
                    output[i] = output[i] - 5;
                else
                    output[i] = output[i] + 5;
            }
        }
    } else if (choosenVariant == ROT47) {
        for (int i = 0; i < output.size(); i++) {
            if (output[i] >= '!' && output[i] <= '~') {
                if (output[i] > 'P')
                    output[i] = output[i] - 47;
                else
                    output[i] = output[i] + 47;
            }
        }
    }
}

bool Rotx::isTwoWays() {
    return false;
}

QHash<QString, QString> Rotx::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLVARIANT,QString::number((int)choosenVariant));

    return properties;
}

bool Rotx::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLVARIANT).toInt(&ok);
    if (!ok || (val != ROT13 && val != ROT47 && val != ROT5)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLVARIANT),id);
    }
    setRotVariant((Rotx::RotVariant)val);
    return res;
}

QWidget *Rotx::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) RotXWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for RotXWidget X{");
    }
    return widget;
}

QString Rotx::help() const
{
    QString help;
    help.append("<p>Character rotation \"Cipher\".</p><p>Variant available: <ul><li>ROT13, also known as the Caesar Cipher. This is used within the Microsoft Windows registry to obfuscate certain strings.</li><li>ROT5, which rotates numbers characters only</li><li>ROT47, which rotates all alphanumeriques characters</li></ul></p>");
    return help;
}

Rotx::RotVariant Rotx::getRotVariant()
{
    return choosenVariant;
}

void Rotx::setRotVariant(Rotx::RotVariant val)
{
    choosenVariant = val;
    emit confUpdated();
}



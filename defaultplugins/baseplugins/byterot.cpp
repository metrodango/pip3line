/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "byterot.h"
#include "confgui/byterotwidget.h"

const QString ByteRot::id = "Byte ROT";

ByteRot::ByteRot()
{
    rotation = 13;
}

ByteRot::~ByteRot()
{

}

QString ByteRot::name() const
{
    return id;
}

QString ByteRot::description() const
{
    return tr("Byte value rotation");
}

void ByteRot::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    int sign = (wayValue == INBOUND ? 1 : -1);
    for (int i = 0; i < input.size(); i++) {
        output.append(input.at(i) + static_cast<char>(sign * rotation));
    }
}

bool ByteRot::isTwoWays()
{
    return true;
}

QWidget *ByteRot::requestGui(QWidget *parent)
{
    QWidget * ret = new(std::nothrow) ByteRotWidget(this, parent);
    if (ret == nullptr) {
        qFatal("Cannot allocate memory for ByteRotWidget X{");
    }
    return ret;
}

QString ByteRot::help() const
{
    QString ret;
    ret.append(tr("<p>Implement a rotation algorithm over the (extended) ASCII table.</p><p>This is used by some application to obfuscate data.</p><p>For instance the general.config.obscure_value parameter in Firefox apply such algorithm and it is used to obfuscate configuration files.</p><p>The actual Firefox algorithm is byte += 512 + rotation but this is not what is implemented here. One would need to guess why ;).</p><p>This is pretty useless as a protection though.</p>"));
    return ret;
}

QHash<QString, QString> ByteRot::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLKEY,QString::number(rotation));

    return properties;
}

bool ByteRot::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;
    int val = propertiesList.value(XMLKEY).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLKEY),id);
    } else {
        setRotation(val);
    }

    return res;
}

int ByteRot::getRotation() const
{
    return rotation;
}

void ByteRot::setRotation(int rot)
{
    if (rotation != rot) {
        rotation = rot % 256;
        emit confUpdated();
    }
}

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "jsonvalue.h"
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <cmath>
#include "confgui/jsonvaluewidget.h"

const QString JsonValue::id = "JsonValue";

JsonValue::JsonValue() :
    outputJsonFormat(QJsonDocument::Compact)
{
}

JsonValue::~JsonValue()
{

}

QString JsonValue::name() const
{
    return id;
}

QString JsonValue::description() const
{
    return tr("Extract value from Json doc Root");
}

void JsonValue::transform(const QByteArray &input, QByteArray &output)
{
    QJsonDocument jdoc;
    QJsonParseError parseError;

    jdoc = QJsonDocument::fromJson(input,&parseError);

    if (parseError.error == QJsonParseError::NoError) {
        if (jdoc.isObject()) {
            QJsonObject jobj = jdoc.object();
            if (jobj.contains(valueName)) {
                QJsonValue val = jobj.value(valueName);
                jsonValueToByteArray(val, output);
            }
        } else if (jdoc.isArray()) {
            QJsonArray ja = jdoc.array();
            bool ok = false;
            int index = valueName.toInt(&ok);
            if (!ok) {
                emit error("Json root document is an array, but the value name is not an integer",id);
            } else {
                QJsonValue val = ja.at(index);
                jsonValueToByteArray(val, output);
            }
        }
    }
}

bool JsonValue::isTwoWays()
{
    return false;
}

QHash<QString, QString> JsonValue::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLVALUE,valueName);
    properties.insert(XMLTYPE,QString::number(static_cast<int>(outputJsonFormat)));
    return properties;
}

bool JsonValue::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    bool ok = true;

    int val = propertiesList.value(XMLTYPE).toInt(&ok);
    if (!ok || ( val != QJsonDocument::Compact && val != QJsonDocument::Indented)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLTYPE),id);
    } else {
        setOutputJsonFormat(static_cast<QJsonDocument::JsonFormat>(val));
    }
    valueName = propertiesList.value(XMLVALUE);
    return res;
}

QWidget *JsonValue::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) JsonValueWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for JsonValueWidget X{");
    }
    return widget;
}

QString JsonValue::help() const
{
    QString help;
    help.append("<p>Extract Json value from Json document</p><p>Just specify the name, and enjoy.</p>");
    return help;
}

QString JsonValue::getValueName() const
{
    return valueName;
}

void JsonValue::setValueName(const QString &value)
{
    if (value != valueName) {
        valueName = value;
        emit confUpdated();
    }
}

QJsonDocument::JsonFormat JsonValue::getOutputJsonFormat() const
{
    return outputJsonFormat;
}

void JsonValue::setOutputJsonFormat(const QJsonDocument::JsonFormat &value)
{
    if (value != outputJsonFormat) {
        outputJsonFormat = value;
        emit confUpdated();
    }
}

void JsonValue::jsonValueToByteArray(QJsonValue val, QByteArray &output)
{
    if (val.isObject()) {
        QJsonObject fv = val.toObject();
        QJsonDocument ojdoc(fv);
        output = ojdoc.toJson(outputJsonFormat);
    } else if (val.isArray()) {
        QJsonArray fv = val.toArray();
        QJsonDocument ojdoc(fv);
        output = ojdoc.toJson(outputJsonFormat);
    } else if (val.isString()) {
        output = val.toString().toUtf8();
    } else if (val.isBool()) {
        output = (val.toBool() ? "true" : "false");
    } else if (val.isNull()) {
        output = "null";
    } else if (val.isDouble()) {
        double out = val.toDouble();
        double intprt;
        if (std::modf(out,&intprt) == 0.0) { // this is an integer
            output = QString::number(static_cast<qint64>(intprt)).toUtf8();
        } else {
            output = QString::number(out,'g',10).toUtf8();
        }
    } else {
        output = "undefined";
    }
}

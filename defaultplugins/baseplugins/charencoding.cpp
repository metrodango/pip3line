/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "charencoding.h"
#include "confgui/charencodingwidget.h"
#include <QTextStream>
#include <QTextCodec>
#include <QTextEncoder>
#include <QTextDecoder>
#include <QtAlgorithms>

const QString CharEncoding::id = "Char encoding";
const QString CharEncoding::IGNORE_BOM_XML = "ignoreBOM";
const QString CharEncoding::CONVERT_INVALID_TO_NULL_XML = "InvalidToNull";

CharEncoding::CharEncoding()
{
    QList<QByteArray> codecs =  QTextCodec::availableCodecs();
    qSort(codecs);
    codecName = codecs.at(0);
    includeHeader = false;
    convertInvalidToNull = true;
}

CharEncoding::~CharEncoding()
{
}

QString CharEncoding::name() const {
    return id;
}

QString CharEncoding::description() const {
    return tr("Convert the input into different character encoding standards.");
}

bool CharEncoding::isTwoWays() {
    return true;
}

QHash<QString, QString> CharEncoding::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLCODECNAME,QString(codecName));
    properties.insert(IGNORE_BOM_XML, QString::number(includeHeader ? 1 : 0));
    properties.insert(CONVERT_INVALID_TO_NULL_XML, QString::number(convertInvalidToNull ? 1 : 0));
    return properties;
}

bool CharEncoding::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    res = setCodecName(propertiesList.value(XMLCODECNAME).toUtf8()) && res;
    bool ok = false;
    int val = propertiesList.value(IGNORE_BOM_XML).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(IGNORE_BOM_XML),id);
    } else {
        setIncludeHeader(val == 1);
    }

    val = propertiesList.value(CONVERT_INVALID_TO_NULL_XML).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(CONVERT_INVALID_TO_NULL_XML),id);
    } else {
        setConvertInvalidToNull(val == 1);
    }

    return res;
}

QWidget *CharEncoding::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) CharEncodingWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for CharEncodingWidget X{");
    }
    return widget;
}

QString CharEncoding::help() const
{
    QString help;
    help.append("<p>Character encoding converter</p><p>This transformation is used to convert the input to a specific Character encoding (UTF-8, UTF-16 ...).</p><p>Note: <ul><li>Encoding: UTF-8 => [Choosen encoding]</li><li>Decoding: [Choosen encoding] => UTF-8</li></ul></p>");
    return help;
}

QByteArray CharEncoding::getCodecName()
{
    return codecName;
}

bool CharEncoding::setCodecName(QByteArray val)
{
    if (val != codecName) {
        QTextCodec *codec = QTextCodec::codecForName(val);
        if (codec == nullptr) {
            emit error(tr("No codec named: %1").arg(QString(val)),id);
            return false;
        } else {
            codecName = val;
            emit confUpdated();
        }
    }
    return true;
}
bool CharEncoding::getIncludeHeader() const
{
    return includeHeader;
}

void CharEncoding::setIncludeHeader(bool value)
{
    if (value != includeHeader) {
        includeHeader = value;
        emit confUpdated();
    }
}
bool CharEncoding::getConvertInvalidToNull() const
{
    return convertInvalidToNull;
}

void CharEncoding::setConvertInvalidToNull(bool value)
{
    if (convertInvalidToNull != value) {
        convertInvalidToNull = value;
        emit confUpdated();
    }
}

void CharEncoding::transform(const QByteArray &input, QByteArray &output) {
    output.clear();

    QTextCodec *codec = QTextCodec::codecForName(codecName);
    if (codec == nullptr) {
        emit error("Unknown codec",id);
        return;
    }
    QTextCodec::ConversionFlags flags = QTextCodec::DefaultConversion;
    if (!includeHeader) {
        flags |= QTextCodec::IgnoreHeader;
    }

    if (convertInvalidToNull) {
        flags |= QTextCodec::ConvertInvalidToNull;
    }
    if (wayValue == TransformAbstract::INBOUND) {
        QString init = QString::fromUtf8(input);
        QTextEncoder *encoder = codec->makeEncoder(flags);
        output = encoder->fromUnicode(init);
        if (encoder->hasFailure()) {
            emit error(tr("Errors were encountered while encoding"),id);
        }
    } else {
        QTextDecoder *decoder = codec->makeDecoder(flags);
        output = decoder->toUnicode(input).toUtf8();
        if (decoder->hasFailure()) {
            emit error(tr("Errors were encountered while decoding"),id);
        }
    }
}


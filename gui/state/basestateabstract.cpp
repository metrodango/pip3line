/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "basestateabstract.h"
#include <QSettings>
#include <QDebug>
#include <QThread>
#include "../shared/guiconst.h"

const QChar BaseStateAbstract::B_ZERO = '0';
const QChar BaseStateAbstract::B_ONE = '1';
const char BaseStateAbstract::COMPRESSED_MARKER = '#';

BaseStateAbstract::BaseStateAbstract(QObject *parent) :
    QObject(parent),
    writer(nullptr),
    reader(nullptr),
    flags(0)
{
}

BaseStateAbstract::~BaseStateAbstract()
{

}

QString BaseStateAbstract::getName()
{
    if (name.isEmpty()) {
        name = metaObject()->className();
    }

    return name;
}

void BaseStateAbstract::setName(const QString &value)
{
    name = value;
}

void BaseStateAbstract::start()
{
    run();
    emit finished();
    deleteLater();
}

void BaseStateAbstract::logLostInTranslation(QString message)
{
    qCritical() << message << tr("%1[%2]").arg(reader->name().toString()).arg(reader->tokenString());
}

void BaseStateAbstract::logWasExpectingClosing(QString source, QString expected)
{
    qCritical() << tr("{%4}[%1] Was expecting a closing tag (%5) here, but got %2[%3] instead")
                   .arg(source)
                   .arg(reader->name().toString())
                   .arg(reader->tokenString())
                   .arg(metaObject()->className())
                   .arg(expected);
}

void BaseStateAbstract::logReaderIsNull(QString source)
{
    qCritical() << tr("{%2}[%1]: reader is null T_T").arg(source).arg(metaObject()->className());
}

void BaseStateAbstract::logWriterIsNull(QString source)
{
    qCritical() << tr("{%2}[%1]: writer is null T_T").arg(source).arg(metaObject()->className());
}

void BaseStateAbstract::logNotAdequate(QString source, QString expected)
{
    qCritical() << tr("{%5}[%1] Current element is not adequate: %2[%3] instead of %4[StartElement]")
                   .arg(source)
                   .arg(reader->name().toString())
                   .arg(reader->tokenString())
                   .arg(expected)
                   .arg(metaObject()->className());
}

void BaseStateAbstract::logCannotRead(QString source)
{
    qCritical() << tr("{%3}[%1] Cannot read next element :%2 (%4[%5])")
                   .arg(source)
                   .arg(reader->errorString())
                   .arg(metaObject()->className())
                   .arg(reader->name().toString())
                   .arg(reader->tokenString());
}

void BaseStateAbstract::logStatus(QString mess)
{
    qWarning() << tr("%4 : %1[%2] errors:%3")
                   .arg(reader->name().toString())
                   .arg(reader->tokenString())
                   .arg(reader->errorString())
                   .arg(mess);
}

quint64 BaseStateAbstract::getFlags() const
{
    return flags;
}

void BaseStateAbstract::setFlags(const quint64 &value)
{
    flags = value;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        actionName = GuiConst::STATE_ACTION_SAVE_STR;
    } else {
        actionName = GuiConst::STATE_ACTION_RESTORE_STR;
    }
}

bool BaseStateAbstract::readNext(QString expected)
{
    if (reader != nullptr) {
        if (reader->readNext() == QXmlStreamReader::StartElement) {
            attributes = reader->attributes();
        }
        if (!expected.isEmpty() && reader->name() != expected) {
            logNotAdequate("readNext",expected );
        }
    } else {
        logReaderIsNull("readNext:String");
    }
    return false;
}

bool BaseStateAbstract::readNextStart()
{
    return readNextStart(getName());
}

// read next and return true if the next start element is "expected"
// if expected is empty ignore and jsut return true if there is a start element

bool BaseStateAbstract::readNextStart(QString expected)
{
    if (reader != nullptr) {

        if (reader->readNextStartElement()) {
            if (expected.isEmpty())
                return true;

            if (reader->name() == expected) {
                attributes = reader->attributes();
              //  qDebug() << "opening: " << expected;
                return true;
            } else {
                logNotAdequate("readNextStart",expected );
            }
        } else {
            logCannotRead("readNextStart:String");
        }
    } else {
        logReaderIsNull("readNextStart:String");
    }
    return false;
}

bool BaseStateAbstract::skipUntilStartElement()
{
    if (reader != nullptr) {
        while (reader->tokenType() == QXmlStreamReader::EndElement) {
          //  qDebug() << tr("Skipping %1[%2]").arg(reader->name().toString()).arg(reader->tokenString());
            if (reader->readNext() == QXmlStreamReader::Invalid) {
                qCritical() << tr("Cannot read element(Invalid) %1 ").arg(reader->errorString());
                return false;
            }
        }
        if (reader->tokenType() == QXmlStreamReader::StartElement) {
            attributes = reader->attributes();
            return true;
        } else {
            logCannotRead("skipUntilStartElement");
        }
    } else {
        logReaderIsNull("skipUntilStartElement");
    }
    return false;
}

bool BaseStateAbstract::skipUntilStartElement(QString expected)
{
    if (skipUntilStartElement()) {
        if (reader->name() == expected) {
            return true;
        } else {
            logNotAdequate("skipUntilStartElement:String",expected );
        }
    }
    return false;
}

bool BaseStateAbstract::genCloseElement()
{
    bool ok = false;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        ok = writeCloseElement();
    } else {
        ok = readEndElement();
    }

    return ok;
}

bool BaseStateAbstract::writeCloseElement()
{
    if (writer != nullptr) {
        writer->writeEndElement();
        return true;
    } else {
        logWriterIsNull("writeCloseElement");
    }
    return false;
}

bool BaseStateAbstract::readEndElement()
{
    if (reader != nullptr) {
    //    qDebug() << "closing element " << reader->name();
        if (reader->readNext() == QXmlStreamReader::EndElement) { // closing element
            return true;
        } else {
            logWasExpectingClosing("readEndElement", QString());
        }
    } else {
        logReaderIsNull("readEndElement");
    }

    return false;
}

bool BaseStateAbstract::readEndElement(QString expected)
{
    if (readEndElement()) {
        if (reader->name() == expected)
            return true;
        else
            logNotAdequate("readEndElement:String",expected );
    } else {
        logStatus();
    }
    return false;
}

bool BaseStateAbstract::readEndAndNext(QString expected)
{
    if (reader != nullptr) {
       // qDebug() << "closing element " << reader->name();
        if (reader->readNext() == QXmlStreamReader::EndElement) { // closing element
           // qDebug() << "closing element " << reader->name();
            if (reader->name() != expected) {
                logNotAdequate("readEndAndNext:String",expected );
            }
            if (reader->readNext() == QXmlStreamReader::StartElement) {
                attributes = reader->attributes();// if next is start set attributes
              //  qDebug() << "Next element " << reader->name() << reader->tokenString();
            }
            return true;
        } else {
            logWasExpectingClosing("readEndAndNext", expected);
        }
    } else {
        logReaderIsNull("closeAndReadNext");
    }
    return false;
}

QString BaseStateAbstract::write(QByteArray data)
{
    if (data.size() > 20000000) {
        emit log(tr("Saving %1 bytes of data. That's a fat cat you have here, it will take some time to mush.").arg(data.size()), "Save state",Pip3lineConst::LWARNING );
    }
    return byteArrayToString(data);
}

QString BaseStateAbstract::byteArrayToString(QByteArray data, bool compress)
{
    QString ret;
    if (compress)
        data = qCompress(data,9).toBase64();
    else
        data = data.toBase64();
    ret = QString::fromUtf8(data.constData(), data.size());
    return ret;
}

QByteArray BaseStateAbstract::readByteArray(QStringRef data)
{
    if (data.size() > 10000000) {
        emit log(tr("Restoring %1 bytes of data. That's a fat cat you have here, it will take some time to reconstitute.").arg(data.size()), "Save state",Pip3lineConst::LWARNING );
    }
    return stringToByteArray(data.toString());;
}

QByteArray BaseStateAbstract::readByteArray(QString data)
{
    return stringToByteArray(data);
}

QByteArray BaseStateAbstract::stringToByteArray(QString data, bool uncompress)
{
    QByteArray ret;
    if (data.isEmpty())
        return ret;
    if (uncompress)
        ret = qUncompress(QByteArray::fromBase64(data.toUtf8()));
    else
        ret = QByteArray::fromBase64(data.toUtf8());
    return ret;
}

QString BaseStateAbstract::write(bool value)
{
    return value ? GuiConst::STATE_YES : GuiConst::STATE_NO;
}

bool BaseStateAbstract::readBool(QStringRef val)
{
    return val == GuiConst::STATE_YES;
}

QString BaseStateAbstract::write(QColor color)
{
    return colorToString(color);
}

QString BaseStateAbstract::colorToString(const QColor &color)
{
    return QString::number(color.rgb());
}

QColor BaseStateAbstract::readColor(QStringRef val)
{
    return stringToColor(val);
}

QColor BaseStateAbstract::stringToColor(QStringRef val)
{
    return stringToColor(val.toString());
}

QColor BaseStateAbstract::stringToColor(QString val)
{
    bool ok = false;
    QColor ret;
    uint rgb = val.toUInt(&ok);

    if (!ok) {
        qCritical() << tr("[BaseStateAbstract::stringToColor] Invalid color");
        return ret;
    }

    return QColor(rgb);
}

QString BaseStateAbstract::write(int val)
{
    return QString::number(val);
}

int BaseStateAbstract::readInt(QStringRef val, bool *ok)
{
    return val.toString().toInt(ok);
}

QString BaseStateAbstract::write(quint64 val)
{
    return QString::number(val);
}

quint64 BaseStateAbstract::readUInt64(QStringRef val, bool *ok)
{
    return (quint64)val.toString().toULongLong(ok);
}

QString BaseStateAbstract::write(QBitArray barray)
{
    QString ret;
    for (int i = 0; i < barray.size(); i++) {
        if (barray.at(i))
            ret.append(B_ONE);
        else
            ret.append(B_ZERO);
    }

    return ret;
}

QBitArray BaseStateAbstract::readBitArray(QStringRef val)
{
    QBitArray ret(val.size(), false);
    for (int i = 0; i < val.size(); i++) {
        if (val.at(i) == B_ONE) {
            ret.setBit(i);
        } // already set to false by default, nothing to change
    }

    return ret;
}

QString BaseStateAbstract::write(QString val, bool compress)
{
    QString ret;
    QByteArray data = val.toUtf8();

    if (compress)
        data = qCompress(data,9).toBase64().prepend(COMPRESSED_MARKER); // prepending # to signify the string is compressed
    else
        data = data.toBase64();

    ret = QString::fromUtf8(data.constData(), data.size());
    return ret;
}

QString BaseStateAbstract::readString(QStringRef val)
{
    QString ret;

    QByteArray data = val.toString().toUtf8();
    if (data.isEmpty())
        return ret;

    bool compressed = false;

    if (data.at(0) == COMPRESSED_MARKER) {
        compressed = true;
        data = data.mid(1);
    }

    data = QByteArray::fromBase64(data);

    if (compressed)
        data = qUncompress(data);

    ret = QString::fromUtf8(data.constData(), data.size());
    return ret;
}

QXmlStreamReader *BaseStateAbstract::getReader() const
{
    return reader;
}

void BaseStateAbstract::setReader(QXmlStreamReader *value)
{
    reader = value;
}

QXmlStreamWriter *BaseStateAbstract::getWriter() const
{
    return writer;
}

void BaseStateAbstract::setWriter(QXmlStreamWriter *value)
{
    writer = value;
}


void BaseStateAbstract::run()
{
    qWarning() << "Default BaseStateAbstract::run() T_T";
}

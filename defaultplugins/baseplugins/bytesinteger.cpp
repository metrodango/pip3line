/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "bytesinteger.h"
#include "confgui/byteintegerwidget.h"
#include <QDebug>

const QString BytesInteger::id = "Byte(s) to Integer";
const QString BytesInteger::PROP_ENDIAN = "Endianness";
const QString BytesInteger::PROP_INTEGERSIZE = "IntegerSize";
const QString BytesInteger::PROP_SIGNEDINTEGER = "SignedInteger";

BytesInteger::BytesInteger()
{
    integerSize  = I32bits;
    signedInteger = true;
    littleendian = true;
}

BytesInteger::~BytesInteger()
{
}

QString BytesInteger::name() const
{
    return id;
}

QString BytesInteger::description() const
{
    return tr("convert an array of bytes to an Integer");
}

void BytesInteger::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    if (input.isEmpty())
        return;

    bool reverseByteOrdering = false;

#ifdef Q_LITTLE_ENDIAN
    bool currentSystemLittleEndian = true;
#else
    bool currentSystemLittleEndian = false;
#endif

    if (currentSystemLittleEndian != littleendian) {
        reverseByteOrdering = true;
    }

    QByteArray temp = input;
    if (wayValue == INBOUND) {

        if (temp.size() > integerSize) {
            temp = temp.mid(0,integerSize);
            emit warning(tr("Input size too big for the selected Integer size. the last bytes have been ignored"),id);
        }

        if (temp.size() < integerSize) {
            if (reverseByteOrdering)
                temp.prepend(QByteArray(integerSize - temp.size(),'\x00'));
            else
                temp.append(QByteArray(integerSize - temp.size(),'\x00'));
            emit error(tr("Input size too small for the selected Integer size. Padded with zeros"),id);
        }

        if (reverseByteOrdering) {
            QByteArray temp2;
            for (int i = temp.size() - 1; i >= 0; i--) {
                temp2.append(temp.at(i));
            }
            temp = temp2;
        }



        if (signedInteger) {
            switch (integerSize) {
            case I8bits: {
                    qint8 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            case I16bits: {
                    qint16 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            case I32bits: {
                    qint32 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            case I64bits: {
                    qint64 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            }

        } else {
            switch (integerSize) {
            case I8bits: {
                    quint8 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            case I16bits: {
                    quint16 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            case I32bits: {
                    quint32 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            case I64bits: {
                    quint64 val = 0;
                    memcpy(&val, temp.data(), integerSize);
                    output.append(QByteArray::number(val));
                    break;
            }
            }
        }
    } else {
        bool ok;
        char data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        if (signedInteger) {
            switch (integerSize) {
            case I8bits: {
                    qint16 val16 = input.toShort(&ok);
                    if (ok && val16 > -129 && val16 < 128) {
                        qint8 val8 = (qint8)val16;
                        memcpy(data, &val8, I8bits);
                    } else {
                        emit error(tr("Invalid 8bits signed Integer"),id);
                    }

                    break;
            }
            case I16bits: {
                    qint16 val16 = input.toShort(&ok);
                    if (ok) {
                        memcpy(data, &val16, I16bits);
                    } else {
                        emit error(tr("Invalid 16bits signed Integer"),id);
                    }
                    break;
            }
            case I32bits: {
                    qint32 val32 = input.toInt(&ok);
                    if (ok) {
                        memcpy(data, &val32, I32bits);
                    } else {
                        emit error(tr("Invalid 32bits signed Integer"),id);
                    }
                    break;
            }
            case I64bits: {
                    qint64 val64 = input.toInt(&ok);
                    if (ok) {
                        memcpy(data, &val64, I64bits);
                    } else {
                        emit error(tr("Invalid 64bits signed Integer"),id);
                    }
                    break;
            }
            }
        } else {
                switch (integerSize) {
                case I8bits: {
                        quint16 val16 = input.toShort(&ok);
                        if (ok && val16 < 256) {
                            quint8 val8 = (qint8)val16;
                            memcpy(data, &val8, I8bits);
                        } else {
                            emit error(tr("Invalid 8bits unsigned Integer"),id);
                        }
                }
                case I16bits: {
                        quint16 val16 = input.toShort(&ok);
                        if (ok) {
                            memcpy(data, &val16, I16bits);
                        } else {
                            emit error(tr("Invalid 16bits unsigned Integer"),id);
                        }
                        break;
                }
                case I32bits: {
                        quint32 val32 = input.toInt(&ok);
                        if (ok) {
                            memcpy(data, &val32, I32bits);
                        } else {
                            emit error(tr("Invalid 32bits unsigned Integer"),id);
                        }
                        break;
                }
                case I64bits: {
                        quint64 val64 = input.toInt(&ok);
                        if (ok) {
                            memcpy(data, &val64, I64bits);
                        } else {
                            emit error(tr("Invalid 64bits unsigned Integer"),id);
                        }
                        break;
                }
                }
            }
        if (reverseByteOrdering) {
            for (int i = integerSize - 1; i >= 0; i--) {
                output.append(data[i]);
            }
        } else {
            output = QByteArray(data, integerSize);
        }

    }
}

bool BytesInteger::isTwoWays()
{
    return true;
}

QHash<QString, QString> BytesInteger::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_ENDIAN,QString::number(littleendian ? 1 : 0));
    properties.insert(PROP_INTEGERSIZE,QString::number((int)integerSize));
    properties.insert(PROP_SIGNEDINTEGER,QString::number(signedInteger ? 1 : 0));

    return properties;
}

bool BytesInteger::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    bool ok = true;
    int val = 0;

    val = propertiesList.value(PROP_ENDIAN).toInt(&ok);

    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(PROP_ENDIAN),id);
    } else {
        setLittleEndian(val == 1);
    }

    val = propertiesList.value(PROP_INTEGERSIZE).toInt(&ok);
    if (!ok || (val != I8bits && val != I16bits && val != I32bits && val != I64bits)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(PROP_INTEGERSIZE),id);
    } else {
        setIntegerSize((IntSize)val);
    }

    val = propertiesList.value(PROP_SIGNEDINTEGER).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(PROP_SIGNEDINTEGER),id);
    } else {
        setSignedInteger(val == 1);
    }

    return res;
}

QString BytesInteger::inboundString() const
{
    return tr("Bytes to Int");
}

QString BytesInteger::outboundString() const
{
    return tr("Int to bytes");
}

QWidget *BytesInteger::requestGui(QWidget * parent )
{
    QWidget * widget = new(std::nothrow) ByteIntegerWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for ByteIntegerWidget X{");
    }
    return widget;
}

QString BytesInteger::help() const
{
    QString help;
    help.append("<p>Bytes to Integer translator</p><p> Try to convert a byte array into an integer of a given size</p><p>Parameters include sign, endianess and size</p>");
    return help;
}

bool BytesInteger::isLittleEndian() const
{
    return littleendian;
}

void BytesInteger::setLittleEndian(bool val)
{
    if (val != littleendian) {
        littleendian = val;
        emit confUpdated();
    }
}

bool BytesInteger::isSignedInteger() const
{
    return signedInteger;
}

void BytesInteger::setSignedInteger(bool val)
{
    if (val != signedInteger) {
        signedInteger = val;
        emit confUpdated();
    }
}

BytesInteger::IntSize BytesInteger::getIntegerSize() const
{
    return integerSize;
}

void BytesInteger::setIntegerSize(BytesInteger::IntSize val)
{
    if (val != integerSize) {
        integerSize = val;
        emit confUpdated();
    }
}

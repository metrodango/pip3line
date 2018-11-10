/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "bytestofloat.h"
#include "confgui/bytestofloatwdiget.h"

const QString BytesToFloat::id = "Bytes to float";

BytesToFloat::BytesToFloat()
{
    littleendian = true;
    floatSize = F32bits;
    precision = 6;
}

BytesToFloat::~BytesToFloat()
{
}

QString BytesToFloat::name() const
{
    return id;
}

QString BytesToFloat::description() const
{
    return "Cast bytes to a float or a double";
}

void BytesToFloat::transform(const QByteArray &input, QByteArray &output)
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
        if (temp.size() > floatSize) {
            temp = temp.mid(0,floatSize);
            emit warning(tr("Input size too big for the selected float size. the last bytes have been ignored"),id);
        }

        if (temp.size() < floatSize) {
            temp.append(QByteArray(floatSize - temp.size(),'\x00'));
            emit error(tr("Input size too small for the selected float size. Padded with zeros"),id);
        }

        if (reverseByteOrdering) {
            QByteArray temp2;
            for (int i = temp.size() - 1; i >= 0; i--) {
                temp2.append(temp.at(i));
            }
            temp = temp2;
        }

        switch (floatSize) {
            case F32bits: {
                    float val = 0;
                    memcpy(&val, temp.data(), floatSize);
                    output.append(QByteArray::number(static_cast<double>(val),'g',precision));
                    break;
            }
            case F64bits: {
                    double val = 0;
                    memcpy(&val, temp.data(), floatSize);
                    output.append(QByteArray::number(val,'g',precision));
                    break;
            }
        }
    } else {
        bool ok;
        char data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        switch (floatSize) {
            case F32bits: {
                    float val32 = input.toFloat(&ok);
                    if (ok) {
                        memcpy(data, &val32, F32bits);
                    } else {
                        emit error(tr("Invalid 32bits float"),id);
                    }
                    break;
            }
            case F64bits: {
                    double val64 = input.toDouble(&ok);
                    if (ok) {
                        memcpy(data, &val64, F64bits);
                    } else {
                        emit error(tr("Invalid 64bits double"),id);
                    }
                    break;
            }
        }

        if (reverseByteOrdering) {
            for (int i = floatSize - 1; i >= 0; i--) {
                output.append(data[i]);
            }
        } else {
            output = QByteArray(data, floatSize);
        }
    }
}


bool BytesToFloat::isTwoWays()
{
    return true;
}

QWidget *BytesToFloat::requestGui(QWidget * parent)
{
    QWidget *wid = new(std::nothrow) BytesToFloatWdiget(this, parent);
    if (wid == nullptr)
        qFatal("Cannot allocate memory for BytesToFloatWdiget X{");
    return wid;
}

QHash<QString, QString> BytesToFloat::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLLITTLEENDIAN,QString::number(littleendian ? 1 : 0));
    properties.insert(XMLFORMAT,QString::number(precision));
    properties.insert(XMLLENGTH,QString::number(floatSize));
    return properties;
}

bool BytesToFloat::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok = false;

    int val = propertiesList.value(XMLLITTLEENDIAN).toInt(&ok);

    if (!ok || (val != 1 && val != 0)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLLITTLEENDIAN),id);
    } else {
        setLittleEndian(val == 1);
    }

    val = propertiesList.value(XMLFORMAT).toInt(&ok);

    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLFORMAT),id);
    } else {
        setPrecision(val);
    }

    val = propertiesList.value(XMLLENGTH).toInt(&ok);

    if (!ok || (val != F32bits && val != F64bits)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLLENGTH),id);
    } else {
        setFloatSize(static_cast<FloatSize>(val));
    }

    return res;
}

bool BytesToFloat::isLittleEndian() const
{
    return littleendian;
}

void BytesToFloat::setLittleEndian(bool val)
{
    if (littleendian != val) {
        littleendian = val;
        emit confUpdated();
    }
}

BytesToFloat::FloatSize BytesToFloat::getFloatSize() const
{
    return floatSize;
}

void BytesToFloat::setFloatSize(FloatSize val)
{
    if (floatSize != val) {
        floatSize = val;
        emit confUpdated();
    }
}

int BytesToFloat::getPrecision() const
{
    return precision;
}

void BytesToFloat::setPrecision(int val)
{
    if (precision != val) {
        precision = val;
        emit confUpdated();
    }
}

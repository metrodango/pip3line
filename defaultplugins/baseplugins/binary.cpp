/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "binary.h"
#include "confgui/binarywidget.h"

const QString Binary::id = "Binary encoding";
const QByteArray Binary::BINARYCHAR = "01";

Binary::Binary()
{
    blockSize = MINBLOCKSIZE;
}

Binary::~Binary()
{
}

QString Binary::name() const {
    return id;
}

QString Binary::description() const {
    return tr("Transform bytes into a binary stream");
}

void Binary::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    QByteArray temp;


    if (wayValue == TransformAbstract::INBOUND) {
        for (int i = 0; i < input.size(); i++) {
            temp = QByteArray::number((uchar)(input.at(i)), 2);
            if (temp.size() < 8) {
                int count = temp.size();
                for (int j = 0; j < 8 - count ; j++)
                    temp.prepend("0");
            }
            output.append(temp);
        }
        if (blockSize != 0) {
            temp = output;
            output.clear();
            for (int i = 0; i < temp.size(); i += blockSize) {
                output.append(temp.mid(i, blockSize)).append(' ');
            }
            output.chop(1);
        }
    } else {
        bool ok = false;
        for (int i = 0; i < input.size(); i++) {
            if (BINARYCHAR.contains(input.at(i))) {
                temp.append(input.at(i));
            } else {
                ok = false;
            }
        }

        if (!ok)
            emit warning(tr("Invalid characters have been found, they have been ignored."),id);

        int length = (temp.size() / 8) * 8;
        int rest = temp.size() % 8;

        bool errors = false;
        for (int i = 0; i < length; i += 8) {
            char ch = char(temp.mid(i,8).toUShort(&ok,2));
            if (ok)
                output.append(ch);
            else {
                errors = true;
                output.append(temp.mid(i,8));
            }
        }
        if (rest != 0) {
            output.append(temp.mid(length,rest));
            emit warning(tr("Length is not a multiple of 8, ignoring the last bit(s)"),id);
        }
        if (errors)
            emit error(tr("Errors were encountered during the process, faulty blocks have been skipped."),id);
    }
}

bool Binary::isTwoWays() {
    return true;
}

QHash<QString, QString> Binary::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLBLOCKSIZE,QString::number(blockSize));

    return properties;
}

bool Binary::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    bool ok = true;
    int val = 0;

    val = propertiesList.value(XMLBLOCKSIZE).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLBLOCKSIZE),id);
    } else {
        res = setBlockSize(val) && res;
    }

    return res;
}

QWidget *Binary::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) BinaryWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for BinaryWidget X{");
    }
    return widget;
}

QString Binary::help() const
{
    QString help;
    help.append("<p>Binary decoder/encoder</p><p>Consider the input as a char array, and will produce a binary version of it (the unit is a char, i.e. 8 bits)</p><p>It is possible to group the bits together when encoding.</p>");
    return help;
}

int Binary::getBlockSize()
{
    return blockSize;
}

bool Binary::setBlockSize(int val)
{
    if (val < MINBLOCKSIZE || val > MAXBLOCKSIZE) {
        emit error(tr("Blocksize value out of bound [%1-%2]").arg(MINBLOCKSIZE).arg(MAXBLOCKSIZE),id);
        return false;
    }
    if (blockSize != val) {
        blockSize = val;
        emit confUpdated();
    }

    return true;
}


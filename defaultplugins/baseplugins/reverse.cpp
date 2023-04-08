/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "reverse.h"
#include "confgui/reversewidget.h"

const QString Reverse::id = "Reverse";

Reverse::Reverse()
{
    blockSize = 8;
    noBlock = false;
}

Reverse::~Reverse()
{
}

QString Reverse::name() const {
    return id;
}

QString Reverse::description() const {
    return tr("Reverse the input byte order");
}

bool Reverse::isTwoWays() {
    return false;
}

QHash<QString, QString> Reverse::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLNOBLOCK,QString::number(noBlock ? 1 : 0));
    properties.insert(XMLBLOCKSIZE,QString::number(blockSize));

    return properties;
}

bool Reverse::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLBLOCKSIZE).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLBLOCKSIZE),id);
    } else {
        res = setBlocksize(val) && res;
    }

    val = propertiesList.value(XMLNOBLOCK).toInt(&ok);
    if (!ok || (val !=0 && val !=1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLNOBLOCK),id);
    } else {
        setNoBlock(val == 1);
    }

    return res;
}

QWidget *Reverse::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) ReverseWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for ReverseWidget X{");
    }
    return widget;
}

QString Reverse::help() const
{
    QString help;
    help.append("<p>Reverse the input byte order</p><p>It is possible to specify the block size</p>");
    return help;
}

int Reverse::getBlocksize()
{
    return blockSize;
}

bool Reverse::getNoBlock()
{
    return noBlock;
}

bool Reverse::setBlocksize(int val)
{

    if (val < MINBLOCKSIZE || val > MAXBLOCKSIZE) {
        emit error(tr("Blocksize value out of bound [%1-%2]").arg(MINBLOCKSIZE).arg(MAXBLOCKSIZE),id);
        return false;
    }

    blockSize = val;
    noBlock = false;
    emit confUpdated();

    return true;
}

void Reverse::setNoBlock(bool val)
{
    noBlock = val;
    emit confUpdated();
}

void Reverse::transform(const QByteArray &input, QByteArray &output) {
    output.clear();

    if (noBlock) {
        for (int i = input.size() - 1; i > -1; i--) {
            output.append(input[i]);
        }
    } else {
        int numBlock = input.size()/blockSize;
        int restBlock = input.size()%blockSize;

        for (int i = 0; i < numBlock; i++) {
            for (int j = blockSize - 1; j > -1; j--) {
                output.append(input[j + (blockSize*i)]);
            }
        }

        if ( restBlock != 0) {
            for (int k = 0; k < blockSize - restBlock; k++) {
                output.append('\x00');
            }
            for (int i = input.size() - 1; i > (input.size() - 1) - restBlock; i--){
                output.append(input[i]);
            }
        }
    }
}

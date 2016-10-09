/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "padding.h"
#include "confgui/paddingwidget.h"
#include <QTime>
#include <QDebug>

const QString Padding::id = "Padding";

Padding::Padding()
{
    padChar = '\x01';
    choosenVariant = ZERO;
    blockSize = 8;
}

Padding::~Padding()
{
}

QString Padding::name() const {
    return id;
}

QString Padding::description() const {
    return tr("Add some padding to the byte stream to fit a defined block size");
}

bool Padding::isTwoWays() {
    return false;
}

QString Padding::help() const
{
    QString help;
    help.append("<p>Add some padding to the byte stream to fit a defined block size</p><p>Available Padding variants:<ul><li>Zero padding = just pad with zeroes</li><li>ANSI padding = pad with zeroes, apart from the last padding byte, which is the length of the padding.</li><li>ISO padding = pad with random bytes</li><li>PKCS7 padding = pad with the padding length value</li><li>custom = pad with a user defined character</li></ul></p>");
    return help;
}

QHash<QString, QString> Padding::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLVARIANT,QString::number((int)choosenVariant));
    properties.insert(XMLPADDINGCHAR,saveChar(padChar));
    properties.insert(XMLBLOCKSIZE,QString::number((int)blockSize));

    return properties;
}

bool Padding::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLBLOCKSIZE).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLBLOCKSIZE),id);
    } else {
        res = setBlockSize(val) && res;
    }

    val = propertiesList.value(XMLVARIANT).toInt(&ok);
    if (!ok || (val != ZERO && val != PKCS7 && val != ANSI && val != ISO && val != CUSTOM)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLVARIANT),id);
    } else {
        setVariant((PaddingVariant)val);
    }

    QString tmp = propertiesList.value(XMLPADDINGCHAR);
    char tmpChar = '\x00';
    if (!loadChar(tmp,&tmpChar)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLPADDINGCHAR),id);
    } else {
        setPadChar(tmpChar);
    }

    return res;
}

QWidget *Padding::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) PaddingWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for PaddingWidget X{");
    }
    return widget;
}

char Padding::getPadChar()
{
    return padChar;
}

Padding::PaddingVariant Padding::getVariant()
{
    return choosenVariant;
}

int Padding::getBlocksize()
{
    return blockSize;
}

void Padding::setPadChar(char val)
{
    padChar = val;
    emit confUpdated();
}

void Padding::setVariant(Padding::PaddingVariant val)
{
    if (choosenVariant != val) {
        choosenVariant = val;
        emit confUpdated();
        qDebug() << "setting " << choosenVariant;
    }
}

bool Padding::setBlockSize(int val)
{
    if (val < MINBLOCKSIZE || val > MAXBLOCKSIZE) {
        emit error(tr("BlockSize is out of the valid range [%1-%2]").arg(MINBLOCKSIZE).arg(MAXBLOCKSIZE),id);
        return false;
    }
    blockSize = val;
    emit confUpdated();
    return true;
}


void Padding::transform(const QByteArray &input, QByteArray &output) {
    output = input;
    int paddingLength = blockSize - output.size() % blockSize;
    if (paddingLength < 1)
        return;

            qDebug() << "transform " << choosenVariant;
    switch (choosenVariant) {
        case ZERO:
            for (int i = 0; i < paddingLength; i++)
                output.append('\x00');
            break;
        case ANSI:
            for (int i = 0; i < paddingLength - 1; i++)
                output.append('\x00');
            output.append((char)paddingLength);
            break;
        case ISO:
            qsrand(QTime::currentTime().msec()); // non cryptographic quality, boo
            for (int i = 0; i < paddingLength; i++)
                output.append((char)(qrand()) % 255);
            break;
        case PKCS7:
            for (int i = 0; i < paddingLength; i++)
                output.append((char)paddingLength);
            break;
        case CUSTOM:
            for (int i = 0; i < paddingLength; i++)
                output.append(padChar);
            break;
    }
}

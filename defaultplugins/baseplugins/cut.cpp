/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "cut.h"
#include "confgui/cutwidget.h"

const QString Cut::id = "Cut";

Cut::Cut()
{
    from = 0;
    length = 1;
    everything = false;
    classicCut = true;
    lineByLine = false;
}

Cut::~Cut()
{
}

QString Cut::name() const
{
    return id;
}

QString Cut::description() const
{
    return tr("Cut the input");
}

void Cut::transform(const QByteArray &input, QByteArray &output)
{
    QList<QByteArray> list;
    QByteArray temp;
    output.clear();
    if (lineByLine) {
        list = input.split('\n');
    } else {
        list.append(input);
    }

    for (int i = 0; i < list.size(); i++) {
        if (classicCut)
            output.append(list.at(i).mid(from, (everything ? -1 : length)));
        else {
            temp = list.at(i);
            temp.chop(length);
            output.append(temp);
        }
        output.append('\n');
    }
    output.chop(1);
}

bool Cut::isTwoWays()
{
    return false;
}

QHash<QString, QString> Cut::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLFROM,QString::number(from));
    properties.insert(XMLLENGTH,QString::number(length));
    properties.insert(XMLEVERYTHING,QString::number(everything ? 1 : 0));
    properties.insert(XMLTYPE, QString::number(classicCut));
    properties.insert(XMLPROCESSLINEBYLINE, QString::number(lineByLine));
    return properties;
}

bool Cut::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLFROM,QString("0")).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLFROM),id);
    } else {
        res = setFromPos(val) && res;
    }

    val = propertiesList.value(XMLLENGTH,QString("1")).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLLENGTH),id);
    } else {
        res = setLength(val) && res;
    }

    val = propertiesList.value(XMLEVERYTHING,QString("0")).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLEVERYTHING),id);
    } else {
        setCutEverything(val == 1);
    }

    val = propertiesList.value(XMLTYPE,QString("1")).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLTYPE),id);
    } else {
        setClassicCut(val == 1);
    }

    val = propertiesList.value(XMLPROCESSLINEBYLINE,QString("0")).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLPROCESSLINEBYLINE),id);
    } else {
        setLineByLine(val == 1);
    }

    return res;
}

QWidget *Cut::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) CutWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for CutWidget X{");
    }
    return widget;
}

QString Cut::help() const
{
    QString help;
    help.append("<p>This transformation will cut out part of the input</p><p>Nothing fancy here, this is just a convenience transformation.</p>");
    return help;
}

int Cut::getFromPos()
{
    return from;
}

int Cut::getLength()
{
    return length;
}

bool Cut::doCutEverything()
{
    return everything;
}

bool Cut::setFromPos(int val)
{
    if (val < 0) {
        emit error(tr("Invalid starting position: %1").arg(val),id);
        return false;
    }
    if (from != val) {
        from = val;
        emit confUpdated();
    }
    return true;
}

bool Cut::setLength(int val)
{
    if (val < 1) {
        emit error(tr("Invalid length: %1").arg(val),id);
        return false;
    }
    if (length != val) {
        length = val;
        everything = false;
        emit confUpdated();
    }
    return true;
}

void Cut::setCutEverything(bool val)
{
    if (everything != val) {
        everything = val;
        emit confUpdated();
    }
}
bool Cut::isClassicCut() const
{
    return classicCut;
}

void Cut::setClassicCut(bool value)
{
    if (classicCut != value) {
        classicCut = value;
        emit confUpdated();
    }
}
bool Cut::isLineByLine() const
{
    return lineByLine;
}

void Cut::setLineByLine(bool value)
{
    if (lineByLine != value) {
        lineByLine = value;
        emit confUpdated();
    }
}



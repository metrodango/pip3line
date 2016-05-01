/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "split.h"
#include "confgui/splitwidget.h"
#include <QTextStream>

const QString Split::id = "Split";

Split::Split()
{
    separator = ':';
    group = 0;
    allGroup = false;
    trimBlank = true;
    processLineByLine = true;
}

Split::~Split()
{
}

QString Split::name() const
{
    return id;
}

QString Split::description() const
{
    return tr("Split the input");
}

void Split::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    QList<QByteArray> lines;
    QByteArray temp;
    if (processLineByLine) {
        lines = input.split('\n');
    } else
        lines.append(input);

    for (int j = 0; j < lines.size(); j++) {
        QList<QByteArray> list = lines.at(j).split(separator);
        if (list.size() > 1) {
            if (allGroup) {
                for (int i = 0; i < list.size() - 1 ; i++) {
                    if (!list.at(i).isEmpty()) {
                        temp = list.at(i);
                        appendingToOutput(output,temp);
                    }
                }
                temp = list.at(list.size() - 1);
                appendingToOutput(output,temp);
            } else {
                if (group > list.size() - 1) {
                    emit error(tr("Only %1 group(s) found, cannot return group number %2").arg(list.size()).arg(group),id);
                } else {
                    temp = list.at(group);
                    appendingToOutput(output,temp);
                }
            }
        } else if (list.size() == 1){
            temp = list.at(0);
            appendingToOutput(output,temp);
        }
    }
    output.chop(1);
}

bool Split::isTwoWays()
{
    return false;
}

QHash<QString, QString> Split::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLSEPARATOR,saveChar(separator));
    properties.insert(XMLGROUP,QString::number((int)group));
    properties.insert(XMLEVERYTHING,QString::number(allGroup ? 1 : 0));
    properties.insert(XMLPROCESSLINEBYLINE,QString::number(processLineByLine ? 1 : 0));
    properties.insert(XMLCLEAN,QString::number(trimBlank ? 1 : 0));

    return properties;
}

bool Split::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    QString tmp = propertiesList.value(XMLSEPARATOR);
    char tmpChar;
    if (!loadChar(tmp,&tmpChar)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLSEPARATOR),id);
    } else {
        setSeparator(tmpChar);
    }

    int val = propertiesList.value(XMLGROUP).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLGROUP),id);
    } else {
        res = setSelectedGroup(val) && res;
    }

    val = propertiesList.value(XMLEVERYTHING).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLEVERYTHING),id);
    } else {
        setTakeAllGroup(val == 1);
    }

    val = propertiesList.value(XMLPROCESSLINEBYLINE).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLPROCESSLINEBYLINE),id);
    } else {
        setProcessLineByLine(val == 1);
    }

    val = propertiesList.value(XMLCLEAN).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLCLEAN),id);
    } else {
        setTrimBlank(val == 1);
    }

    return res;
}

QWidget *Split::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) SplitWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for SplitWidget X{");
    }
    return widget;
}

QString Split::help() const
{
    QString help;
    help.append("<p>Split the input (UTF-8)</p><p>Convience function, nothing fancy.</p>");
    return help;
}

char Split::getSeparator() const
{
    return separator;
}

int Split::getSelectedGroup() const
{
    return group;
}

bool Split::doWeTakeAllGroup() const
{
    return allGroup;
}

bool Split::isTrimmingBlank() const
{
    return trimBlank;
}

bool Split::isProcessingLineByLine() const
{
    return processLineByLine;
}

void Split::setSeparator(char val)
{
    separator = val;
    emit confUpdated();
}

bool Split::setSelectedGroup(int val)
{
    if (val < 0 || val > MAXGROUPVALUE) {
        emit error(tr("Invalid selected group value %1").arg(val),id);
        return false;
    }
    if (group != val) {
        group = val;
        allGroup = false;
        emit confUpdated();
    }
    return true;
}

void Split::setTakeAllGroup(bool val)
{
    if (allGroup != val) {
        allGroup = val;
        emit confUpdated();
    }
}

void Split::setTrimBlank(bool val)
{
    if (trimBlank != val) {
        trimBlank = val;
        emit confUpdated();
    }
}

void Split::setProcessLineByLine(bool val)
{
    if (processLineByLine != val) {
        processLineByLine = val;
        emit confUpdated();
    }
}

void Split::appendingToOutput(QByteArray &out, QByteArray &val)
{
    if (trimBlank)
        val = val.trimmed();

    if (!val.isEmpty()) {
        out.append(val);
        out.append('\n');
    }
}



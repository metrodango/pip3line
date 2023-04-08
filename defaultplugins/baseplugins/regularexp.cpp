/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "regularexp.h"
#include "confgui/regularexpwidget.h"
#include <QTextStream>

const QString RegularExp::id = "RegExp";

RegularExp::RegularExp()
{
    processLineByLine = false;
    actionType = EXTRACT;
    useGreedyQuantifier = true;
    caseInsensitive = false;
    selectedGroup = 0;
    allGroups = false;
}

RegularExp::~RegularExp()
{
}

QString RegularExp::name() const
{
    return id;
}

QString RegularExp::description() const
{
    return tr("Search or replace using a Regular Expression on the input");
}

void RegularExp::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();

    Qt::CaseSensitivity cs = caseInsensitive ? Qt::CaseInsensitive : Qt::CaseSensitive;

    QRegExp::PatternSyntax syntax = useGreedyQuantifier ? QRegExp::RegExp2 : QRegExp::RegExp;
    QRegExp rx(expression, cs, syntax);

    if (!rx.isValid() || rx.isEmpty()) {
        emit error(tr("Invalid Regex"),id);
    } else {
        QList<QByteArray> inputList;
        if (processLineByLine) {
            inputList = input.split('\n');
        } else {
            inputList.append(input);
        }
        for (int k = 0; k < inputList.size(); k++) {
            QString data = QString::fromUtf8(inputList.at(k));
            if (actionType == EXTRACT) {
                int pos = 0;
                QStringList list;

                while ((pos = rx.indexIn(data, pos)) != -1 && pos < data.size()) {
                    QString temp = rx.cap(1);
                    if (temp.isEmpty()) {
                        temp = rx.cap(0);
                    }
                    if (!temp.isEmpty()) {
                        list << temp;
                        pos += temp.size();
                    } else {
                        pos++;
                    }
                }

                if (allGroups) {
                    QStringList::iterator it = list.begin();
                    while (it != list.end()) {
                        if (!(*it).isEmpty()) {
                            output.append((*it).toUtf8()).append("\n");
                            ++it;
                        }
                     }

                } else if (selectedGroup > list.size() -1) {
                    emit warning(tr("Selected group index not found"),id);
                } else {
                    QString val = list.at(selectedGroup);
                    if (!val.isEmpty())
                        output.append(val.toUtf8()).append('\n');
                }

            } else {
                if (!data.isEmpty()) {
                    data.replace(rx,replacement);
                    output.append(data.toUtf8()).append('\n');
                }
            }
        }
        if (!output.isEmpty() && output.at(output.size() - 1) == '\n')
            output.chop(1);
    }
}

bool RegularExp::isTwoWays()
{
    return false;
}

QHash<QString, QString> RegularExp::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLPROCESSLINEBYLINE,QString::number(processLineByLine ? 1 : 0));
    properties.insert(XMLACTIONTYPE,QString::number(static_cast<int>(actionType)));
    properties.insert(XMLGREEDYQUANT,QString::number(useGreedyQuantifier ? 1 : 0));
    properties.insert(XMLREGEXP,QString(expression.toUtf8().toBase64()));
    properties.insert(XMLCASEINSENSITIVE,QString::number(caseInsensitive ? 1 : 0));

    if (actionType == EXTRACT) {
        properties.insert(XMLALLGROUPS, QString::number(allGroups ? 1 : 0));
        properties.insert(XMLGROUP, QString::number(static_cast<int>(selectedGroup)));
    } else {
        properties.insert(XMLREPLACEWITH, QString(replacement.toUtf8().toBase64()));
    }

    return properties;
}

bool RegularExp::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLPROCESSLINEBYLINE).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLPROCESSLINEBYLINE),id);
    } else {
        setProcessLineByLine(val == 1);
    }

    val = propertiesList.value(XMLACTIONTYPE).toInt(&ok);
    if (!ok || (val != EXTRACT && val != REPLACE)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLACTIONTYPE),id);
    } else {
        setActionType(static_cast<Actions>(val));
    }

    val = propertiesList.value(XMLGREEDYQUANT).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLGREEDYQUANT),id);
    } else {
        setUsingGreedyQuantifier(val == 1);
    }

    setExpression(QString(QByteArray::fromBase64(propertiesList.value(XMLREGEXP).toUtf8())));
    val = propertiesList.value(XMLCASEINSENSITIVE).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLCASEINSENSITIVE),id);
    } else {
        setCaseInsensitive(val ==  1);
    }

    if (actionType == EXTRACT) {
        val = propertiesList.value(XMLALLGROUPS).toInt(&ok);
        if (!ok || (val != 0 && val != 1)) {
            res = false;
            emit error(tr("Invalid value for %1").arg(XMLALLGROUPS),id);
        } else {
            setAllGroups(val == 1);
        }

        if (!allGroups) {
            val = propertiesList.value(XMLGROUP).toInt(&ok);
            if (!ok) {
                res = false;
                emit error(tr("Invalid value for %1").arg(XMLGROUP),id);
            } else {
                res = setSelectedGroup(val) && res;
            }
        }
    } else {
        replacement = QString(QByteArray::fromBase64(propertiesList.value(XMLREPLACEWITH).toUtf8()));
    }

    return res;
}

QWidget *RegularExp::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) RegularExpWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for RegularExpWidget X{");
    }
    return widget;
}

QString RegularExp::help() const
{
    QString help;
    help.append("<p>Apply a regular expression to the input (UTF8)</p><p>Two main modes:");
    help.append("<ul><li>Search & Extract = to extract strings</li><li>Search & Replace = to replace string portions</li></ul>");
    help.append("</p><p>There are lots of fine grain configurations options available</p>");
    return help;
}

bool RegularExp::doWeProcessLineByLine()
{
    return processLineByLine;
}

RegularExp::Actions RegularExp::getActionType()
{
    return actionType;
}

bool RegularExp::isUsingGreedyQuantifier()
{
    return useGreedyQuantifier;
}

QString RegularExp::getRegularExpression()
{
    return expression;
}

bool RegularExp::isCaseInsensitive()
{
    return caseInsensitive;
}

int RegularExp::getSelectedGroupIndex()
{
    return selectedGroup;
}

bool RegularExp::isTakingAllGroup()
{
    return allGroups;
}

QString RegularExp::getReplacementString()
{
    return replacement;
}

void RegularExp::setProcessLineByLine(bool val)
{
    processLineByLine = val;
    emit confUpdated();
}

void RegularExp::setActionType(RegularExp::Actions val)
{
    actionType = val;
    emit confUpdated();
}

void RegularExp::setUsingGreedyQuantifier(bool val)
{
    useGreedyQuantifier = val;
    emit confUpdated();
}

void RegularExp::setExpression(QString exp)
{
    expression = exp;
    emit confUpdated();
}

void RegularExp::setCaseInsensitive(bool val)
{
    caseInsensitive = val;
    emit confUpdated();
}

bool RegularExp::setSelectedGroup(int val)
{
    if (val < 0) {
        emit error(tr("Invalid group index value: %1").arg(val),id);
        return false;
    }
    selectedGroup = val;
    emit confUpdated();
    return true;
}

void RegularExp::setAllGroups(bool val)
{
    allGroups = val;
    emit confUpdated();
}

void RegularExp::setReplacementString(QString val)
{
    replacement = val;
    emit confUpdated();
}

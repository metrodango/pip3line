/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "fixprotocol.h"
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QTextStream>
#include <QFile>

const QString FixProtocol::id = "Fix Protocol parser";

FixProtocol::FixProtocol()
{
    QTextStream cout(stdout);
    QFile referenceFile;
    referenceFile.setFileName(":/definitions/FIX44.xml");
    if (!referenceFile.open(QIODevice::ReadOnly)) {
        cout << tr("Error while opening the reference file:\n %1").arg(referenceFile.errorString()) << QTENDL;
    }
    query.setFocus(&referenceFile);
    referenceFile.close();
}

FixProtocol::~FixProtocol()
{
}

QString FixProtocol::name() const
{
    return id;
}

QString FixProtocol::description() const
{
    return tr("Parse/unpack a FIX 4.4 stream");
}

void FixProtocol::transform(const QByteArray &input, QByteArray &output)
{
    QList<QByteArray> fields = input.split(0x01);
    for (int i = 0; i < fields.size(); i++) {
        output.append(translateField(fields.at(i)));
        if (!output.isEmpty())
            output.append('\n');
    }
}

bool FixProtocol::isTwoWays()
{
    return false;
}

QString FixProtocol::help() const
{
    QString help;
    help.append("<p>Financial Information eXchange (FIX) protocol parser</p><p>This transformation will parse a raw (binary) FIX transaction, also resolving fields' name and value when possible</p><p>Currently only supporting FIX 4.4.<br>Test data are needed to enable support for other versions.</p>");
    return help;
}

QByteArray FixProtocol::translateField(QByteArray val)
{
    QByteArray ret;
    if (val.isEmpty())
        return ret;

    bool ok = false;

    int fieldNum = 0;
    QList<QByteArray> list = val.split('=');
    if (list.size() != 2) {
        emit error(tr("Invalid pair:").append(val),id);
        return ret;
    }

    fieldNum = list.at(0).toInt(&ok);

    if (!ok) {
        emit error(tr("Error while parsing the field number value: %1").arg(QString(list.at(0))),id);
        return ret;
    }

    query.bindVariable("field",QVariant(fieldNum));
    query.bindVariable("val",QVariant(QString(list.at(1))));
    query.setQuery("/fix/fields/field[@number=$field]/@name/string()");

    if (!query.isValid()) {
        emit error(tr("Invalid XPATH query"),id);
        return ret;
    }

    QStringList qout;
    if (!query.evaluateTo(&qout)) {
        emit error(tr("Error while evaluating xmlQuery with %1").arg(QString(val)),id);
    }


    if (qout.size() < 1) {
        emit error(tr("Unknown field number:").append(fieldNum),id);
        ret.append(val);
        return ret;
    } else if (qout.size() > 1) {
        emit warning(tr("Strange, found multiple value for field number %1 (taking the first one)").arg(fieldNum),id);
    }
    ret.append(qout.at(0).toUtf8()).append('(').append(list.at(0)).append(')').append(" = ");

    query.setQuery("/fix/fields/field[@number=$field]/value[@enum=$val]/@description/string()");
    if (!query.isValid()) {
        emit error(tr("Invalid XPATH query"),id);
        return ret.append(list.at(1));
    }

    qout.clear();
    if (!query.evaluateTo(&qout)) {
        emit error(tr("Error while evaluating xmlQuery (2) with %1").arg(QString(val)),id);
    }

    if (qout.size() < 1) {
        ret.append(list.at(1));
        return ret;
    } else if (qout.size() > 1) {
        emit warning(tr("Strange, found multiple value for field value %1 (taking the first one)").arg(QString(list.at(1))),id);
    }
    ret.append(qout.at(0).toUtf8()).append('(').append(list.at(1)).append(')');
    return ret;
}

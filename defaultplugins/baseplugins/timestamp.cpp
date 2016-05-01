/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "timestamp.h"
#include "confgui/timestampwidget.h"
#include <QDateTime>
#include <QDebug>

const QString TimeStamp::id = "Timestamp (Epoch)";
const QString TimeStamp::DEFAULT_DATE_FORMAT = "dd/MM/yyyy hh:mm:ss";
const QString TimeStamp::DEFAULT_DATE_FORMAT_MS = "dd/MM/yyyy hh:mm:ss.zzz";
const QString TimeStamp::PROP_DATEFORMAT = "DateFormat";

TimeStamp::TimeStamp()
{
    dateFormat = DEFAULT_DATE_FORMAT;
    tz = TZ_LOCAL;
}

TimeStamp::~TimeStamp()
{

}

QString TimeStamp::name() const
{
    return id;
}

QString TimeStamp::description() const
{
    return tr("Epoch timestamp translator");
}

void TimeStamp::transform(const QByteArray &input, QByteArray &output)
{
    QDateTime timestamp;
    if (wayValue == INBOUND) {
        bool ok;
        timestamp.setTimeSpec(getTimeSpec());
        uint val1 = input.toUInt(&ok);
        if (ok) {
            timestamp.setTime_t(val1);
            output = timestamp.toString(DEFAULT_DATE_FORMAT).toUtf8();
        } else {
            qint64 val2 = input.toLongLong(&ok);
            if (ok) {
                timestamp.setMSecsSinceEpoch(val2);
                output = timestamp.toString(DEFAULT_DATE_FORMAT_MS).toUtf8();
            } else {
                emit error(tr("could not parse the number"),id);
                return;
            }
        }
#if QT_VERSION >= 0x052000
        output.append(timestamp.timeZone().displayName(QTimeZone::GenericTime));
#else
        if (tz == TZ_UTC)
            output.append(" UTC");
        else
            output.append(" Local time");
#endif
    } else {
        QByteArray temp = input;
        do  {
            timestamp = QDateTime::fromString(QString::fromUtf8(temp,temp.size()), DEFAULT_DATE_FORMAT_MS);
            timestamp.setTimeSpec(getTimeSpec());
            temp.chop(1);
        } while (!timestamp.isValid() && temp.size() != 0);

        if (temp.size() == 0) {
            temp = input;
            do  {
                timestamp = QDateTime::fromString(QString::fromUtf8(temp,temp.size()), DEFAULT_DATE_FORMAT);
                timestamp.setTimeSpec(getTimeSpec());
                temp.chop(1);
            } while (!timestamp.isValid() && temp.size() != 0);
            if (temp.size() == 0) {
                emit error(tr("Cannot parse the input"),id);
                return;
            } else {
                output = QByteArray::number(timestamp.toTime_t());
            }
        } else {
            output = QByteArray::number(timestamp.toMSecsSinceEpoch());
        }
    }
}

bool TimeStamp::isTwoWays()
{
    return true;
}

QWidget *TimeStamp::requestGui(QWidget * parent)
{
    QWidget * widget = new(std::nothrow) TimestampWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for TimestampWidget X{");
    }
    return widget;
}

QString TimeStamp::help() const
{
    QString help;
    help.append("<p>Epoch timestamp translator</p><p>For Unix timestamp mostly, translate between integer and string representation.</p><p> Warning: There is no assumption on the timezone at all, it could be anything. Setting it to UTC by default</p>");
    return help;
}

QHash<QString, QString> TimeStamp::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_DATEFORMAT,dateFormat.toUtf8().toBase64());
    properties.insert(XMLTZ,QString::number(tz));
    return properties;
}

bool TimeStamp::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    setDateFormat(QString::fromUtf8(QByteArray::fromBase64(propertiesList.value(PROP_DATEFORMAT).toUtf8())));

    bool ok = false;
    int val = propertiesList.value(XMLTZ).toInt(&ok);
    if (!ok || (val != TZ_LOCAL && val != TZ_UTC)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLTZ),id);
    } else {
        setTZ((TimeStamp::TZ) val);
    }

    return res;
}

QString TimeStamp::inboundString() const
{
    return tr("Int to string");
}

QString TimeStamp::outboundString() const
{
    return tr("String to int");
}

void TimeStamp::setDateFormat(QString format)
{
    if (dateFormat != format) {
        dateFormat = format;
        emit confUpdated();
    }
}

QString TimeStamp::getDateFormat() const
{
    return dateFormat;
}
TimeStamp::TZ TimeStamp::getTZ() const
{
    return tz;
}

void TimeStamp::setTZ(const TimeStamp::TZ &value)
{
    if (value != tz) {
        tz = value;
        emit confUpdated();
    }
}

Qt::TimeSpec TimeStamp::getTimeSpec()
{
    if (tz == TZ_UTC) {
        return Qt::UTC;
    } else {
        return Qt::LocalTime;
    }
}





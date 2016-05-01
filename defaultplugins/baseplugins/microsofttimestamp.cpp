/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "microsofttimestamp.h"
#include "confgui/microsofttimestampwidget.h"
#include <QDebug>
#include <QDateTime>
#include <QDate>

const QString MicrosoftTimestamp::id = "Timestamp (Microsoft)";
const QString MicrosoftTimestamp::DEFAULT_DATE_FORMAT = "ddd d MMMM yyyy hh:mm:ss.zzz";
const QString MicrosoftTimestamp::PROP_DATEFORMAT = "DateFormat";
const QString MicrosoftTimestamp::PROP_ADD_OUT_NS = "AddedNs";

MicrosoftTimestamp::MicrosoftTimestamp()
{
    dateFormat = DEFAULT_DATE_FORMAT;
    outNS = 0;
    tz = TZ_LOCAL;
}

MicrosoftTimestamp::~MicrosoftTimestamp()
{
}

QString MicrosoftTimestamp::name() const
{
    return id;
}

QString MicrosoftTimestamp::description() const
{
    return tr("Translate Microsoft timestamp");
}

void MicrosoftTimestamp::transform(const QByteArray &input, QByteArray &output)
{
    if (input.isEmpty())
        return;

    output.clear();
    QDateTime timestamp;
    quint64 val2;
    if (wayValue == INBOUND) {
        bool ok;

        val2 = input.toULongLong(&ok);

        if (ok) {

            quint64 val1 = val2 / (10000);
            quint64 rest = val2 % 10000;
            timestamp.setTimeSpec(Qt::UTC);
            timestamp.setDate(QDate(1601,1,1));
            timestamp = timestamp.addMSecs(val1 > LONG_MAX ? LONG_MAX : (qint64) val1);
            if (tz == TZ_LOCAL)
                timestamp = timestamp.addMSecs(getTimeZoneOffset());
            output = timestamp.toString(dateFormat).toUtf8();
            output.append("ms ").append(QByteArray::number(rest)).append(" ns");
#if QT_VERSION >= 0x052000
            output.append(timestamp.timeZone().displayName(QTimeZone::GenericTime));
#else
            if (tz == TZ_UTC)
                output.append(" UTC");
            else
                output.append(" Local time");
#endif

        } else {
            emit error(tr("Invalid number"),id);
        }
    } else {
        QByteArray temp = input;
        timestamp.setTimeSpec(Qt::UTC);
        timestamp.setDate(QDate(1601,1,1));
        val2 = qAbs(timestamp.toMSecsSinceEpoch());
        do  {
            timestamp = QDateTime::fromString(QString::fromUtf8(temp,temp.size()), dateFormat);
            timestamp.setTimeSpec(getTimeSpec());
            temp.chop(1);
        } while (!timestamp.isValid() && temp.size() != 0);

        if (temp.size() == 0) {
            emit error(tr("Cannot parse the input"),id);
            return;
        }
        val2 = ((val2 + timestamp.toMSecsSinceEpoch()) * 10000) + outNS;

        output = QByteArray::number(val2);
    }
}

bool MicrosoftTimestamp::isTwoWays()
{
    return true;
}

QWidget *MicrosoftTimestamp::requestGui(QWidget * parent)
{
    QWidget * widget = new(std::nothrow) MicrosoftTimestampWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for MicrosoftTimestampWidget X{");
    }
    return widget;
}

QString MicrosoftTimestamp::help() const
{
    QString help;
    help.append("<p>Microsoft timestamp translator</p><p>For Microsoft Active Directory timestamps mostly, translate between uint64 and string representation.</p><p>Microsoft timestamps are usually UTC based, so setting to UTC.</p>");
    return help;
}

QHash<QString, QString> MicrosoftTimestamp::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_DATEFORMAT,dateFormat.toUtf8().toBase64());
    properties.insert(XMLTZ,QString::number(tz));
    properties.insert(PROP_ADD_OUT_NS,QString::number(outNS));
    return properties;
}

bool MicrosoftTimestamp::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    setDateFormat(QString::fromUtf8(QByteArray::fromBase64(propertiesList.value(PROP_DATEFORMAT).toUtf8())));

    bool ok = false;
    int val = propertiesList.value(XMLTZ).toInt(&ok);
    if (!ok || (val != TZ_LOCAL && val != TZ_UTC)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLTZ),id);
    } else {
        setTZ((MicrosoftTimestamp::TZ) val);
    }

    val = propertiesList.value(PROP_ADD_OUT_NS).toInt(&ok);
    if (!ok || (val < 0 || val > 9999)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(PROP_ADD_OUT_NS),id);
    } else {
        setOutNS(val);
    }

    return res;
}

QString MicrosoftTimestamp::inboundString() const
{
    return tr("Int to string");
}

QString MicrosoftTimestamp::outboundString() const
{
    return tr("String to int");
}

void MicrosoftTimestamp::setDateFormat(QString format)
{
    if (dateFormat != format) {
        dateFormat = format;
        emit confUpdated();
    }
}

QString MicrosoftTimestamp::getDateFormat() const
{
    return dateFormat;
}
MicrosoftTimestamp::TZ MicrosoftTimestamp::getTZ() const
{
    return tz;
}

void MicrosoftTimestamp::setTZ(const TZ &value)
{
    if (value != tz) {
        tz = value;
        emit confUpdated();
    }
}

Qt::TimeSpec MicrosoftTimestamp::getTimeSpec()
{
    if (tz == TZ_UTC) {
        return Qt::UTC;
    } else {
        return Qt::LocalTime;
    }
}
int MicrosoftTimestamp::getOutNS() const
{
    return outNS;
}

void MicrosoftTimestamp::setOutNS(int value)
{
    if (value < 0 || value > 9999) {
        emit error("Invalid nanosec value (it needs to be in the [0-9999] range)",id);
    } else if (value != outNS) {
        outNS = value;
        emit confUpdated();
    }

}

qint64 MicrosoftTimestamp::getTimeZoneOffset()
{
    QDateTime dt1 = QDateTime::currentDateTime();
    QDateTime dt2 = dt1.toUTC();
    dt1.setTimeSpec(Qt::UTC);
    return dt2.msecsTo(dt1);
}



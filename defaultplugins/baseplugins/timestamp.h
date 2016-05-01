/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <transformabstract.h>

class TimeStamp : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        static const QString DEFAULT_DATE_FORMAT;
        static const QString DEFAULT_DATE_FORMAT_MS;
        static const QString PROP_DATEFORMAT;
        enum TZ { TZ_UTC = 0, TZ_LOCAL = 1 };
        explicit TimeStamp();
        ~TimeStamp();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        QString help() const;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);

        QString inboundString() const;
        QString outboundString() const;

        void setDateFormat(QString format);
        QString getDateFormat() const;

        TZ getTZ() const;
        void setTZ(const TZ &value);

    private:
        Qt::TimeSpec getTimeSpec();
        QString dateFormat;
        TZ tz;

};

#endif // TIMESTAMP_H

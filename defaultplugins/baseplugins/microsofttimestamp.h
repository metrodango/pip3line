/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MICROSOFTTIMESTAMP_H
#define MICROSOFTTIMESTAMP_H

#include <transformabstract.h>

class MicrosoftTimestamp : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        static const QString DEFAULT_DATE_FORMAT;
        static const QString PROP_ADD_OUT_NS;
        enum TZ { TZ_UTC = 0, TZ_LOCAL = 1 };
        static const QString PROP_DATEFORMAT;
        explicit MicrosoftTimestamp();
        ~MicrosoftTimestamp() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QWidget * requestGui(QWidget * parent) override;
        QString help() const override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;

        QString inboundString() const override;
        QString outboundString() const override;

        void setDateFormat(QString format);
        QString getDateFormat() const;
        
        MicrosoftTimestamp::TZ getTZ() const;
        void setTZ(const TZ &value);

        int getOutNS() const;
        void setOutNS(int value);

        qint64 getTimeZoneOffset();
    private:
        Qt::TimeSpec getTimeSpec();
        QString dateFormat;
        TZ tz;
        int outNS;
        
};

#endif // MICROSOFTTIMESTAMP_H

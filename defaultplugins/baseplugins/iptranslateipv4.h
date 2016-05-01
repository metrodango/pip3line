/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef IPTRANSLATE_H
#define IPTRANSLATE_H

#include <transformabstract.h>

class IPTranslateIPv4 : public TransformAbstract
{
        Q_OBJECT
    public:
        enum BASE {BASE10 = 0, BASE16};
        explicit IPTranslateIPv4();
        ~IPTranslateIPv4() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget *requestGui(QWidget *parent);
        QString inboundString() const;
        QString outboundString() const;
        QString help() const;
        void setBase(BASE val);
        BASE getBase() const;
        void setLittleEndian(bool val = true);
        bool isLittleendian() const;
        void setUppercase(bool val = true);
        bool isFormatUpperCase() const;
    private:
        quint32 reverseBytes(quint32 val);
        int base;
        bool littleEndian;
        bool toUpper;
};

#endif // IPTRANSLATE_H

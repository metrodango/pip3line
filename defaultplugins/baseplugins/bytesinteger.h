/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESINTEGER_H
#define BYTESINTEGER_H

#include <transformabstract.h>

class BytesInteger : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        static const QString PROP_ENDIAN;
        static const QString PROP_INTEGERSIZE;
        static const QString PROP_SIGNEDINTEGER;
        enum IntSize { I8bits = 1, I16bits = 2, I32bits = 4, I64bits = 8 };
        explicit BytesInteger();
        ~BytesInteger();
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

        bool isLittleEndian() const;
        void setLittleEndian(bool val);
        bool isSignedInteger() const;
        void setSignedInteger(bool val);
        IntSize getIntegerSize() const;
        void setIntegerSize(IntSize val);

    private:
        bool littleendian;
        IntSize integerSize;
        bool signedInteger;
};

#endif // BYTESINTEGER_H

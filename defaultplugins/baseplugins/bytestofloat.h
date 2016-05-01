/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESTOFLOAT_H
#define BYTESTOFLOAT_H

#include <transformabstract.h>

class BytesToFloat : public TransformAbstract
{
    public:
        enum FloatSize { F32bits = 4, F64bits = 8 };
        explicit BytesToFloat();
        ~BytesToFloat();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);

        bool isLittleEndian() const;
        void setLittleEndian(bool val);

        FloatSize getFloatSize() const;
        void setFloatSize(FloatSize val);

        int getPrecision() const;
        void setPrecision(int val);
    private:
        bool littleendian;
        FloatSize floatSize;
        int precision;
};

#endif // BYTESTOFLOAT_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NUMBERTOCHAR_H
#define NUMBERTOCHAR_H

#include <transformabstract.h>

class NumberToChar : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit NumberToChar();
        ~NumberToChar();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
        QWidget * requestGui(QWidget * parent);
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QString inboundString() const;
        QString outboundString() const;

        char getSeparator() const;
        bool setSeparator(char c);

    private:
        static const char DEFAULT_SEPARATOR;
        char separator;
        bool signedShort;
};

#endif // NUMBERTOCHAR_H

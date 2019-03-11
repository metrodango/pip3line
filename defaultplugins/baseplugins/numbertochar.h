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
        ~NumberToChar() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
        QWidget * requestGui(QWidget * parent) override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QString inboundString() const override;
        QString outboundString() const override;

        char getSeparator() const;
        bool setSeparator(char c);

    private:
        static const char DEFAULT_SEPARATOR;
        char separator;
        bool signedShort;
};

#endif // NUMBERTOCHAR_H

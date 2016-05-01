/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASE64_H
#define BASE64_H

#include "transformabstract.h"

class Base64 : public TransformAbstract
{
    Q_OBJECT
    public:
        static const QByteArray BASE64CHAR;
        static const QStringList VARIATIONS;
        static const char DEFAULTPADDINGCHAR = '=';
        enum Variation {STANDARD = 0, SAFEURL = 1, DOTNET = 2, CUSTOM = 3};
        enum PaddingType {DEFAULTPADDING = 0, DOTNETPADDING = 1, NOPADDING = 2, CUSTOMPADDING = 3};
        explicit Base64();
        ~Base64();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input,QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        Variation getChoosenVariation();
        PaddingType getPaddingType();
        char getChar62();
        char getChar63();
        char getPaddingChar();
        void setChoosenVariation(Variation val);
        void setPaddingType(PaddingType val);
        bool setChar62(char val);
        bool setChar63(char val);
        bool setPaddingChar(char val);
    private:
        Variation choosenVariation;
        PaddingType paddingType;
        char char62;
        char char63;
        char paddingChar;
};

#endif // BASE64_H

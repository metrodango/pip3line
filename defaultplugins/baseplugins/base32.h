/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASE32_H
#define BASE32_H


#include "transformabstract.h"

class Base32 : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QStringList VARIATIONS;
        enum CharSetVariant {RFC4648 = 0, CROCKFORD = 1, BASE32HEX = 2};
        static const QByteArray CharTableRFC4648;
        static const QByteArray CharTableCrockford;
        static const QByteArray CharTableBase32Hex;
        explicit Base32();
        ~Base32() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;
// specific to class
        QByteArray getCharTable();
        bool isPaddingIncluded();
        char getPaddingChar();
        CharSetVariant getVariant();
        void setIncludePadding(bool val);
        bool setPaddingChar(char val);
        void setVariant(CharSetVariant val);

    private:
        bool includePadding;
        char paddingChar;
        CharSetVariant variant;
        static const int BlockSize = 5;
        static const int ByteSize = 8;
};

#endif // BASE32_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef PADDING_H
#define PADDING_H

#include "transformabstract.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif

class Padding : public TransformAbstract
{
    Q_OBJECT
    
    public:
        enum PaddingVariant {ZERO = 0, ANSI, ISO, PKCS7, CUSTOM};
        static const int MINBLOCKSIZE = 2;
        static const int MAXBLOCKSIZE = 1024;
        explicit Padding();
        ~Padding() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;

        char getPadChar();
        PaddingVariant getVariant();
        int getBlocksize();
        void setPadChar(char val);
        void setVariant(PaddingVariant val);
        bool setBlockSize(int val);
        QString help() const override;
    private:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        QRandomGenerator rand;
#endif
        char padChar;
        PaddingVariant choosenVariant;
        int blockSize;
};

#endif // PADDING_H

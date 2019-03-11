/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BINARY_H
#define BINARY_H


#include "transformabstract.h"

class Binary : public TransformAbstract
{
    Q_OBJECT
    
    public:
        static const QByteArray BINARYCHAR;
        static const int MINBLOCKSIZE = 0;
        static const int MAXBLOCKSIZE = 1024;
        explicit Binary();
        ~Binary() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        int getBlockSize();
        bool setBlockSize(int val);

    private:
        int blockSize;
};

#endif // BINARY_H

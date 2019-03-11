/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef REVERSE_H
#define REVERSE_H

#include "transformabstract.h"

class Reverse : public TransformAbstract
{
    Q_OBJECT
    
    public:
        static const int MINBLOCKSIZE = 2;
        static const int MAXBLOCKSIZE = 1024;
        explicit Reverse();
        ~Reverse() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget *requestGui(QWidget *parent) override;
        static const QString id;
        QString help() const override;

        int getBlocksize();
        bool getNoBlock();
        bool setBlocksize(int val);
        void setNoBlock(bool val);
    private:
        int blockSize;
        bool noBlock;
};

#endif // REVERSE_H

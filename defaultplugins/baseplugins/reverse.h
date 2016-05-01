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
        ~Reverse();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget *requestGui(QWidget *parent);
        static const QString id;
        QString help() const;

        int getBlocksize();
        bool getNoBlock();
        bool setBlocksize(int val);
        void setNoBlock(bool val);
    private:
        int blockSize;
        bool noBlock;
};

#endif // REVERSE_H

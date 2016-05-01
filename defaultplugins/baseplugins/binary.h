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
        ~Binary();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        int getBlockSize();
        bool setBlockSize(int val);

    private:
        int blockSize;
};

#endif // BINARY_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ZLIB_H
#define ZLIB_H

#include <transformabstract.h>

class Zlib : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        static const QString XMLREMOVEHEADER;
        explicit Zlib();
        ~Zlib() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        QString help() const;
        QString inboundString() const;
        QString outboundString() const;

        bool setCompression(int level);
        int getCompression();
        void setRemoveHeader(bool flag);
        bool doRemoveHeader();
        
    private:
        int compressionLevel;
        bool removeHeader;
        
};

#endif // ZLIB_H

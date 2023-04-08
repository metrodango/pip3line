/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
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
        ~Zlib() override {}
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        QString help() const override;
        QString inboundString() const override;
        QString outboundString() const override;

        bool setCompression(int level);
        int getCompression();
        void setRemoveHeader(bool flag);
        bool doRemoveHeader();
        
    private:
        int compressionLevel;
        bool removeHeader;
        
};

#endif // ZLIB_H

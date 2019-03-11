/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HTML_H
#define HTML_H


#include <QHash>
#include "transformabstract.h"

class Html : public TransformAbstract
{
        Q_OBJECT
        
    public:
        static const QHash<char,QString> reserverdCharNames;
        explicit Html();
        ~Html() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        bool doWeUseName();
        bool doWeUseHexadecimal();
        bool doWeEncodeAll();
        void setUseName(bool val);
        void setUseHexadecimal(bool val);
        void setEncodeAll(bool val);
    private:
        QByteArray encode(char c);
        static QHash<char,QString> _initializeReservedChar();
        bool useName;
        bool useHexadecimal;
        bool encodeAll;
};

#endif // HTML_H

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
        ~Html();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

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

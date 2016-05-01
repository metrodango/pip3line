/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef XOR_H
#define XOR_H

#include "transformabstract.h"

class Xor : public TransformAbstract
{
    Q_OBJECT
    
    public:
        static const QByteArray HEXCHAR;
        enum Type {Basic = 0, PREVIOUSINPUT = 1, PREVIOUSOUTPUT = 2};
        explicit Xor();
        ~Xor();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        QByteArray getKey() const;
        void setKey(QByteArray val);
        bool isFromHex() const;
        void setFromHex(bool val);
        void setType(Xor::Type val);
        Xor::Type getType() const;
        QString inboundString() const;
        QString outboundString() const;
    private:
        QByteArray applyPreviousInput(const QByteArray & data, QByteArray & key);
        QByteArray applyPreviousOutput(const QByteArray & data, QByteArray & key);
        static const QString XMLXORALGORITHM;
        QByteArray key;
        bool hexDecode;
        Xor::Type xortype;
};

#endif // XOR_H

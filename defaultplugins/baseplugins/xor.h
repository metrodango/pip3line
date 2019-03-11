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
        ~Xor() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        QByteArray getKey() const;
        void setKey(QByteArray val);
        bool isFromHex() const;
        void setFromHex(bool val);
        void setType(Xor::Type val);
        Xor::Type getType() const;
        QString inboundString() const override;
        QString outboundString() const override;
    private:
        QByteArray applyPreviousInput(const QByteArray & data, QByteArray & key);
        QByteArray applyPreviousOutput(const QByteArray & data, QByteArray & key);
        static const QString XMLXORALGORITHM;
        QByteArray key;
        bool hexDecode;
        Xor::Type xortype;
};

#endif // XOR_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HEXENCODE_H
#define HEXENCODE_H

#include "transformabstract.h"

class HexEncode : public TransformAbstract
{
    Q_OBJECT
    public:
        enum Type {NORMAL = 0, ESCAPED = 1, CSTYLE = 2, CSV = 3, ESCAPED_MIXED = 4};
        explicit HexEncode();
        ~HexEncode() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        Type getType();
        void setType(Type ntype);
        bool getAddHexPrefix() const;
        void setAddHexPrefix(bool value);

    private:
        static const QString XML_ADDPREFIX;
        Type type;
        bool addHexPrefix;
};

#endif // HEXENCODE_H

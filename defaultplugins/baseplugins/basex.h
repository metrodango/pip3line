/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASEX_H
#define BASEX_H

#include <transformabstract.h>
#include <QStringList>

class BaseX : public TransformAbstract
{
        Q_OBJECT 
    public:
        static const QString id;
        static const int MINBASE = 2;
        static const int MAXBASE = 36;
        explicit BaseX(int base = 10);
        ~BaseX() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        QString help() const override;

        int getBase() const;
        bool getUppercase() const;
        bool setBase(int val);
        void setUppercase(bool val);
    private:
        int base;
        bool uppercase;
};

#endif // BASEX_H

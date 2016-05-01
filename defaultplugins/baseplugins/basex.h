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
        ~BaseX();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        QString help() const;

        int getBase() const;
        bool getUppercase() const;
        bool setBase(int val);
        void setUppercase(bool val);
    private:
        int base;
        bool uppercase;
};

#endif // BASEX_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef URLENCODE_H
#define URLENCODE_H

#include "transformabstract.h"

class UrlEncode : public TransformAbstract
{
    Q_OBJECT
        
    public:
        static const QByteArray TEXTCHAR;
        explicit UrlEncode();
        ~UrlEncode();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        char getPercentSign();
        QByteArray getExclude();
        QByteArray getInclude();
        void setPercentSign(char val);
        void setExclude(QByteArray vals);
        void setInclude(QByteArray vals);
    private:
        char percentSign;
        QByteArray exclude;
        QByteArray include;
};

#endif // URLENCODE_H

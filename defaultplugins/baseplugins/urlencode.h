/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
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
        ~UrlEncode() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

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

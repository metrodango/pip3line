/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HIEROGLYPHY_H
#define HIEROGLYPHY_H

#include <transformabstract.h>
#include <QStringList>

class Hieroglyphy : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit Hieroglyphy();
        ~Hieroglyphy() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        QString inboundString() const override;
        QString outboundString() const override;
        QString hChar(char c);
        QString hUnicode(QChar u);
        QString hNumber(int num);
        QString hString(QString str);
        QString hScript(QString scr);
        QString toHex(char c);
        QString help() const override;
        QString credits() const override;

        void setUseBtoa(bool val);
        bool isBtoaInUse() const;
    private:
        static const QString XMLBTOAINUSE;
        void init();
        void btoaInit();
        QStringList numbers;
        QHash<char, QString> characters;
        QString Sobject_Object;
        QString SNaN;
        QString Strue;
        QString Sfalse;
        QString Sundefined;
        QString SInfinity;
        QString S1e100;
        QString functionConstructor;
        QString locationString;
        QString unescapeString;
        QString escapeString;
        QString fromCharCodeS;
        QString btoaFunction;
        bool btoaInUse;
};

#endif // HIEROGLYPHY_H

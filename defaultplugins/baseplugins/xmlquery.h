/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef XMLQUERY_H
#define XMLQUERY_H


#include "transformabstract.h"
#include <QAbstractMessageHandler>

class XmlQuery;

class XmlMessageHandler : public QAbstractMessageHandler
{
        Q_OBJECT
    public:
        explicit XmlMessageHandler(XmlQuery *parent);
        ~XmlMessageHandler() {}
        void handleMessage ( QtMsgType type, const QString & description, const QUrl & /*unused*/, const QSourceLocation & /*unused*/ );
    private:
        XmlQuery *parentClass;
};

class XmlQuery : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit XmlQuery();
        ~XmlQuery() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        // specific functions
        QString getQueryString();
        void setQueryString(QString query);
    private:
        void logMessage(QtMsgType type, const QString & description);
        QString queryString;

        friend class XmlMessageHandler;
        XmlMessageHandler *messHandler;
};

#endif // XMLQUERY_H

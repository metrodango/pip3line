/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef JSONVALUE_H
#define JSONVALUE_H

#include "transformabstract.h"
#include <QJsonDocument>

class JsonValue  : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit JsonValue();
        ~JsonValue() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;
        QString getValueName() const;
        void setValueName(const QString &value);
        QJsonDocument::JsonFormat getOutputJsonFormat() const;
        void setOutputJsonFormat(const QJsonDocument::JsonFormat &value);
    private:
        void jsonValueToByteArray(QJsonValue val, QByteArray &output);
        QString valueName;
        QJsonDocument::JsonFormat outputJsonFormat;
};

#endif // JSONVALUE_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef CHARENCODING_H
#define CHARENCODING_H

#include "transformabstract.h"

class CharEncoding : public TransformAbstract
{
    Q_OBJECT
    
    public:
        explicit CharEncoding();
        ~CharEncoding() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        QByteArray getCodecName();
        bool setCodecName(QByteArray val);
        bool getIncludeHeader() const;
        void setIncludeHeader(bool value);

        bool getConvertInvalidToNull() const;
        void setConvertInvalidToNull(bool value);

    private:
        static const QString IGNORE_BOM_XML;
        static const QString CONVERT_INVALID_TO_NULL_XML;
        QByteArray codecName;
        bool includeHeader;
        bool convertInvalidToNull;
};

#endif // CHARENCODING_H

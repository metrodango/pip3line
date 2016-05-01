/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CHARENCODING_H
#define CHARENCODING_H

#include "transformabstract.h"

class CharEncoding : public TransformAbstract
{
    Q_OBJECT
    
    public:
        explicit CharEncoding();
        ~CharEncoding();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

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

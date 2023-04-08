/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef NTLMSSP_H
#define NTLMSSP_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <transformabstract.h>
#include <QTextCodec>
#include <QString>
#include <QBuffer>

class Ntlmssp : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        explicit Ntlmssp();
        ~Ntlmssp() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QWidget * requestGui(QWidget * parent) override;
        QString help() const override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;

        bool decodeBase64() const;
        void setDecodeBase64(bool val);

    private:
        QByteArray extractFlags(quint32 flags);
        QByteArray extractTargetInfo(QByteArray &data);
        QByteArray extractOSVersion(QBuffer &input);
        QByteArray extractNTLM(QByteArray &data);
        QByteArray getString(const QByteArray &data);
        quint32 reverseBytes(quint32 val);
        QByteArray reverseBytes(const QByteArray &data);
        QByteArray toTimeStamp(const QByteArray &data);
        bool readSecurityBuffer(QBuffer &input, quint16 *length, quint16 *maxLength, quint32 *offset, quint32 maxsize);
        bool readFlags(QBuffer &input, quint32 *flags );
        QHash<quint32, QString> NTMLSSP_TYPE;
        QMap<quint32, QString> FLAGS_VAL;
        QMap<quint16, QString> TARGET_INFO;

        bool doBase64;
        bool oemString;
        QTextCodec *unicodeCodec;
        static const int SecurityBufferSize;
        static const int OSFooterSize;
};

#endif // NTLMSSP_H

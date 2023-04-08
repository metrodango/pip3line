/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef BASESTATEABSTRACT_H
#define BASESTATEABSTRACT_H

#include <QObject>
#include <QColor>
#include <QBitArray>
#include <commonstrings.h>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

class BaseStateAbstract : public QObject
{
        Q_OBJECT
    public:
        explicit BaseStateAbstract(QObject *parent = nullptr);
        virtual ~BaseStateAbstract();
        QString getName();
        void setName(const QString &value);
        virtual void run();
        QXmlStreamWriter *getWriter() const;
        void setWriter(QXmlStreamWriter *value);

        QXmlStreamReader *getReader() const;
        void setReader(QXmlStreamReader *value);

        quint64 getFlags() const;
        void setFlags(const quint64 &value);

        bool readNext(QString expected);
        bool readNextStart();
        bool readNextStart(QString expected);
        bool skipUntilStartElement();
        bool skipUntilStartElement(QString expected);
        bool genCloseElement();
        bool writeCloseElement();
        bool readEndElement();
        bool readEndElement(QString expected);
        bool readEndAndNext(QString expected);

        QString write(QByteArray data);
        static QString byteArrayToString(QByteArray data, bool compress = true);
        QByteArray readByteArray(QStringRef data);
        QByteArray readByteArray(QString data);
        static QByteArray stringToByteArray(QString data, bool uncompress = true);

        QString write(bool value);
        bool readBool(QStringRef val);

        QString write(QColor color);
        static QString colorToString(const QColor &color);
        QColor readColor(QStringRef val);
        static QColor stringToColor(QStringRef val);
        static QColor stringToColor(QString val);

        QString write(int val);
        int readInt(QStringRef val, bool *ok);

        QString write(quint64 val);
        quint64 readUInt64(QStringRef val, bool *ok);

        QString write(QBitArray barray);
        QBitArray readBitArray(QStringRef val);

        QString write(QString val, bool compress = false); // for user strings, can be compressed
        QString readString(QStringRef val);

    signals:
        void finished();
        void addNewState(BaseStateAbstract * stateobj);
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);

    public slots:
        void start();

    protected:
        void logLostInTranslation(QString message);
        void logWasExpectingClosing(QString source, QString expected);
        void logReaderIsNull(QString source);
        void logWriterIsNull(QString source);
        void logNotAdequate(QString source, QString expected);
        void logCannotRead(QString source);
        void logStatus(QString mess = QString("Status"));
        QXmlStreamWriter *writer;
        QXmlStreamReader *reader;
        QXmlStreamAttributes attributes;
        quint64 flags;
        QString name;
        QString actionName;
        static const QChar B_ZERO;
        static const QChar B_ONE;
        static const char COMPRESSED_MARKER;
    private:
        Q_DISABLE_COPY(BaseStateAbstract)

};

#endif // BASESTATEABSTRACT_H

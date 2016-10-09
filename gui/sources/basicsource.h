/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASICSOURCE_H
#define BASICSOURCE_H

#include "bytesourceabstract.h"

#include <QBuffer>

class BasicSourceReader : public SourceReader
{
        Q_OBJECT
    public:
        explicit BasicSourceReader(QByteArray *source);
        ~BasicSourceReader();
        bool seek(quint64 pos);
        int read(char * cbuf, int maxLen);
        bool isReadable();
    private:
        const char * rawSource;
        quint64 size;
        quint64 offset;
        QBuffer buffer;
};

class BasicSearch : public SearchAbstract
{
    Q_OBJECT
    public:
        explicit BasicSearch(QByteArray *data);
        ~BasicSearch();
    private:
        void internalStart();
        QByteArray *sdata;
        static const int SearchBlockSize;
};

class BasicSource : public ByteSourceAbstract
{
        Q_OBJECT
    public:
        explicit BasicSource(QObject *parent = 0);
        virtual ~BasicSource();
        QString description();
        virtual void setData(QByteArray data, quintptr source = INVALID_SOURCE);
        QByteArray getRawData();
        void setRawData(QByteArray data, quintptr source = INVALID_SOURCE);
        quint64 size();
        virtual QByteArray extract(quint64 offset, int length);
        char extract(quint64 offset);
        virtual void replace(quint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void insert(quint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void remove(quint64 offset, int length, quintptr source = INVALID_SOURCE);
        virtual void clear(quintptr source = INVALID_SOURCE);
        int getViewOffset(quint64 realoffset);
        int preferredTabType();
        bool isOffsetValid(quint64 offset);
        bool isReadableText();
        virtual void fromLocalFile(QString fileName);
        BaseStateAbstract *getStateMngtObj();
    protected:
        QByteArray rawData;
    private:
        Q_DISABLE_COPY(BasicSource)
        static const QByteArray TEXT;
        static const QString LOGID;
        bool validateOffsetAndSize(quint64 offset, int length);
        SearchAbstract *requestSearchObject(QObject *parent = 0);
        BasicSearch *bsearchObj;
};

class BasicSourceStateObj : public ByteSourceStateObj
{
        Q_OBJECT
    public:
        explicit BasicSourceStateObj(BasicSource *bs);
        ~BasicSourceStateObj();
    protected:
        void internalRun();
};

#endif // BASICSOURCE_H

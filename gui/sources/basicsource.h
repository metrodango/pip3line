/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
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
        ~BasicSourceReader() override;
        bool seek(quint64 pos) override;
        int read(char * cbuf, int maxLen) override;
        bool isReadable() override;
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
        ~BasicSearch() override;
    private:
        void internalStart() override;
        QByteArray *sdata;
        static const int SearchBlockSize;
};

class BasicSource : public ByteSourceAbstract
{
        Q_OBJECT
    public:
        explicit BasicSource(QObject *parent = nullptr);
        virtual ~BasicSource() override;
        QString description() override;
        virtual void setData(QByteArray data, quintptr source = INVALID_SOURCE) override;
        QByteArray getRawData() override;
        void setRawData(QByteArray data, quintptr source = INVALID_SOURCE);
        quint64 size() override;
        virtual QByteArray extract(quint64 offset, int length) override;
        char extract(quint64 offset) override;
        virtual void replace(quint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE) override;
        virtual void insert(quint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE) override;
        virtual void remove(quint64 offset, int length, quintptr source = INVALID_SOURCE) override;
        virtual void clear(quintptr source = INVALID_SOURCE)override;
        int getViewOffset(quint64 realoffset) override;
        int preferredTabType() override;
        bool isOffsetValid(quint64 offset) override;
        bool isReadableText() override;
        virtual void fromLocalFile(QString fileName) override;
        BaseStateAbstract *getStateMngtObj() override;
    protected:
        QByteArray rawData;
    private:
        Q_DISABLE_COPY(BasicSource)
        static const QByteArray TEXT;
        static const QString LOGID;
        bool validateOffsetAndSize(quint64 offset, int length);
        SearchAbstract *requestSearchObject(QObject *parent = nullptr) override;
        BasicSearch *bsearchObj;
};

class BasicSourceStateObj : public ByteSourceStateObj
{
        Q_OBJECT
    public:
        explicit BasicSourceStateObj(BasicSource *bs);
        ~BasicSourceStateObj() override;
    protected:
        void internalRun() override;
};

#endif // BASICSOURCE_H

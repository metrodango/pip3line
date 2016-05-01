/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SEARCHABSTRACT_H
#define SEARCHABSTRACT_H

#include <QMutex>
#include <QThread>
#include <QHash>
#include <QModelIndex>
#include <QAtomicInt>
#include <commonstrings.h>
#include <QIODevice>
#include <QBitArray>
#include <QAbstractListModel>
#include <QList>
#include <QTime>
#include <QColor>

class BytesRangeList;

class SourceReader : public QObject
{
        Q_OBJECT
    public:
        static int MAX_READ_SIZE; // this class is used to read small chunks, setting a small read size limit
        explicit SourceReader();
        virtual ~SourceReader();
        virtual bool seek(quint64 pos) = 0;
        virtual int read(char * buffer, int maxLen) = 0;
        virtual bool isReadable() = 0;
    signals:
        void log(QString mess,QString source,Pip3lineConst::LOGLEVEL level);
};

class SearchWorker : public QObject
{
        Q_OBJECT
    public:
        explicit SearchWorker(SourceReader * device, QObject *parent = 0);
        ~SearchWorker();
        quint64 getStartOffset() const;
        void setStartOffset(const quint64 &value);
        quint64 getEndOffset() const;
        void setEndOffset(const quint64 &value);
        void setSearchItem(QByteArray &value, char *mask);
        BytesRangeList *getFoundList();
        bool getHasError() const;
        void setStatsStep(int value);

    signals:
        void progressUpdate(quint64); // bytes processed
        void log(QString mess,QString source,Pip3lineConst::LOGLEVEL level);
        void finished();
    public slots:
        void cancel();
        void search();
    private:
        quint64 startOffset;
        quint64 endOffset;
        int BufferSize;
        int statsStep;
        bool cancelled;
        char *searchMask;
        char *rawitem;
        SourceReader * targetdevice;
        BytesRangeList * foundList;
        bool listSend;
        int searchSize;
        bool hasError;
};


class SearchAbstract : public QObject
{
        Q_OBJECT
    public:
        SearchAbstract();
        ~SearchAbstract();
        void setSearchItem(const QByteArray &value, QBitArray bitmask = QBitArray());
        BytesRangeList *getGlobalFoundList();
        static QColor SEARCH_COLOR;
        quint64 getCursorOffset() const;
        void setCursorOffset(const quint64 &value);
        bool getJumpToNext() const;
        void setJumpToNext(bool value);

    public slots:
        void startSearch();
        void stopSearch();
    protected slots:
        void onChildFinished();
        void addSearchWorker(SearchWorker * worker);
    signals:
        void foundList(BytesRangeList * list);
        void jumpRequest(quint64 soffset,quint64 eoffset);
        void errorStatus(bool val);
        void log(QString mess,QString source,Pip3lineConst::LOGLEVEL level);
        void progressStatus(double percent);
        void searchStarted();
        void searchEnded();
        void dataReset();
    protected slots:
        void processStats(quint64 val);
    protected:
        static const int MAX_SEARCH_ITEM_SIZE;
        virtual void internalStart() = 0;
        quint64 cursorOffset;
        QByteArray sitem;
        char *mask;
        bool stopped;
        QThread eventThread;
        QHash<SearchWorker *, quint64> workers;
        quint64 oldStats;
        quint64 totalSearchSize;
        int statsStep;
        BytesRangeList * globalFoundList;
        bool hasError;
        bool jumpToNext;
};

#endif // SEARCHABSTRACT_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef LARGERANDOMACCESSSOURCE_H
#define LARGERANDOMACCESSSOURCE_H

#include "bytesourceabstract.h"
#include <QTimer>
#include <QTime>
#include <QSemaphore>

class LargeRandomAccessSource : public ByteSourceAbstract
{
        Q_OBJECT
    public:
        explicit LargeRandomAccessSource(QObject *parent = 0);
        virtual ~LargeRandomAccessSource();
        virtual void setData(QByteArray data, quintptr source = INVALID_SOURCE);
        virtual QByteArray getRawData(); // not always possible
        virtual int viewSize();
        virtual QByteArray extract(quint64 offset, int length);
        virtual QByteArray viewExtract(int offset, int length);
        virtual char extract(quint64 offset);
        virtual char viewExtract(int offset);
        virtual void replace(quint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void viewReplace(int offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void insert(quint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void remove(quint64 offset, int length, quintptr source = INVALID_SOURCE);
        virtual void clear(quintptr source = INVALID_SOURCE);

        virtual void fromLocalFile(QString fileName);

        virtual void viewMark(int start, int end, const QColor &bgcolor,const QColor &fgColor = QColor(), QString toolTip = QString());
        virtual void viewClearMarking(int start, int end);
        virtual QColor getBgViewColor(int pos);
        virtual QColor getFgViewColor(int pos);
        virtual QString getViewToolTip(int pos);

        virtual int getViewOffset(quint64 realoffset);
        virtual quint64 getRealOffset(int viewOffset);
        virtual quint64 startingRealOffset();
        virtual bool isReadableText();
        virtual bool hasDiscreetView();

        virtual int preferredTabType();
        bool isRefreshEnabled() const;
        int refreshInterval() const;
        virtual bool tryMoveView(int size) = 0;

        virtual BaseStateAbstract *getStateMngtObj();

        static const int DEFAULT_CHUNKSIZE;
        
    public slots:
        virtual bool historyForward();
        virtual bool historyBackward();
        virtual void setViewSize(int size);
        void setRefreshEnable(bool refresh);
        void setTimerInterval(int msec);
        virtual bool setStartingOffset(quint64 offset);

    protected slots:
        virtual void refreshData(bool compare = true);

    protected:
        virtual bool validateViewOffsetAndSize(int offset, int length);
        virtual bool readData(QByteArray &data, int size);
        virtual bool readData(quint64 offset, QByteArray &data, int size);
        virtual bool writeData(quint64 offset, int length, const QByteArray &data, quintptr source);
        virtual void addToHistory(quint64 offset);
        QByteArray dataChunk;
        int chunksize;
        quint64 currentStartingOffset;
        QList<quint64> pointerHistory;
        int currentPointerHistoryPointer;

        QTimer refreshTimer;
        int intervalMSec;
        QTime dtimer;
        QSemaphore sem;

};

class LargeRandomAccessSourceStateObj : public ByteSourceStateObj
{
        Q_OBJECT
    public:
        explicit LargeRandomAccessSourceStateObj(LargeRandomAccessSource *ls);
        virtual ~LargeRandomAccessSourceStateObj();
    protected:
        virtual void internalRun();
};

#endif // LARGERANDOMACCESSSOURCE_H

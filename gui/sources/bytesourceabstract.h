/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESOURCEABSTRACT_H
#define BYTESOURCEABSTRACT_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QColor>
#include <QMap>
#include <commonstrings.h>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QSharedPointer>
#include "searchabstract.h"
#include "../shared/guiconst.h"
#include "../state/basestateabstract.h"

class SourceWidgetAbstract;
class BytesRange;

class BytesRangeList : public QObject, public QList<QSharedPointer<BytesRange> > {
        Q_OBJECT
    public:
        explicit BytesRangeList(QObject *parent = nullptr);
        BytesRangeList(BytesRangeList const &other);
        BytesRangeList& operator = (BytesRangeList const &other);
        virtual ~BytesRangeList();
        quint64 byteSize();
        void unify();
    signals:
        void updated();
};

class BytesRange : public QObject
{
        Q_OBJECT
    public:
        static const QString HEXFORMAT;
        explicit BytesRange(quint64 lowerVal, quint64 upperVal, QString description = QString());
        BytesRange(const BytesRange& other);
        BytesRange &operator=(const BytesRange &other);

        virtual ~BytesRange();
        bool isInRange(int value);
        bool isInRange(quint64 value);
        QString getDescription() const ;
        void setDescription(const QString &descr);
        QColor getForeground() const;
        void setForeground(const QColor &color);
        QColor getBackground() const;
        void setBackground(const QColor &color);
        quint64 getLowerVal() const;
        void setLowerVal(quint64 val);
        quint64 getUpperVal() const;
        void setUpperVal(quint64 val);
        virtual bool operator<(const BytesRange& other) const;
        bool hasSameMarkings(const BytesRange& other) const;
        void copyMarkings(const BytesRange& other);
        static QString offsetToString(quint64 val);
        static bool lessThanFunc(QSharedPointer<BytesRange> or1, QSharedPointer<BytesRange> or2);
        quint64 getSize() const;
        void setSize(const quint64 &value);
        static void addMarkToList(BytesRangeList *list, QSharedPointer<BytesRange> range);
        static void addMarkToList(BytesRangeList *list, quint64 start, quint64 end, const QColor &bgcolor, const QColor &fgColor, QString toolTip);
        static void clearMarkingFromList(BytesRangeList *list, quint64 start, quint64 end);
        static void moveMarkingAfterDelete(BytesRangeList *list, quint64 pos, quint64 deleteSize);
        static void moveMarkingAfterInsert(BytesRangeList *list, quint64 pos, quint64 insertSize);
        static void moveMarkingAfterReplace(BytesRangeList *list, quint64 pos, int diff);
        static void compareAndMark(const QByteArray &data1, const QByteArray &data2, BytesRangeList *list, const QColor &bgcolor, const QColor &fgColor, const QString &toolTip);
    protected:
        QString description;
        quint64 lowerVal;
        quint64 upperVal;
        quint64 size;
        QColor foregroundColor;
        QColor backgroundColor;

    friend class ByteSourceAbstract;
    friend class ByteSourceStateObj;
};

class ByteSourceAbstract : public QObject
{
        Q_OBJECT

        friend class ByteSourceStateObj;
    public:
        enum CAPABILITIES {
            CAP_RESET = 0x01,     // can be reset entirely
            CAP_COMPARE = 0x02,   // can be used for comparison algorithm
            CAP_RESIZE = 0x04,    // can be resized
            CAP_WRITE = 0x08,     // can be written to
            CAP_HISTORY = 0x10,   // can manage an history of changes
            CAP_TRANSFORM = 0x20, // can be used for transform (i.e. not too big)
            CAP_LOADFILE = 0x40,  // can accept data from file
            CAP_SEARCH = 0x80     // can be used by the search algorithm
        };

        enum GUI_TYPE {
            GUI_CONFIG,
            GUI_BUTTONS,
            GUI_UPPER_VIEW
        };

        enum HistAction {INSERT = 0, REMOVE = 1, REPLACE = 2};
        struct HistItem {
               HistAction action;
               quint64 offset;
               QByteArray before;
               QByteArray after;
        };

        enum MarkingLayer {USER_MARKINGS = 0, SEARCH_MARKINGS = 1, COMPARE_MARKINGS = 2};

        explicit ByteSourceAbstract(QObject *parent = nullptr);
        virtual ~ByteSourceAbstract();
        virtual QString description() = 0;
        virtual QString name();
        virtual void setName(QString newName);
        virtual QByteArray getRawData(); // not always possible
        virtual quint64 size();
        virtual int viewSize();
        virtual QByteArray extract(quint64 offset, int length) = 0;
        virtual QByteArray viewExtract(int offset, int length);
        virtual char extract(quint64 offset) = 0;
        virtual char viewExtract(int offset);
        virtual void replace(quint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void viewReplace(int offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void insert(quint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void viewInsert(int offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void remove(quint64 offset, int length, quintptr source = INVALID_SOURCE);
        virtual void viewRemove(int offset, int length, quintptr source = INVALID_SOURCE);
        virtual void clear(quintptr source = INVALID_SOURCE);

        virtual void fromLocalFile(QString fileName) = 0; // children have to implement this, to fit specific requirements and limits
        virtual void saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset);
        virtual void saveToFile(QString destFilename);

        virtual int getViewOffset(quint64 realoffset);
        virtual quint64 getRealOffset(int viewOffset);
        virtual quint64 startingRealOffset();
        virtual bool isOffsetValid(quint64 offset);
        virtual bool isReadableText();

        SearchAbstract * getSearchObject(QObject *parent = nullptr,bool singleton = true);

        virtual bool tryMoveUp(int size);
        virtual bool tryMoveDown(int size);
        virtual bool hasDiscreetView();

        QWidget * getGui(QWidget *parent = nullptr,ByteSourceAbstract::GUI_TYPE type = ByteSourceAbstract::GUI_CONFIG);

        virtual void viewMark(int start, int end, const QColor &bgcolor,const QColor &fgColor = QColor(), QString toolTip = QString());

        virtual void viewClearMarking(int start, int end);
        void clearMarking(quint64 start, quint64 end);
        bool hasMarking() const;
        virtual QColor getBgColor(quint64 pos);
        virtual QColor getBgViewColor(int pos);
        virtual QColor getFgColor(quint64 pos);
        virtual QColor getFgViewColor(int pos);
        virtual QString getToolTip(quint64 pos);
        virtual QString getViewToolTip(int pos);

        bool hasCapability(CAPABILITIES cap);
        quint32 getCapabilities() const;
        virtual bool setReadOnly(bool readonly = true);
        virtual bool isReadonly();
        virtual bool checkReadOnly();

        virtual int preferredTabType();
        static QString toPrintableString(const QByteArray &val);

        virtual quint64 lowByte();
        virtual quint64 highByte();
        virtual int textOffsetSize();

        void markNoUpdate(quint64 start, quint64 end, const QColor &bgcolor,const QColor &fgColor = QColor(), QString toolTip = QString());
        BytesRangeList *getUserMarkingsRanges() const;

        virtual BaseStateAbstract *getStateMngtObj() = 0;

        bool hasStaticMarking() const;
        void setStaticMarking(bool value);

        bool isTrackChanges() const;
    public slots:
        virtual bool historyForward();
        virtual bool historyBackward();
        virtual void historyClear();
        virtual void setViewSize(int size);
        void clearAllMarkings();
        void mark(quint64 start, quint64 end, const QColor &bgcolor,const QColor &fgColor = QColor(), QString toolTip = QString());
        virtual void setData(QByteArray data, quintptr source = INVALID_SOURCE); // not always possible
        void setNewMarkings(BytesRangeList * newUserMarkingsRanges);
        void setTrackChanges(bool value);
    private slots:
        void onGuiDestroyed();
        void onMarkingsListDeleted();
    signals:
        void updated(quintptr source);
        void reset();
        void minorUpdate(quint64,quint64);
        void log(QString mess, QString source, Pip3lineConst::LOGLEVEL level);
        void nameChanged(QString newName);
        void sizeChanged();
        void readOnlyChanged(bool readonly);
        void askFileLoad();
        
    protected:
        enum TabType {TAB_GENERIC = 0, TAB_TRANSFORM = 1, TAB_LARGERANDOM = 2};
        virtual QWidget * requestGui(QWidget *parent,ByteSourceAbstract::GUI_TYPE type);
        virtual SearchAbstract *requestSearchObject(QObject *parent);
        void historyApply(HistItem item, bool forward);
        void historyAddInsert(quint64 offset, QByteArray after);
        void historyAddRemove(quint64 offset, QByteArray before);
        void historyAddReplace(quint64 offset, QByteArray before, QByteArray after);
        void historyAdd(HistItem item);
        void writeToFile(QString destFilename, QByteArray data);
        void clearAllMarkingsNoUpdate();
        static const quintptr INVALID_SOURCE;
        QWeakPointer<BytesRange> cachedRange; // internal use value (does not need saving)
        QWidget *confGui;
        QWidget *buttonBar;
        QWidget *upperView;
        SearchAbstract *searchObj;
        bool applyingHistory; // internal use value  (does not need saving)

        BytesRangeList * userMarkingsRanges; // markings
        int currentHistoryPointer;
        QList<HistItem> history; // edit history

        quint32 capabilities; // does not need to be saved as it is specific to the source

        bool _readonly;
        QString _name;
        bool staticMarking; // markings won't get affected by deletion or insertion
        bool trackChanges; // not applicable to all
    private:
        Q_DISABLE_COPY(ByteSourceAbstract)
};

class ByteSourceStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit ByteSourceStateObj(ByteSourceAbstract *bs);
        virtual ~ByteSourceStateObj();
        virtual void run();
    protected:
        virtual void internalRun();
        ByteSourceAbstract *bs;
    private:
        Q_DISABLE_COPY(ByteSourceStateObj)
};

class ByteSourceClosingObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit ByteSourceClosingObj(ByteSourceAbstract *bs);
        ~ByteSourceClosingObj();
        void run();
        void setReadonly(bool value);

    private:
        Q_DISABLE_COPY(ByteSourceClosingObj)
        ByteSourceAbstract *bs;
        bool readonly;
};

#endif // BYTESOURCEABSTRACT_H

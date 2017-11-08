#ifndef PACKETMODELABSTRACT_H
#define PACKETMODELABSTRACT_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QPair>
#include <QItemSelection>
#include <transformabstract.h>

class Packet;
class TransformRequest;
class TransformMgmt;

class PacketModelAbstract : public QAbstractTableModel
{
        Q_OBJECT
    public:

        enum HARDCODED_COLUMN_INDEXES {
            COLUMN_DIRECTION = 0,
            COLUMN_TIMESPTAMP = 1,
            COLUMN_PAYLOAD = 2,
            COLUMN_COMMENT = 3,
            COLUMN_CID = 4,
            COLUMN_LENGTH = 5,
            COLUMN_INVALID = -1
        };

        static const QString COLUMN_DIRECTION_STR;
        static const QString COLUMN_TIMESPTAMP_STR;
        static const QString COLUMN_PAYLOAD_STR;
        static const QString COLUMN_COMMENT_STR;
        static const QString COLUMN_CID_STR;
        static const QString COLUMN_LENGTH_STR;

        static const int DEFAULT_MAX_PAYLOAD_DISPLAY_SIZE;
        static const int MAX_PAYLOAD_DISPLAY_SIZE_MIN_VAL;
        static const int MAX_PAYLOAD_DISPLAY_SIZE_MAX_VAL;

        static const qint64 INVALID_POS;
        PacketModelAbstract(TransformMgmt *transformFactory, QObject *parent = nullptr);
        virtual ~PacketModelAbstract();

        virtual QStringList getColumnNames() const;
        virtual void setColumnNames(const QStringList &value);
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
        virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

        virtual QVariant payloadData(const Packet *packet,  int column, int role) const;

        virtual Packet *getPacket(qint64 index) = 0;
        virtual Packet *getPacket(const QModelIndex & index);
        virtual qint64 indexToPacketIndex(const QModelIndex & index);
        virtual void removePacket(qint64 index) = 0;
        virtual void removePackets(QList<qint64> index) = 0;
        virtual qint64 merge(QList<qint64> list) = 0;
        virtual qint64 size() const = 0 ;

        QModelIndex	createIndex ( int row, int column) const;
        virtual QItemSelection getLastPacketRow() = 0;

        void addUserColumn(const QString &name, TransformAbstract * transform, OutputFormat outFormat);
        QWidget *getGuiForUserColumn(const QString &name, QWidget *parent = nullptr);
        QWidget *getGuiForUserColumn(int index, QWidget *parent = nullptr);
        TransformAbstract *getTransform(const QString &columnName);
        TransformAbstract *getTransform(int index);
        void setColumnFormat(int index, Pip3lineConst::OutputFormat format);
        void setColumnFormat(const QString &columnName, Pip3lineConst::OutputFormat format);
        Pip3lineConst::OutputFormat getColumnFormat(int index);
        Pip3lineConst::OutputFormat getColumnFormat(const QString &columnName);
        QString getColumnName(int index);
        void removeUserColumn(const QString &name);
        void removeUserColumn(const int &index);
        void clearUserColumns();
        bool isUserColumn(const QString &name) const;
        bool isUserColumn(int column) const;
        bool isDefaultColumn(const QString &name) const;
        bool isDefaultColumn(int column) const;

        int getColumnIndex(const QString &name);
        virtual void clearAdditionalField(const QString &name) = 0;
        bool isAutoMergeConsecutivePackets() const;
        int getMaxPayloadDisplaySize() const;
        int getDefaultWidthForColumn(const int &col);
    signals:
        void updated();
        void columnsUpdated();
        void sendRequest(TransformRequest *);
        void readOnlyStateChanged(bool readonly);
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
    public slots:
        virtual qint64 addPacket(Packet * packet) = 0;
        virtual qint64 addPackets(QList<Packet *> packets) = 0;
        virtual void mergeConsecutivePackets() = 0;
        virtual void clear() = 0;
        virtual void refreshAllColumn();
        void setAutoMergeConsecutivePackets(bool value);
        void setMaxPayloadDisplaySize(int value);
    protected slots:
        virtual void transformRequestFinished(QList<QByteArray> dataList, Messages messages) = 0;
        virtual void transformUpdated() = 0;
        virtual void refreshColumn(const QString colName) = 0;
        virtual void onRowsInserted(const QModelIndex & parent, int start, int end);
    protected:
        struct Usercolumn {
                Usercolumn() : transform(nullptr), format(Pip3lineConst::TEXTFORMAT) {}
                TransformAbstract * transform;
                OutputFormat format;
        };

        enum DEFAULT_COLUMN_WIDTH {
            TIMESTAMP_COLUMN_WIDTH = 150,
            DIRECTION_COLUMN_WIDTH = 25,
            LENGTH_COLUMN_WIDTH = 40,
            CID_COLUMN_WIDTH = 50,
            RAWDATA_COLUMN_WIDTH = 200
        };

        virtual void internalAddUserColumn(const QString &name, TransformAbstract * transform) = 0;
        virtual void launchUpdate(TransformAbstract * transform, int row, int column,int length = -1) = 0;

        void resetColumnNames();
        static const QString TRUNCATED_STR;
        QStringList columnNames;
        static const QString DEFAULT_DATETIME_FORMAT;
        QString dateTimeFormat;
        QHash<QString, Usercolumn> userColumnsDef;
        QHash<quintptr, QPair<int, int> > transformRequests;
        TransformMgmt *transformFactory;
        bool autoMergeConsecutivePackets;
        int lastPredefinedColumn;
        int maxPayloadDisplaySize;
};

#endif // PACKETMODELABSTRACT_H

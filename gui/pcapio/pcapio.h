#ifndef PCAPIO_H
#define PCAPIO_H

#include <QByteArray>
#include <QObject>
#include <QIODevice>
#include <QList>
#include <QHash>
#include <QString>
#include <QDateTime>
#include "pcapdef.h"

class PcapPacket
{
public:
    explicit PcapPacket(QByteArray data = QByteArray());
    quint32 getTimestampRaw() const;
    QDateTime getTimestamp() const;
    void setTimestamp(const quint32 &value);
    void setTimestamp(const QDateTime &value);

    quint32 getMicrosec() const;
    void setMicrosec(const quint32 &value);

    quint32 getOriginalSize() const;
    void setOriginalSize(const quint32 &value);

    quint32 getCapturedSize() const;
    void setCapturedSize(const quint32 &value);

    QByteArray getData() const;
    void setData(const QByteArray &value);

    private:
    quint32 timestamp;
    quint32 microsec;
    quint32 originalSize;
    quint32 capturedSize;
    QByteArray data;
};

class QFile;

class PcapIO : public QObject
{
    Q_OBJECT
    public:
        static const QByteArray PCAP_MAGIC_LE;
        static const QByteArray PCAP_MAGIC_BE;
        explicit PcapIO(QString filename = QString(), QObject *parent = 0);
        explicit PcapIO(QIODevice * device, QObject *parent = 0);
        ~PcapIO();
        bool openExistingFile(QIODevice::OpenModeFlag mode);
        bool createAndOpenFile();
        void close();

        PcapPacket *nextPacket();
        QList<PcapPacket *> getPacketList();
        void appendPacket(PcapPacket * packet);
        void appendPacketList(QList<PcapPacket *> list);

        quint64 numberOfPackets();
        quint16 getVersionMajor() const;
        quint16 getVersionMinor() const;
        quint32 getTimeZoneShift() const;
        quint32 getPrecision() const;
        quint32 getMaxSnapshotlenght() const;
        PcapDef::Link_Type getLinkLayerType() const;
        QString getLinkLayerTypeString() const;
        void setVersionMajor(const quint16 &value);
        void setVersionMinor(const quint16 &value);
        void setTimeZoneShift(const quint32 &value);
        void setPrecision(const quint32 &value);
        void setMaxSnapshotlenght(const quint32 &value);
        void setLinkLayerType(const PcapDef::Link_Type &value);
        QString getFilename() const;
        void setFilename(const QString &value);
        bool getLittleEndian() const;
        void setLittleEndian(bool value);
        static QByteArray reverseBytesOrder(QByteArray &in);
    private:
        static  QHash<int, QString> initTypeList();
        void initAttributes();
        bool readUInt32(quint32 *temp, QString field = QString("Unknown"));
        bool readUInt16(quint16 *temp, QString field = QString("Unknown"));
        bool readByteArray(QByteArray *temp, qint64 length, QString field = QString("Unknown"));
        QByteArray packetToBytes(PcapPacket *packet);
        QByteArray uint32ToBytes(quint32 val);
        QByteArray uint16ToBytes(quint16 val);
        QByteArray generatePcapHeader();
        bool readExistingFile();
        QString filename;
        QIODevice *file;
        bool reverseOrder;
        bool littleEndian;
        quint16 versionMajor;
        quint16 versionMinor;
        quint32 timezoneshift;
        quint32 precision;
        quint32 maxSnapshotlenght;
        PcapDef::Link_Type linkLayerType;
        qint64 packetsStart;
        qint64 currentPos;
        quint64 numberOfPacketsInFile;
        bool ownFileObject;
};

#endif // PCAPIO_H

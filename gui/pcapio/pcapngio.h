#ifndef PCAPNGIO_H
#define PCAPNGIO_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QHash>
#include <QVariant>
#include <QByteArray>
#include "pcapdef.h"

class GeneralBlock {
    public:
        struct GeneralBlockStructHeader {
               quint32 type;
               quint32 totalLenth;
        };

        explicit GeneralBlock();
        virtual ~GeneralBlock();
        quint32 getBlockType() const;
        void setBlockType(const quint32 &value);
        quint32 getBlockLength() const;
        void setBlockLength(const quint32 &value);
        qint64 getOffset() const;
        void setOffset(const qint64 &value);

    protected:
        quint32 blockType;
        quint32 blockLength;
        qint64 offset;
        QHash<int, QVariant> options;
};

class SectionHeader : public GeneralBlock {
    public:
        struct SectionHeaderStructHeader : GeneralBlockStructHeader{
                quint32 byteOrderMagic;
                quint16 versionMajor;
                quint16 versionMinor;
                qint64 sectionLength;
        };
        explicit SectionHeader();
        ~SectionHeader();
        quint32 getByteOrderMagic() const;
        void setByteOrderMagic(const quint32 &value);
        bool isLittleEndian();
        quint16 getVersionMajor() const;
        void setVersionMajor(const quint16 &value);
        quint16 getVersionMinor() const;
        void setVersionMinor(const quint16 &value);

    private:
        static const quint32 MAGIC;
        quint32 byteOrderMagic;
        quint16 versionMajor;
        quint16 versionMinor;
        qint64 sectionLength;
};

class InterfaceDescriptionBlock : public GeneralBlock {
    public:
        explicit InterfaceDescriptionBlock();
        ~InterfaceDescriptionBlock();

        PcapDef::Link_Type getLinkType() const;
        void setLinkType(const PcapDef::Link_Type &value);

        quint16 getReserved() const;
        void setReserved(const quint16 &value);

        quint32 getSnapLen() const;
        void setSnapLen(const quint32 &value);

    private:
        PcapDef::Link_Type linkType;
        quint16 reserved;
        quint32 snapLen;
};

class AbstractPcapNGPacket : public GeneralBlock {
    public:
        explicit AbstractPcapNGPacket();
        virtual ~AbstractPcapNGPacket();

        QByteArray getData() const;
        void setData(const QByteArray &value);

    protected:
        QByteArray data;
        quint32 originalPacketSize;
};

class PcapNgIO : public QObject
{
        Q_OBJECT
    public:
        enum BlockType {
            BLOCK_TYPE_IDB = 0x00000001,
            BLOCK_TYPE_PACKET_BLOCK = 0x00000002,
            BLOCK_TYPE_SIMPLE_PACKET_BLOCK = 0x00000003,
            BLOCK_TYPE_NAME_RESOLUTION_BLOCK = 0x00000004,
            BLOCK_TYPE_INTERFACE_STATISTICS_BLOCK = 0x00000005,
            BLOCK_TYPE_ENHANCED_PACKET_BLOCK = 0x00000006,
            BLOCK_TYPE_IRIG_TIMESTAMP_BLOCK = 0x00000007,
            BLOCK_TYPE_ARINC = 0x00000008,
            BLOCK_TYPE_CUSTOM_BLOCK_COPYABLE = 0x00000BAD,
            BLOCK_TYPE_CUSTOM_BLOCK_NOT_COPYABLE = 0x40000BAD,
            BLOCK_TYPE_SECTION_HEADER_BLOCK = 0x0A0D0D0A
        };

        static const QHash<quint32, QString> BLOCK_DESC;
        static const QHash<quint32, QStringList> BLOCK_OPTIONS;

        explicit PcapNgIO(QObject *parent = 0);
        ~PcapNgIO();

        AbstractPcapNGPacket *nextPacket();
        QList<AbstractPcapNGPacket *> getPacketList();
        void appendPacket(AbstractPcapNGPacket * packet);


    private:

        static GeneralBlock *readNextBlock();
        static QHash<quint32, QString>initBlockDesc();
        static QHash<quint32, QStringList>initBlockOptions();
        static QByteArray reverseBytesOrder(QByteArray &in);

        bool readUInt32(quint32 *temp, QString field = QString("Unknown"));
        bool readUInt16(quint16 *temp, QString field = QString("Unknown"));
        bool readByteArray(QByteArray *temp, qint64 length, QString field = QString("Unknown"));
        QByteArray uint32ToBytes(quint32 val);
        QByteArray uint16ToBytes(quint16 val);

        QString filename;
        QIODevice *file;
        bool reverseOrder;
        bool littleEndian;
        SectionHeader *lastSHB;
        QList<SectionHeader *> sectionHeaders;
};

#endif // PCAPNGIO_H

#include "pcapngio.h"
#include <QDebug>

GeneralBlock::GeneralBlock()
{
    blockType = 0;
    blockLength = 0;
    offset = 0;
}

GeneralBlock::~GeneralBlock()
{

}
quint32 GeneralBlock::getBlockType() const
{
    return blockType;
}

void GeneralBlock::setBlockType(const quint32 &value)
{
    blockType = value;
}
quint32 GeneralBlock::getBlockLength() const
{
    return blockLength;
}

void GeneralBlock::setBlockLength(const quint32 &value)
{
    blockLength = value;
}
qint64 GeneralBlock::getOffset() const
{
    return offset;
}

void GeneralBlock::setOffset(const qint64 &value)
{
    offset = value;
}

const quint32 SectionHeader::MAGIC = 0x0A0D0D0A;

SectionHeader::SectionHeader()
{
    byteOrderMagic = MAGIC;
    versionMajor = 1;
    versionMinor = 0;
    sectionLength = -1;
}

SectionHeader::~SectionHeader()
{

}

quint32 SectionHeader::getByteOrderMagic() const
{
    return byteOrderMagic;
}

void SectionHeader::setByteOrderMagic(const quint32 &value)
{
    byteOrderMagic = value;
}

bool SectionHeader::isLittleEndian()
{
#ifdef Q_LITTLE_ENDIAN
    return byteOrderMagic == MAGIC;
#else
    return byteOrderMagic != MAGIC;
#endif
}

quint16 SectionHeader::getVersionMajor() const
{
    return versionMajor;
}

void SectionHeader::setVersionMajor(const quint16 &value)
{
    versionMajor = value;
}
quint16 SectionHeader::getVersionMinor() const
{
    return versionMinor;
}

void SectionHeader::setVersionMinor(const quint16 &value)
{
    versionMinor = value;
}

const QHash<quint32, QString> PcapNgIO::BLOCK_DESC = PcapNgIO::initBlockDesc();
const QHash<quint32, QStringList> PcapNgIO::BLOCK_OPTIONS = PcapNgIO::initBlockOptions();

PcapNgIO::PcapNgIO(QObject *parent) : QObject(parent)
{
    file = nullptr;
    lastSHB = nullptr;
    reverseOrder = false;

#ifdef Q_LITTLE_ENDIAN
    littleEndian = true;
#else
    littleEndian = false;
#endif

}

PcapNgIO::~PcapNgIO()
{

}

AbstractPcapNGPacket *PcapNgIO::nextPacket()
{
    return nullptr;
}

QList<AbstractPcapNGPacket *> PcapNgIO::getPacketList()
{
    QList<AbstractPcapNGPacket *>  list;
    return list;
}

void PcapNgIO::appendPacket(AbstractPcapNGPacket */* packet */)
{

}

GeneralBlock *PcapNgIO::readNextBlock()
{
    return nullptr;
}

QHash<quint32, QString> PcapNgIO::initBlockDesc()
{
    QHash<quint32, QString> ret;
    ret.insert(BLOCK_TYPE_IDB, "Interface Description Block");
    ret.insert(BLOCK_TYPE_PACKET_BLOCK, "Packet Block (Obsolete)");
    ret.insert(BLOCK_TYPE_SIMPLE_PACKET_BLOCK, "Simple Packet Block");
    ret.insert(BLOCK_TYPE_NAME_RESOLUTION_BLOCK, "Name Resolution Block");
    ret.insert(BLOCK_TYPE_INTERFACE_STATISTICS_BLOCK, "Interface Statistics Block");
    ret.insert(BLOCK_TYPE_ENHANCED_PACKET_BLOCK, "Enhanced Packet Block");
    ret.insert(BLOCK_TYPE_IRIG_TIMESTAMP_BLOCK, "IRIG Timestamp Block");
    ret.insert(BLOCK_TYPE_ARINC, "ARINC 429 Block");
    ret.insert(BLOCK_TYPE_CUSTOM_BLOCK_COPYABLE, "Custom Block than can be copied into new files");
    ret.insert(BLOCK_TYPE_CUSTOM_BLOCK_NOT_COPYABLE, "Custom Block than should not be copied into new files");
    ret.insert(BLOCK_TYPE_SECTION_HEADER_BLOCK, "Section Header Block");

    return ret;
}

QHash<quint32, QStringList> PcapNgIO::initBlockOptions()
{
    QHash<quint32, QStringList> ret;
    QStringList optionsList;

    // Section Header Block
    optionsList << "End of Options"
                << "Comment"
                << "Hardware"
                << "OS"
                << "Application";

    ret.insert(BLOCK_TYPE_SECTION_HEADER_BLOCK, optionsList);

    // Interface Description Block
    optionsList.clear();

    optionsList << "End of Options"
                << "Comment"
                << "Interface name"
                << "Interface description"
                << "IPv4 address"
                << "IPv6 address"
                << "MAC address"
                << "Interface Hardware EUI address"
                << "Interface Speed"
                << "Timestamp resolution"
                << "Timezone"
                << "Filter"
                << "OS"
                << "Frame Check Sequence length"
                << "Timestamp offset";

    ret.insert(BLOCK_TYPE_IDB, optionsList);

    // Enhanced Packet Block
    optionsList.clear();

    optionsList << "End of Options"
                << "Comment"
                << "Flags"
                << "Hash"
                << "Packets loss";

    ret.insert(BLOCK_TYPE_ENHANCED_PACKET_BLOCK, optionsList);

    // Name Resolution Block
    optionsList.clear();

    optionsList << "End of Options"
                << "Comment"
                << "Domain Name Server"
                << "DNS IPv4"
                << "DNS IPv6";

    ret.insert(BLOCK_TYPE_NAME_RESOLUTION_BLOCK, optionsList);

    // Interface Statistics Block
    optionsList.clear();

    optionsList << "End of Options"
                << "Comment"
                << "Start time"
                << "End time"
                << "Packets received by the interface"
                << "Packets dropped by the interface"
                << "Packets accepted by filter"
                << "Packets dropped by the OS"
                << "Packets delivered to user";

    ret.insert(BLOCK_TYPE_INTERFACE_STATISTICS_BLOCK, optionsList);

    // Packet Block (obsolete)
    optionsList.clear();

    optionsList << "End of Options"
                << "Comment"
                << "Flags"
                << "Hash";
    ret.insert(BLOCK_TYPE_PACKET_BLOCK, optionsList);

    return ret;
}

QByteArray PcapNgIO::reverseBytesOrder(QByteArray &in)
{
    QByteArray ret;
    ret.resize(in.size());
    for(int i=in.size(); i>=0; --i)
        ret.append(in.at(i));
    return ret;
}

bool PcapNgIO::readUInt32(quint32 *temp, QString field)
{
    QByteArray fourBytes(sizeof(quint32), '\0');
    qint64 read = file->read(fourBytes.data(), sizeof(quint32));
    if (read != sizeof(quint32)) {
        qWarning() << "[PcapNgIO::readUInt32] Invalid uint32:" << field;
        return false;
    }

    if (reverseOrder)
        fourBytes = reverseBytesOrder(fourBytes);

    memcpy(temp, fourBytes.constData(), sizeof(quint32));

    return true;
}

bool PcapNgIO::readUInt16(quint16 *temp, QString field)
{
    QByteArray twoBytes(sizeof(quint16), '\0');
    qint64 read = file->read(twoBytes.data(), sizeof(quint16));
    if (read != sizeof(quint16)) {
        qWarning() << "[PcapNgIO::readUInt16] Invalid field:" << field;
        return false;
    }

    if (reverseOrder)
        twoBytes = reverseBytesOrder(twoBytes);

    memcpy(temp, twoBytes.constData(), sizeof(quint16));

    return true;
}

bool PcapNgIO::readByteArray(QByteArray *temp, qint64 length, QString field)
{
    if (temp == nullptr) {
        qWarning() << "[PcapNgIO::readByteArray] null bytearray:" << field;
        return false;
    }

    temp->resize(length);

    qint64 read = file->read(temp->data(), length);
    if (read != length) {
        qWarning() << "[PcapNgIO::readByteArray] Invalid byte array:" << field;
        return false;
    }

    return true;
}

QByteArray PcapNgIO::uint32ToBytes(quint32 val)
{
    QByteArray fourBytes(sizeof(quint32),'\0');
    memcpy(fourBytes.data(),&val,sizeof(quint32));

    if (reverseOrder) {
        fourBytes = reverseBytesOrder(fourBytes);
    }

    return fourBytes;
}

QByteArray PcapNgIO::uint16ToBytes(quint16 val)
{
    QByteArray twoBytes(sizeof(quint16),'\0');
    memcpy(twoBytes.data(),&val,sizeof(quint16));

    if (reverseOrder) {
        twoBytes = reverseBytesOrder(twoBytes);
    }

    return twoBytes;
}

AbstractPcapNGPacket::AbstractPcapNGPacket()
{
    originalPacketSize = 0;
}

AbstractPcapNGPacket::~AbstractPcapNGPacket()
{

}
QByteArray AbstractPcapNGPacket::getData() const
{
    return data;
}

void AbstractPcapNGPacket::setData(const QByteArray &value)
{
    data = value;
}

InterfaceDescriptionBlock::InterfaceDescriptionBlock()
{
    linkType = PcapDef::LINKTYPE_nullptr;
    reserved = 0;
    snapLen = 0;
}

InterfaceDescriptionBlock::~InterfaceDescriptionBlock()
{

}
PcapDef::Link_Type InterfaceDescriptionBlock::getLinkType() const
{
    return linkType;
}

void InterfaceDescriptionBlock::setLinkType(const PcapDef::Link_Type &value)
{
    linkType = value;
}
quint16 InterfaceDescriptionBlock::getReserved() const
{
    return reserved;
}

void InterfaceDescriptionBlock::setReserved(const quint16 &value)
{
    reserved = value;
}
quint32 InterfaceDescriptionBlock::getSnapLen() const
{
    return snapLen;
}

void InterfaceDescriptionBlock::setSnapLen(const quint32 &value)
{
    snapLen = value;
}




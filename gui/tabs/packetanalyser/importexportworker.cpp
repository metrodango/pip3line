#include "importexportworker.h"
#include <QFile>
#include <QDebug>
#if QT_VERSION >= 0x050000
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#endif
#include "pcapio/pcapio.h"
#include "packet.h"
#include "shared/guiconst.h"
#include "state/basestateabstract.h"

ImportExportWorker::ImportExportWorker(PacketModelAbstract *model, QString filename, GuiConst::FileOperations ops, GuiConst::FileFormat format, QObject *parent) :
    QObject(parent),
    ops(ops),
    format(format),
    filename(filename),
    model(model)
{
    pcapLinkType = PcapDef::LINKTYPE_ETHERNET; // ethernet
    exportFormattedXML = true;
    applyFilter = false;
    currentIndex = -1;
    noMore = false;
    if (filename.isEmpty()) { // in case this is not specified
        this->ops = GuiConst::UNKNOWN_OPERATION;
        this->format = GuiConst::INVALID_FORMAT;
    }

    if (this->ops == GuiConst::IMPORT_OPERATION)
        connect(this, SIGNAL(newPacket(Packet*)), model, SLOT(addPacket(Packet*)),Qt::QueuedConnection);

    connect(this, SIGNAL(finished()), SLOT(deleteLater()), Qt::QueuedConnection);
}

ImportExportWorker::~ImportExportWorker()
{
    //qDebug() << "Destroying" << this;
}

void ImportExportWorker::run()
{
    if (filename.isEmpty() || ops == GuiConst::UNKNOWN_OPERATION || format == GuiConst::INVALID_FORMAT) {
        qCritical() << tr("[ImportExportWorker::run] worker not configured properly to work on its own: %1 | %2 | %3").arg(filename).arg(ops).arg(format);
        return;
    }
    QFile target(filename);
    if (ops == GuiConst::EXPORT_OPERATION) {
        if (target.open(QIODevice::WriteOnly)) {
            switch (format) {
                case GuiConst::PCAP_FORMAT:
                    toPcap(&target);
                    break;
                case GuiConst::XML_FORMAT:
                    toXMLFile(&target);
                    break;
                case GuiConst::JSON_FORMAT:
#if QT_VERSION >= 0x050000
                    toJSonFile(&target);
#else
                    qCritical() << tr("Json is only supported with QT > 5.0");
#endif
                    break;
                default:
                    qCritical() << tr("[ImportExportWorker::run] Unmanaged format for writing: %1 T_T").arg(format);
                    break;
            }
        }
    } else if (ops == GuiConst::IMPORT_OPERATION) {
        if (target.open(QIODevice::ReadOnly)) {
            switch (format) {
                case GuiConst::PCAP_FORMAT:
                    loadFromPcap(&target);
                    break;
                case GuiConst::XML_FORMAT:
                    loadFromXMLFile(&target);
                    break;
                case GuiConst::JSON_FORMAT:
#if QT_VERSION >= 0x050000
                    loadFromJsonFile(&target);
#else
                    qCritical() << tr("Json is only supported with QT > 5.0");
#endif
                    break;
                default:
                    qCritical() << tr("[ImportExportWorker::run] Unmanaged format for reading: %1 T_T").arg(format);
                    break;
            }
        }
    }

    if (target.isOpen())
        target.close();
    emit finished();
}

void ImportExportWorker::toPcap(QIODevice *file)
{
    PcapIO *pfile = new(std::nothrow) PcapIO(file);
    if (pfile == nullptr)
        qFatal("Cannot allocate memory for PcapIO X{");

    pfile->setLinkLayerType(pcapLinkType);

    if (pfile->createAndOpenFile()) {
        Packet *pac = nextPacket();
        while (pac != nullptr ) {
            PcapPacket * ppacket = new(std::nothrow) PcapPacket(pac->getData());
            if (ppacket == nullptr)
                qFatal("Cannot allocate memory for PcapPacket X{");

            ppacket->setTimestamp(pac->getTimestamp());
            // at this point ppacket->microsec is missing the last three digits
            ppacket->setMicrosec(ppacket->getMicrosec() + pac->getMicrosec());

            pfile->appendPacket(ppacket);
            delete ppacket;
            pac = nextPacket();
        }

        pfile->close();
    } else {
        qCritical("Cannot open the pcap file");
    }
}

void ImportExportWorker::loadFromPcap(QIODevice *file)
{
    PcapIO *pfile = new(std::nothrow) PcapIO(file);
    if (pfile == nullptr)
        qFatal("Cannot allocate memory for PcapIO X{");

    if (pfile->openExistingFile(QIODevice::ReadOnly)) {
        PcapPacket * ppacket = pfile->nextPacket();
        qint64 count = 0;
        while (ppacket != nullptr) {
            count++;
            Packet *pac = new(std::nothrow) Packet(ppacket->getData());
            if (pac == nullptr)
                qFatal("Cannot allocate memory for Packet X{");

            if ((quint32)ppacket->getData().size() < ppacket->getOriginalSize()) {
                qWarning() << tr("[Pcap] Packet %1 appears to have been truncated").arg(count);
            }
            pac->setTimestamp(ppacket->getTimestamp());
            pac->setMicrosec(ppacket->getMicrosec() % 1000);

            emit newPacket(pac);

            delete ppacket;
            ppacket = pfile->nextPacket();
        }
    } else {
        qCritical() << tr("Cannot open the pcap file");
    }
}

void ImportExportWorker::toXML(QXmlStreamWriter *stream)
{
    stream->writeStartElement(GuiConst::STATE_PACKET_LIST);
    Packet *pac = nextPacket();
    while (pac != nullptr ) {
        stream->writeStartElement(GuiConst::STATE_PACKET);
        stream->writeAttribute(GuiConst::STATE_TIMESTAMP, QString::number(pac->getTimestamp().toMSecsSinceEpoch()));
        if (pac->getMicrosec() != 0)
            stream->writeAttribute(GuiConst::STATE_MICROSEC, QString::number(pac->getMicrosec()));
        if (pac->getDirection() != Packet::NODIRECTION)
            stream->writeAttribute(GuiConst::STATE_DIRECTION, QString::number(pac->getDirection()));
        if (!pac->getComment().isEmpty())
            stream->writeAttribute(GuiConst::STATE_COMMENT, QString::fromUtf8(pac->getComment().toUtf8().toBase64()));
        if (!pac->getSourceString().isEmpty())
            stream->writeAttribute(GuiConst::STATE_SOURCE_STRING, QString::fromUtf8(pac->getSourceString().toUtf8().toBase64()));
        if (pac->getForeground().isValid())
            stream->writeAttribute(GuiConst::STATE_FOREGROUNG_COLOR, BaseStateAbstract::colorToString(pac->getForeground()));
        if (pac->getBackground().isValid())
            stream->writeAttribute(GuiConst::STATE_BACKGROUNG_COLOR, BaseStateAbstract::colorToString(pac->getBackground()));

        if (pac->isInjected())
            stream->writeAttribute(GuiConst::STATE_INJECTED_PACKET, QString::number(1));

        stream->writeTextElement(GuiConst::STATE_DATA, BaseStateAbstract::byteArrayToString(pac->getData()));
        if (pac->hasBeenModified())
            stream->writeTextElement(GuiConst::STATE_ORIGINAL_DATA, BaseStateAbstract::byteArrayToString(pac->getOriginalData()));
        QHash<QString, QString> fields = pac->getAdditionalFields();
        if (fields.size() > 0) {
            stream->writeStartElement(GuiConst::STATE_ADDITIONAL_FIELDS_LIST);
            QHashIterator<QString, QString> i(fields);
             while (i.hasNext()) {
                 i.next();
                 stream->writeTextElement(i.key(), QString::fromUtf8(i.value().toUtf8().toBase64()));
             }
            stream->writeEndElement(); // STATE_ADDITIONAL_FIELDS_LIST
        }

        stream->writeEndElement(); // PACKET
        pac = nextPacket();
    }
    stream->writeEndElement(); // PACKET_LIST
}

void ImportExportWorker::toXMLFile(QIODevice *file)
{
    QXmlStreamWriter stream(file);
    stream.setAutoFormatting(exportFormattedXML);
    toXML(&stream);
}

void ImportExportWorker::loadFromXML(QXmlStreamReader *stream)
{
    if (stream->name() == GuiConst::STATE_PACKET_LIST || stream->readNextStartElement()) {
        if (stream->name() == GuiConst::STATE_PACKET_LIST) {
            while (stream->readNextStartElement()) {
                if (stream->name() == GuiConst::STATE_PACKET) {
                    Packet *pac = new(std::nothrow) Packet();
                    if (pac == nullptr)
                        qFatal("[ImportExportWorker::loadFromXML] Cannot allocate memory for Packet X{");

                    bool ok = false;
                    QXmlStreamAttributes attributes = stream->attributes();
                    QDateTime dateTime;
                    if (attributes.hasAttribute(GuiConst::STATE_TIMESTAMP)) {
                        qint64 timems = attributes.value(GuiConst::STATE_TIMESTAMP).toString().toLongLong(&ok);
                        if (!ok) {
                            qCritical() << tr("[ImportExportWorker::loadFromXML] Cannot parse the timestamp: %1").arg(attributes.value(GuiConst::STATE_TIMESTAMP).toString());
                            pac->setTimestamp(dateTime);
                        }
                        else {
                            dateTime = QDateTime::fromMSecsSinceEpoch(timems);
                            pac->setTimestamp(dateTime);
                        }
                    } else {
                        qCritical() << tr("[ImportExportWorker::loadFromXML] No timestamp attribute");
                        pac->setTimestamp(dateTime);
                    }

                    if (attributes.hasAttribute(GuiConst::STATE_MICROSEC)) {
                        quint32 microsec = attributes.value(GuiConst::STATE_MICROSEC).toString().toUInt(&ok);
                        if (!ok) {
                            qCritical() << tr("[ImportExportWorker::loadFromXML] Cannot parse microseconds: %1").arg(attributes.value(GuiConst::STATE_MICROSEC).toString());
                        }
                        else {
                            pac->setMicrosec(microsec);
                        }
                    }

                    if (attributes.hasAttribute(GuiConst::STATE_DIRECTION)) {
                        qint32 direcInt = attributes.value(GuiConst::STATE_DIRECTION).toString().toInt(&ok);
                        if (!ok || (direcInt != Packet::LEFTRIGHT && direcInt != Packet::RIGHTLEFT && direcInt != Packet::NODIRECTION)) {
                            qCritical() << tr("[ImportExportWorker::loadFromXML] Cannot parse direction: %1").arg(attributes.value(GuiConst::STATE_DIRECTION).toString());
                        } else {
                            pac->setDirection((Packet::Direction)direcInt);
                        }
                    }

                    if (attributes.hasAttribute(GuiConst::STATE_COMMENT)) {
                        pac->setComment(QString::fromUtf8(QByteArray::fromBase64(attributes.value(GuiConst::STATE_COMMENT).toUtf8())));
                    }

                    if (attributes.hasAttribute(GuiConst::STATE_SOURCE_STRING)) {
                        pac->setSourceString(QString::fromUtf8(QByteArray::fromBase64(attributes.value(GuiConst::STATE_SOURCE_STRING).toUtf8())));
                    }
                    if (attributes.hasAttribute(GuiConst::STATE_FOREGROUNG_COLOR)) {
                        pac->setForeground(BaseStateAbstract::stringToColor(attributes.value(GuiConst::STATE_FOREGROUNG_COLOR)));
                    }
                    if (attributes.hasAttribute(GuiConst::STATE_BACKGROUNG_COLOR)) {
                        pac->setForeground(BaseStateAbstract::stringToColor(attributes.value(GuiConst::STATE_BACKGROUNG_COLOR)));
                    }

                    if (attributes.hasAttribute(GuiConst::STATE_INJECTED_PACKET)) {
                        bool ok = false;
                        int val = attributes.value(GuiConst::STATE_INJECTED_PACKET).toInt(&ok);
                        if (ok) {
                            pac->setInjected(val == 1);
                        }
                    }

                    QByteArray data;
                    QByteArray originalData;
                    while (stream->readNextStartElement()) { // should be some data here
                        QString name = stream->name().toString();

                        if (name == GuiConst::STATE_DATA) {
                            data = BaseStateAbstract::stringToByteArray(stream->readElementText(QXmlStreamReader::SkipChildElements));
                        } else if (name == GuiConst::STATE_ORIGINAL_DATA) {
                            originalData = BaseStateAbstract::stringToByteArray(stream->readElementText(QXmlStreamReader::SkipChildElements));
                        } else if (name == GuiConst::STATE_ADDITIONAL_FIELDS_LIST) {
                            QHash<QString, QString> fields;
                            while (stream->readNextStartElement()) { // ok reading the list now
                                fields.insert(stream->name().toString(), QString::fromUtf8(QByteArray::fromBase64(stream->readElementText(QXmlStreamReader::SkipChildElements).toUtf8())));
                            }
                            pac->setAdditionalFields(fields);
                        }

                    } // if end element , moving on
                    bool gotData = true;
                    if (data.isEmpty()) {
                        if (originalData.isEmpty()) {
                            qCritical() << tr("[ImportExportWorker::loadFromXML] No data read at all within the Packet record");
                            gotData = false;
                        } else {
                            pac->setOriginalData(originalData, true);
                        }
                    } else {
                        if (originalData.isEmpty())
                            pac->setData(data,true);
                        else {
                            pac->setData(data, false);
                            pac->setOriginalData(originalData,false);
                        }
                    }

                    // packet should be formed now
                    if (gotData)
                        emit newPacket(pac);
                    else
                        delete pac;

                } else {
                    qCritical() << tr("[ImportExportWorker::loadFromXML] Expecting a <%1> element here not <%2>").arg(GuiConst::STATE_PACKET).arg(stream->name().toString());
                }
            }
        } else {
            qCritical() << tr("[ImportExportWorker::loadFromXML] The first element is not %1 but %2").arg(GuiConst::STATE_PACKET_LIST).arg(stream->name().toString());
        }
    } else {
        qCritical() << tr("[ImportExportWorker::loadFromXML] Cannot read the first element: %1 | %2").arg(stream->tokenString()).arg(stream->tokenType());
    }
}

void ImportExportWorker::loadFromXMLFile(QIODevice *file)
{
    QXmlStreamReader stream(file);
    loadFromXML(&stream);
}

#if QT_VERSION >= 0x050000
void ImportExportWorker::toJSon(QJsonDocument *jdoc)
{
    Packet *pac = nextPacket();
    QJsonArray packetList;
    while (pac != nullptr ) {
        QJsonObject jsonobj;
        jsonobj[GuiConst::STATE_TIMESTAMP] = QString::number(pac->getTimestamp().toMSecsSinceEpoch());
        if (pac->getMicrosec() != 0)
            jsonobj[GuiConst::STATE_MICROSEC] = QString::number(pac->getMicrosec());
        if (pac->getDirection() != Packet::NODIRECTION)
            jsonobj[GuiConst::STATE_DIRECTION] = QString::number(pac->getDirection());
        if (!pac->getComment().isEmpty())
            jsonobj[GuiConst::STATE_COMMENT] = pac->getComment();
        if (!pac->getSourceString().isEmpty())
            jsonobj[GuiConst::STATE_SOURCE_STRING] = pac->getSourceString();
        if (pac->getForeground().isValid())
            jsonobj[GuiConst::STATE_FOREGROUNG_COLOR] = BaseStateAbstract::colorToString(pac->getForeground());
        if (pac->getBackground().isValid())
            jsonobj[GuiConst::STATE_BACKGROUNG_COLOR] = BaseStateAbstract::colorToString(pac->getBackground());

        jsonobj[GuiConst::STATE_DATA] = QString::fromUtf8(pac->getData().toBase64());
        if (pac->hasBeenModified())
            jsonobj[GuiConst::STATE_ORIGINAL_DATA] = QString::fromUtf8(pac->getOriginalData().toBase64());

        QHash<QString, QString> fields = pac->getAdditionalFields();
        if (fields.size() > 0) {
            QJsonObject fieldsobj;
            QHashIterator<QString, QString> i(fields);
             while (i.hasNext()) {
                 i.next();
                 fieldsobj[i.key()] = i.value();
             }

            jsonobj[GuiConst::STATE_ADDITIONAL_FIELDS_LIST] = fieldsobj;
        }

        packetList.append(jsonobj);
        pac = nextPacket();

    }

    jdoc->setArray(packetList);
}

void ImportExportWorker::toJSonFile(QIODevice *file)
{
    QJsonDocument jdoc;
    toJSon(&jdoc);
    file->write(jdoc.toJson(exportFormattedJson ? QJsonDocument::Indented : QJsonDocument::Compact));
}

void ImportExportWorker::loadFromJson(QJsonDocument *jdoc)
{
    if (jdoc->isArray()) {
        QJsonArray packetList = jdoc->array();
        for (int i = 0; i < packetList.size(); i ++ ) {
            QJsonValue val1 = packetList.at(i);
            if (!val1.isObject()) {
                qCritical() << tr("[ImportExportWorker::loadFromJson] One value of the packet list array is not a JSON object: %1").arg(i);
                continue;
            }
            QJsonObject jsonpac = packetList.at(i).toObject();

            Packet *pac = new(std::nothrow) Packet();
            if (pac == nullptr)
                qFatal("[ImportExportWorker::loadFromJson] Cannot allocate memory for Packet X{");

            bool ok = false;
            QDateTime dateTime;
            if (jsonpac.contains(GuiConst::STATE_TIMESTAMP)) {
                qint64 timems = jsonpac.value(GuiConst::STATE_TIMESTAMP).toString().toLongLong(&ok);
                if (!ok) {
                    qCritical() << tr("[ImportExportWorker::toXMLloadFromJson Cannot parse the timestamp: %1").arg(jsonpac.value(GuiConst::STATE_TIMESTAMP).toString());
                    pac->setTimestamp(dateTime);
                }
                else {
                    dateTime = QDateTime::fromMSecsSinceEpoch(timems);
                    pac->setTimestamp(dateTime);
                }
            } else {
                qCritical() << tr("[ImportExportWorker::loadFromJson] No timestamp attribute");
                pac->setTimestamp(dateTime);
            }

            if (jsonpac.contains(GuiConst::STATE_MICROSEC)) {
                quint32 microsec = jsonpac.value(GuiConst::STATE_MICROSEC).toString().toUInt(&ok);
                if (!ok) {
                    qCritical() << tr("[ImportExportWorker::loadFromJson] Cannot parse microseconds: %1").arg(jsonpac.value(GuiConst::STATE_MICROSEC).toString());
                }
                else {
                    pac->setMicrosec(microsec);
                }
            }

            if (jsonpac.contains(GuiConst::STATE_DIRECTION)) {
                qint32 direcInt = jsonpac.value(GuiConst::STATE_DIRECTION).toString().toInt(&ok);
                if (!ok || (direcInt != Packet::LEFTRIGHT && direcInt != Packet::RIGHTLEFT && direcInt != Packet::NODIRECTION)) {
                    qCritical() << tr("[ImportExportWorker::loadFromJson] Cannot parse direction: %1").arg(jsonpac.value(GuiConst::STATE_DIRECTION).toString());
                } else {
                    pac->setDirection((Packet::Direction)direcInt);
                }
            }

            if (jsonpac.contains(GuiConst::STATE_COMMENT)) {
                pac->setComment(jsonpac.value(GuiConst::STATE_COMMENT).toString());
            }

            if (jsonpac.contains(GuiConst::STATE_SOURCE_STRING)) {
                pac->setSourceString(jsonpac.value(GuiConst::STATE_SOURCE_STRING).toString());
            }
            if (jsonpac.contains(GuiConst::STATE_FOREGROUNG_COLOR)) {
                pac->setForeground(BaseStateAbstract::stringToColor(jsonpac.value(GuiConst::STATE_FOREGROUNG_COLOR).toString()));
            }
            if (jsonpac.contains(GuiConst::STATE_BACKGROUNG_COLOR)) {
                pac->setForeground(BaseStateAbstract::stringToColor(jsonpac.value(GuiConst::STATE_BACKGROUNG_COLOR).toString()));
            }
            QByteArray data;
            QByteArray originalData;

            if (jsonpac.contains(GuiConst::STATE_DATA)) {
                data = QByteArray::fromBase64(jsonpac.value(GuiConst::STATE_DATA).toString().toUtf8());
            }

            if (jsonpac.contains(GuiConst::STATE_ORIGINAL_DATA)) {
                originalData = QByteArray::fromBase64(jsonpac.value(GuiConst::STATE_ORIGINAL_DATA).toString().toUtf8());
            }

            if (jsonpac.contains(GuiConst::STATE_ADDITIONAL_FIELDS_LIST)) {
                QJsonValue val = jsonpac.value(GuiConst::STATE_ADDITIONAL_FIELDS_LIST);
                if (val.isObject()) {
                    QJsonObject jsonFields = jsonpac.value(GuiConst::STATE_ADDITIONAL_FIELDS_LIST).toObject();
                    QStringList keys = jsonFields.keys();
                    QHash<QString, QString> fields;
                    for (int j = 0; j < keys.size(); j++) {
                        fields.insert(keys.at(j), jsonFields.value(keys.at(j)).toString());
                    }
                    pac->setAdditionalFields(fields);
                } else {
                    qCritical() << tr("[ImportExportWorker::loadFromJson] The additional fields value is not a JSON object");
                }
            }

            bool gotData = true;
            if (data.isEmpty()) {
                if (originalData.isEmpty()) { // both data are empty, Aarg ...
                    qCritical() << tr("[ImportExportWorker::loadFromJson] No data read at all within the Packet record");
                    gotData = false;
                } else { // got original data
                    pac->setOriginalData(originalData, true);
                }
            } else {
                if (originalData.isEmpty()) // got data
                    pac->setData(data,true);
                else { // got both
                    pac->setData(data, false);
                    pac->setOriginalData(originalData,false);
                }
            }

            // packet should be formed now
            if (gotData)
                emit newPacket(pac);
            else
                delete pac;
        }
    } else {
        qCritical() << tr("[ImportExportWorker::loadFromJson] The base object has to be a JSON array");
    }
}

void ImportExportWorker::loadFromJsonFile(QIODevice *file)
{
    QByteArray fileData = file->readAll();
    QJsonParseError error;
    QJsonDocument jdoc = QJsonDocument::fromJson(fileData,&error);
    if (error.error == QJsonParseError::NoError) {
        loadFromJson(&jdoc);
    } else {
        qCritical() << tr("[ImportExportWorker::run] Json Parsing error:").arg(error.errorString());
    }
}
#endif

Packet *ImportExportWorker::nextPacket()
{
    if (noMore)
        return nullptr;

    currentIndex++;

    if (applyFilter) {
        if (currentIndex < (qint64)filteredList.size()) {
            return model->getPacket(filteredList.at((int)currentIndex));
        }
    } else { // entire list
        if (currentIndex < model->size()) {
            return model->getPacket(currentIndex);
        }
    }

    noMore = true;

    return nullptr;
}
bool ImportExportWorker::getExportFormattedJson() const
{
    return exportFormattedJson;
}

void ImportExportWorker::setExportFormattedJson(bool value)
{
    exportFormattedJson = value;
}

bool ImportExportWorker::getExportFormattedXML() const
{
    return exportFormattedXML;
}

void ImportExportWorker::setExportFormattedXML(bool value)
{
    exportFormattedXML = value;
}

QList<qint64> ImportExportWorker::getFilteredList() const
{
    return filteredList;
}

void ImportExportWorker::setFilteredList(const QList<qint64> &value)
{
    filteredList = value;
    applyFilter = !filteredList.isEmpty();
}


quint32 ImportExportWorker::getPcapLinkType() const
{
    return pcapLinkType;
}

void ImportExportWorker::setPcapLinkType(const PcapDef::Link_Type &value)
{
    pcapLinkType = value;
}




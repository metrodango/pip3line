#include "importexportworker.h"
#include <QFile>
#include <QDebug>
#include <QBuffer>
#include <QClipboard>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QApplication>
#include "pcapio/pcapio.h"
#include "packet.h"
#include "shared/guiconst.h"
#include "state/basestateabstract.h"

ImportExportWorker::ImportExportWorker(PacketModelAbstract *model,
                                       QString filename,
                                       GuiConst::FileOperations ops,
                                       GuiConst::FileFormat format,
                                       bool enableCompression,
                                       QObject *parent) :
    QObject(parent),
    ops(ops),
    format(format),
    filename(filename),
    model(model),
    enableCompression(enableCompression)
{
    pcapLinkType = PcapDef::LINKTYPE_ETHERNET; // ethernet
    exportFormattedXML = true;
    applyFilter = false;
    currentIndex = -1;
    noMore = false;
    plainBase64 = false;
    plainToFile = true;
    plainSeparator = '\n';

    connect(this, &ImportExportWorker::finished, this, &ImportExportWorker::deleteLater, Qt::QueuedConnection);
}

ImportExportWorker::~ImportExportWorker()
{
    //qDebug() << "Destroying" << this;
}

void ImportExportWorker::run()
{
    if (ops == GuiConst::UNKNOWN_OPERATION || format == GuiConst::INVALID_FORMAT) {
        qCritical() << tr("[ImportExportWorker::run] worker not configured properly to work on its own: %1 | %2 | %3").arg(filename).arg(ops).arg(format);
        return;
    }
    QIODevice *target = nullptr;

    if (format == GuiConst::PLAIN_FORMAT && !plainToFile) {
        target = new(std::nothrow) QBuffer();
    } else {
        if (filename.isEmpty()) {
            qCritical() << tr("[ImportExportWorker::run] worker is missing a filename T_T");
            return;
        }
        target = new(std::nothrow) QFile(filename);
    }

    if (target == nullptr) {
        qFatal("Cannot allocate memory for target(QIODevice) X{");
    }

    timer.restart();

    if (ops == GuiConst::EXPORT_OPERATION) {

        if (target->open(QIODevice::WriteOnly)) {
            switch (format) {
                case GuiConst::PCAP_FORMAT:
                    toPcap(target);
                    break;
                case GuiConst::XML_FORMAT:
                    toXMLFile(target);
                    break;
                case GuiConst::JSON_FORMAT:
                    toJSonFile(target);
                    break;
                case GuiConst::PLAIN_FORMAT:
                    toPlain(target);
                    break;
                default:
                    qCritical() << tr("[ImportExportWorker::run] Unmanaged format for writing: %1 T_T").arg(format);
                    break;
            }
        }

        if (format == GuiConst::PLAIN_FORMAT && !plainToFile) {
            QBuffer *buf = dynamic_cast<QBuffer *>(target);
            if (buf != nullptr) {
                QByteArray data  = buf->data();
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(QString::fromUtf8(data));
            } else {
                qCritical() << tr("[ImportExportWorker::run] invalid cast to QBuffer");
            }
        }

        qDebug() << "Written packets:" << loadedPackets.size() << timer.restart();

    } else if (ops == GuiConst::IMPORT_OPERATION) {
        loadedPackets.clear();
        if (format == GuiConst::PLAIN_FORMAT && !plainToFile) {
            QBuffer *buf = dynamic_cast<QBuffer *>(target);
            if (buf != nullptr) {
                QClipboard *clipboard = QApplication::clipboard();
                QByteArray data  = clipboard->text().toUtf8();
                buf->setData(data);
            } else {
                qCritical() << tr("[ImportExportWorker::run] invalid cast to QBuffer");
            }
        }
        if (target->open(QIODevice::ReadOnly)) {
            switch (format) {
                case GuiConst::PCAP_FORMAT:
                    loadFromPcap(target);
                    break;
                case GuiConst::XML_FORMAT:
                    loadFromXMLFile(target);
                    break;
                case GuiConst::JSON_FORMAT:
                    loadFromJsonFile(target);
                    break;
                case GuiConst::PLAIN_FORMAT:
                    loadFromPlain(target);
                    break;
                default:
                    qCritical() << tr("[ImportExportWorker::run] Unmanaged format for reading: %1 T_T").arg(format);
                    break;
            }
            qDebug() << "loaded packets:" << loadedPackets.size() << timer.restart();
            std::sort(loadedPackets.begin(), loadedPackets.end(), Packet::lessThanFunc);
            qDebug() << "Packets sorted " << timer.restart();
            model->addPackets(loadedPackets);
            qDebug() << "Packet added " << timer.restart();
        }
    }

    if (target->isOpen())
        target->close();

    target->deleteLater();


    emit finished();
}

void ImportExportWorker::toPcap(QIODevice *file)
{
    PcapIO *pfile = new(std::nothrow) PcapIO(file);
    if (pfile == nullptr)
        qFatal("Cannot allocate memory for PcapIO X{");

    pfile->setLinkLayerType(pcapLinkType);

    if (pfile->createAndOpenFile()) {
        QSharedPointer<Packet> pac = nextPacket();
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
        emit log("Cannot open the pcap destination file ", "Pcap export",Pip3lineConst::LERROR);
    }
}

void ImportExportWorker::loadFromPcap(QIODevice *file)
{
    // qDebug() << "perf 1: " << timer.restart();
    PcapIO *pfile = new(std::nothrow) PcapIO(file);
    if (pfile == nullptr)
        qFatal("Cannot allocate memory for PcapIO X{");

    if (pfile->openExistingFile(QIODevice::ReadOnly)) {
        PcapPacket * ppacket = pfile->nextPacket();
        qint64 count = 0;
        while (ppacket != nullptr) {
            count++;
            QSharedPointer<Packet> pac = QSharedPointer<Packet> (new(std::nothrow) Packet(ppacket->getData()));
            if (pac == nullptr)
                qFatal("Cannot allocate memory for Packet X{");

            if (static_cast<quint32>(ppacket->getData().size()) < ppacket->getOriginalSize()) {
                qWarning() << tr("[Pcap] Packet %1 appears to have been truncated").arg(count);
            }
            pac->setTimestamp(ppacket->getTimestamp());
            pac->setMicrosec(ppacket->getMicrosec() % 1000);

            loadedPackets.append(pac);

            delete ppacket;
            ppacket = pfile->nextPacket();
        }
    } else {
        emit log(tr("Cannot open the pcap source file: %1").arg(pfile->getErrorString()), "Pcap import",Pip3lineConst::LERROR);
    }

    delete pfile;

    //qDebug() << "perf 10: " << timer.restart();
}

void ImportExportWorker::toXML(QXmlStreamWriter *stream)
{
    stream->writeStartElement(GuiConst::STATE_PACKET_LIST);
    QSharedPointer<Packet> pac = nextPacket();
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

        stream->writeAttribute(GuiConst::STATE_SOURCEID, QString::number(pac->getSourceid()));
        stream->writeTextElement(GuiConst::STATE_DATA, BaseStateAbstract::byteArrayToString(pac->getData(),enableCompression));
        if (pac->hasBeenModified())
            stream->writeTextElement(GuiConst::STATE_ORIGINAL_DATA, BaseStateAbstract::byteArrayToString(pac->getOriginalData(),enableCompression));
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
    stream.writeStartDocument();
    toXML(&stream);
    stream.writeEndDocument();
}

void ImportExportWorker::loadFromXML(QXmlStreamReader *stream)
{
    if (stream->name() == GuiConst::STATE_PACKET_LIST || stream->readNextStartElement()) {
        if (stream->name() == GuiConst::STATE_PACKET_LIST) {
            while (stream->readNextStartElement()) {
                if (stream->name() == GuiConst::STATE_PACKET) {
                    QSharedPointer<Packet> pac = QSharedPointer<Packet> (new(std::nothrow) Packet());
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
                            pac->setDirection(static_cast<Packet::Direction>(direcInt));
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
                        pac->setBackground(BaseStateAbstract::stringToColor(attributes.value(GuiConst::STATE_BACKGROUNG_COLOR)));
                    }

                    if (attributes.hasAttribute(GuiConst::STATE_INJECTED_PACKET)) {
                        bool ok = false;
                        int val = attributes.value(GuiConst::STATE_INJECTED_PACKET).toString().toInt(&ok);
                        if (ok) {
                            pac->setInjected(val == 1);
                        }
                    }

                    if (attributes.hasAttribute(GuiConst::STATE_SOURCEID)) {
                        bool ok = false;
                        int val = attributes.value(GuiConst::STATE_SOURCEID).toString().toInt(&ok);
                        if (ok) {
                            pac->setSourceid(val);
                        }
                    }

                    QByteArray data;
                    QByteArray originalData;
                    while (stream->readNextStartElement()) { // should be some data here
                        QString name = stream->name().toString();

                        if (name == GuiConst::STATE_DATA) {
                            data = BaseStateAbstract::stringToByteArray(stream->readElementText(QXmlStreamReader::SkipChildElements),enableCompression);
                        } else if (name == GuiConst::STATE_ORIGINAL_DATA) {
                            originalData = BaseStateAbstract::stringToByteArray(stream->readElementText(QXmlStreamReader::SkipChildElements),enableCompression);
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
                        loadedPackets.append(pac);
                    else
                        pac.clear();

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
    if (stream.readNext() != QXmlStreamReader::StartDocument) {
        qCritical() << tr("not XML document starter .. skipping");
        return;
    }
    loadFromXML(&stream);
}

void ImportExportWorker::toJSon(QJsonDocument *jdoc)
{
    QSharedPointer<Packet> pac = nextPacket();
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

        jsonobj[GuiConst::STATE_SOURCEID] = QString::number(pac->getSourceid());
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

            QSharedPointer<Packet> pac = QSharedPointer<Packet>(new(std::nothrow) Packet());
            if (pac.isNull())
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
                    pac->setDirection(static_cast<Packet::Direction>(direcInt));
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
                pac->setBackground(BaseStateAbstract::stringToColor(jsonpac.value(GuiConst::STATE_BACKGROUNG_COLOR).toString()));
            }

            if (jsonpac.contains(GuiConst::STATE_SOURCEID)) {
                int value = jsonpac.value(GuiConst::STATE_SOURCEID).toString().toInt(&ok);
                if (ok)
                    pac->setSourceid(value);
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
                loadedPackets.append(pac);
            else
                pac.clear();
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

void ImportExportWorker::toPlain(QIODevice *file)
{
    QByteArray endChar(1,plainSeparator);
    QSharedPointer<Packet> pac = nextPacket();
    while (pac != nullptr ) {
        QByteArray data = pac->getData();
        if (plainBase64) {
            data = data.toBase64();
        }
        file->write(data);
        file->write(endChar);

        pac = nextPacket();
    }
}

void ImportExportWorker::loadFromPlain(QIODevice *file)
{
    QByteArray data;
    QByteArray prevData;
    QDateTime date = QDateTime::currentDateTime();
    while (!file->atEnd()) {
        data = file->read(GEN_BLOCK_SIZE);
        QList<QByteArray> list = data.split(plainSeparator);
        for (int i = 0; i < list.size() - 1; i++) {
            prevData.append(list.at(i));
            if (!prevData.isEmpty()) {
                addPlainRawPacket(prevData, date);
                prevData.clear();
            }
            date = date.addSecs(1); // arbitrary, so that the packets are sorted in the list order
        }

        prevData = list.last();
        if (!prevData.isEmpty() && data.endsWith(plainSeparator)) { // rare scenario
            addPlainRawPacket(prevData, date);
            prevData.clear();
            date = date.addSecs(1);
        }
    }

    if (!prevData.isEmpty()) { // the last block did not have the separator
        date = date.addSecs(1);
        addPlainRawPacket(prevData, date);
    }

}

QSharedPointer<Packet> ImportExportWorker::nextPacket()
{
    QSharedPointer<Packet> ret;
    if (noMore)
        return ret;

    currentIndex++;

    if (applyFilter) {
        if (currentIndex < static_cast<qint64>(filteredList.size())) {
            return model->getPacket(filteredList.at(static_cast<int>(currentIndex)));
        }
    } else { // entire list
        if (currentIndex < model->size()) {
            return model->getPacket(currentIndex);
        }
    }

    noMore = true;

    return ret;
}

void ImportExportWorker::addPlainRawPacket(QByteArray data, QDateTime date)
{
    if (data.isEmpty()) {
        qCritical() << tr("[ImportExportWorker::addPlainRawPacket] data is empty, ignoring");
        return;
    }

    if (plainBase64) {
        data = QByteArray::fromBase64(data);
        if (data.isEmpty()) {
            qCritical() << tr("[ImportExportWorker::addPlainRawPacket] Decoded data is empty, ignoring");
            return;
        }

    }

    QSharedPointer<Packet> pac = QSharedPointer<Packet> (new(std::nothrow) Packet());
    if (pac == nullptr)
        qFatal("[ImportExportWorker::addPlainRawPacket] Cannot allocate memory for Packet X{");
    pac->setOriginalData(data,true);
    pac->setTimestamp(date);
    loadedPackets.append(pac);
}

void ImportExportWorker::setPlainToFile(bool value)
{
    plainToFile = value;
}

void ImportExportWorker::setPlainBase64(bool value)
{
    plainBase64 = value;
}

QList<QSharedPointer<Packet> > ImportExportWorker::getLoadedPackets() const
{
    return loadedPackets;
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




#ifndef IMPORTEXPORTWORKER_H
#define IMPORTEXPORTWORKER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QIODevice>
#include <QTime>
#include "shared/guiconst.h"
#include "pcapio/pcapdef.h"
#include "packetmodelabstract.h"

class Packet;

class ImportExportWorker : public QObject
{
        Q_OBJECT
    public:
        explicit ImportExportWorker(PacketModelAbstract *model,
                                    QString filename = QString(),
                                    GuiConst::FileOperations ops = GuiConst::UNKNOWN_OPERATION,
                                    GuiConst::FileFormat format = GuiConst::INVALID_FORMAT,
                                    bool enableCompression = true,
                                    QObject *parent = 0);
        ~ImportExportWorker();
        quint32 getPcapLinkType() const;
        void setPcapLinkType(const PcapDef::Link_Type &value);

        QList<qint64> getFilteredList() const;
        void setFilteredList(const QList<qint64> &value);
        bool getExportFormattedXML() const;
        void setExportFormattedXML(bool value);
        bool getExportFormattedJson() const;
        void setExportFormattedJson(bool value);

        void toPcap(QIODevice *file);
        void loadFromPcap(QIODevice *file);
        void toXML(QXmlStreamWriter *stream);
        void toXMLFile(QIODevice *file);
        void loadFromXML(QXmlStreamReader *stream);
        void loadFromXMLFile(QIODevice *file);
        void toJSon(QJsonDocument *jdoc);
        void toJSonFile(QIODevice *file);
        void loadFromJson(QJsonDocument *jdoc);
        void loadFromJsonFile(QIODevice *file);
        void toPlain(QIODevice *file);
        void loadFromPlain(QIODevice *file);

        void setPlainBase64(bool value);
        void setPlainToFile(bool value);

        QList<QSharedPointer<Packet> > getLoadedPackets() const;

    public slots:
        void run();
    signals:
        void finished();
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
    private:
        QSharedPointer<Packet> nextPacket();
        void addPlainRawPacket(QByteArray data, QDateTime date = QDateTime::currentDateTime());
        GuiConst::FileOperations ops;
        GuiConst::FileFormat format;
        QString filename;
        PacketModelAbstract *model;
        PcapDef::Link_Type pcapLinkType;
        QList<qint64> filteredList;
        bool applyFilter;
        qint64 currentIndex;
        bool noMore;
        bool exportFormattedXML;
        bool exportFormattedJson;
        bool enableCompression;
        QTime timer;
        QList<QSharedPointer<Packet> > loadedPackets;
        bool plainBase64;
        bool plainToFile;
        char plainSeparator;
};

#endif // IMPORTEXPORTWORKER_H

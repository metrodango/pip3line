#ifndef IMPORTEXPORTWORKER_H
#define IMPORTEXPORTWORKER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#endif
#include <QIODevice>
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
#if QT_VERSION >= 0x050000
        void toJSon(QJsonDocument *jdoc);
        void toJSonFile(QIODevice *file);
        void loadFromJson(QJsonDocument *jdoc);
        void loadFromJsonFile(QIODevice *file);
#endif
    public slots:
        void run();
    signals:
        void newPacket(Packet * packet);
        void finished();
    private:
        Packet *nextPacket();
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
};

#endif // IMPORTEXPORTWORKER_H

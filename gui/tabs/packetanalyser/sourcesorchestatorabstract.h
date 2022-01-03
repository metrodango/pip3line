﻿#ifndef SOURCESORCHESTATORABSTRACT_H
#define SOURCESORCHESTATORABSTRACT_H

#include <QObject>
#include <QWidget>
#include <commonstrings.h>
#include "sources/blocksources/blockssource.h"
#include "sources/blocksources/target.h"
#include <pipelinecommon.h>
using namespace Pip3lineCommon;

class SourcesOrchestatorAbstract : public QObject
{
        Q_OBJECT
    public:
        static const QStringList OrchestratorsList;
        enum OrchestratorType {
            INVALID_TYPE = 0,
            TCP_CLIENT = 1,
            UDP_CLIENT = 2,
            TCP_SERVER = 3,
            UDP_SERVER = 4,
            TCP_PROXY = 5,
            UDP_PROXY = 6,
            BLOCKS_EXTERNAL_PROXY_TCP = 7,
            BLOCKS_EXTERNAL_PROXY_UDP = 8,
            SOCKS5_PROXY = 9,
            PIPE_CLIENT = 10,
            MYO_PROXY = 11,
            SHARED_MEM = 12,
            FILE_SOURCE = 13,
            DTLS_CLIENT = 14,
            DTLS_SERVER = 15,
            DTLS_PROXY = 16
        };

        explicit SourcesOrchestatorAbstract(QObject *parent = nullptr);
        virtual ~SourcesOrchestatorAbstract();
        bool isForwarder() const;
        QString getName() const;
        void setName(const QString &value);
        QString getDescription() const;
        void setDescription(const QString &value);
        virtual QList<Target<SourcesOrchestatorAbstract *> > getAvailableConnections() = 0;
        OrchestratorType getType() const;
        void setType(const OrchestratorType &value);
        virtual BlocksSource * getBlockSource(int index) = 0;
        virtual int blockSourceCount() const = 0;
        QWidget * getControlGui(QWidget *parent = nullptr);
        QWidget * getConfGui(QWidget *parent = nullptr);
        bool getHasDirection() const;
        virtual QHash<QString, QString> getConfiguration();
        virtual void setConfiguration(QHash<QString, QString> conf);
        virtual bool isTrackingChanges();
    public slots:
        virtual void postPacket(QSharedPointer<Packet> packet);
        virtual bool start();
        virtual void stop();
        virtual bool restart();
    signals:
        void newPacket(QSharedPointer<Packet> packet);
        void packetInjected(QSharedPointer<Packet> packet);
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
        void queueUpdated(int size);
        void forwardingChanged(bool forwarding);
        void started();
        void stopped();
        void connectionsChanged();
        void startChildren();
        void stopChildren();
        void resetChildren();
        void guiRequested();
    protected slots:
        virtual void onBlockReceived(Block *block);
        virtual void onControlGuiDestroyed();
        virtual void onConfigGuiDestroyed();
        void sourceReflexionChanged(const bool &value);
    protected:
        virtual Target<SourcesOrchestatorAbstract *> toOrchestratorTarget(Target<BlocksSource *> bst);
        virtual QList<Target<SourcesOrchestatorAbstract *> > toOrchestratorTargetList(QList<Target<BlocksSource *> > ilist);
        virtual QWidget * requestControlGui(QWidget *parent);
        virtual QWidget * requestConfGui(QWidget *parent);
        bool forwarder;
        bool hasDirection;
        QString name;
        QString description;
        QWidget *confgui{nullptr};
        QWidget *controlGui{nullptr};
        OrchestratorType type;
    private:
        Q_DISABLE_COPY(SourcesOrchestatorAbstract)
        static QStringList initSourcesStrings();
};

#endif // SOURCESORCHESTATORABSTRACT_H

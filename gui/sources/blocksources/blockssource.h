/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef BLOCKSSOURCE_H
#define BLOCKSSOURCE_H

#include <QThread>
#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <QHash>
#include <QDateTime>
#include <transformabstract.h>
#include "target.h"
#include <QReadWriteLock>
#include <QTimer>
#include <commonstrings.h>
#include <pipelinecommon.h>
using namespace Pip3lineCommon;

class QWidget;

class BlocksSource : public QObject
{
        Q_OBJECT
    public:
        enum BSOURCETYPE {
            CLIENT        = 0x0001,
            SERVER        = 0x0002,
            INVALID_TYPE  = 0x0010
        };

        enum Flags: unsigned long {
            TLS_OPTIONS         = 0x1,
            TLS_ENABLED         = 0x2,
            REFLEXION_OPTIONS   = 0x20,
            REFLEXION_ENABLED   = 0x40,
            READ_ONLY           = 0x80,
            B64BLOCKS_OPTIONS   = 0x100,
            B64BLOCKS_ENABLED   = 0x200,
            GEN_IP_OPTIONS      = 0x400,
            WANTS_TRACKING      = 0x800
        };

        static const QString NEW_CONNECTION_STRING;
        static const QString NEXT_CLIENT_STRING;
        static const int DEFAULT_B64_BLOCK_MAX_SIZE;
        static int newSourceID(BlocksSource * source = nullptr);
        static void releaseID(int id);
        static BlocksSource * getSourceObject(int id);

        explicit BlocksSource(QObject *parent = nullptr);
        virtual ~BlocksSource() override;
        QWidget *getGui(QWidget * parent = nullptr);
        virtual QWidget *getAdditionnalCtrls(QWidget * parent = nullptr);
        bool isReadWrite();
        virtual void postBlockForSending(Block *block);
        bool isB64Blocks() const;
        BSOURCETYPE getType() const;
        virtual QString getName() = 0;
        virtual QString getDescription() = 0;
        virtual QList<Target<BlocksSource *> > getAvailableConnections();
        int getSid() const;
        bool isTLSEnabled() const;
        bool isReflexive() const;
        quint64 getFlags() const;
        void setFlags(const quint64 &value);
        virtual bool isStarted() = 0;
        virtual QHash<QString, QString> getConfiguration();
        virtual void setConfiguration(const  QHash<QString, QString> &conf);
        int getB64MaxBlockLength() const;
        char getB64BlocksSeparator() const;
        TransformAbstract *getInboundTranform() const;
        void setInboundTranform(TransformAbstract *transform);
        TransformAbstract *getOutboundTranform() const;
        void setOutboundTranform(TransformAbstract *transform);
        virtual int getTargetIdFor(int sourceId);
    signals:
        void blockReceived(Block *block);
        void blockToBeSend(Block *block);
        void stopped();
        void started();
        void reflexionChanged(bool reflexive);
        void sslChanged(bool enable);
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
        void updated();
        void newConnection(BlocksSource *);
        void connectionClosed(int cid);
        void inboundTransformModfied();
        void outboundTranformModfied();
        void inboundTranformSelectionRequested();
        void outboundTranformSelectionRequested();
    public slots:
        virtual void sendBlock(Block * block);
        virtual bool startListening();
        virtual void stopListening() = 0;
        virtual void restart();
        void setTlsEnable(bool enabled);
        void setReflexive(bool enabled);
        void setB64Blocks(bool enabled);
        void setB64MaxBlockLength(int value);
        void setB64BlocksSeparator(char value);
        virtual void onConnectionClosed(int cid);
    protected:
        void b64DecodeAndEmit(QByteArray data, int rsid);
        virtual QWidget *requestGui(QWidget * parent);
        void processIncomingB64Block(QByteArray data, int rsid);
        QByteArray applyInboundTransform(QByteArray data);
        QByteArray applyOutboundTransform(QByteArray data);

        void updateConnectionsInfo();
        virtual void internalUpdateConnectionsInfo();
        QList<Target<BlocksSource *>> connectionsInfo;
        QTimer updateConnectionsTimer;

        char b64BlocksSeparator;
        int b64MaxBlockLength;
        quint64 flags;
        TransformAbstract * inboundTranform;
        TransformAbstract * outboundTranform;
        BSOURCETYPE type; // should not need to save that
        int sid; // don't save that, that's a transient number
        QHash<int,QByteArray> b64BlockTempDataList; // don't save that, temp data
    protected slots:
        void triggerUpdate();
    private slots:
        void onGuiDestroyed();
    private:
        Q_DISABLE_COPY(BlocksSource)
        QReadWriteLock infoLocker;
        QWidget * gui;
        static int currentid;
        static QHash<int,BlocksSource *> idSourceTable;
        static QMutex idlock;
        QTime initialTime;

};

#endif // BLOCKSSOURCE_H

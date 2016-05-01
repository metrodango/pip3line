/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
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
#include <commonstrings.h>

class QWidget;

class Block {
    public:
        static const int INVALID_ID;
        explicit Block(QByteArray data, int sourceid);
        ~Block();
        QByteArray getData() const;
        void setData(const QByteArray &value);
        int getSourceid() const;
        void setSourceid(int value);
    private:
        QByteArray data;
        int sourceid;
};

class BlocksSource : public QObject
{
        Q_OBJECT
    public:
        enum BSOURCETYPE {CLIENT          = 0x0001,
                          SERVER          = 0x0002,
                          INVALID_TYPE = 0x0010};

        enum Flags {
            TLS_OPTIONS         = 0x1,
            TLS_ENABLED         = 0x2,
            REFLEXION_OPTIONS   = 0x20,
            REFLEXION_ENABLED   = 0x40,
            READ_ONLY           = 0x80,
            B64BLOCKS_OPTIONS   = 0x100,
            B64BLOCKS_ENABLED   = 0x200
        };

        static const QString NEW_CONNECTION_STRING;
        static const QString NEXT_CLIENT_STRING;
        static const int DEFAULT_B64_BLOCK_MAX_SIZE;
        static int newSourceID(BlocksSource * source = nullptr);
        static void releaseID(int id);
        static BlocksSource * getSourceObject(int id);

        explicit BlocksSource(QObject *parent = 0);
        virtual ~BlocksSource();
        QWidget *getGui(QWidget * parent = 0);
        bool isReadWrite();
        virtual void postBlockForSending(Block *block);
        bool isB64Blocks() const;
        BSOURCETYPE getType() const;
        virtual QString getName() = 0;
        virtual QString getDescription() = 0;
        virtual QList<Target<BlocksSource *> > getAvailableConnections();
        int getSid() const;
        bool isTLSEnable() const;
        bool isReflexive() const;
        quint64 getFlags() const;
        void setFlags(const quint64 &value);
        virtual bool isStarted() = 0;
        virtual QHash<QString, QString> getConfiguration();
        virtual void setConfiguration(QHash<QString, QString> conf);
        int getB64MaxBlockLength() const;
        char getB64BlocksSeparator() const;

        TransformAbstract *getInboundTranform() const;
        void setInboundTranform(TransformAbstract *transform);
        TransformAbstract *getOutboundTranform() const;
        void setOutboundTranform(TransformAbstract *transform);

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
        void connectionClosed(BlocksSource *);
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
    protected:
        void b64DecodeAndEmit(QByteArray data);
        virtual QWidget *requestGui(QWidget * parent);
        void processIncomingB64Block(QByteArray data);
        QByteArray applyInboundTransform(QByteArray data);
        QByteArray applyOutboundTransform(QByteArray data);

        char b64BlocksSeparator;
        int b64MaxBlockLength;
        quint64 flags;
        TransformAbstract * inboundTranform;
        TransformAbstract * outboundTranform;
        BSOURCETYPE type; // should not need to save that
        int sid; // don't save that, that's a transient number
        QByteArray b64BlockTempData; // don't save that, temp data

    private slots:
        void onGuiDestroyed();
    private:
        QWidget * gui;
        static int currentid;
        static QHash<int,BlocksSource *> idSourceTable;
        static QMutex idlock;
};

#endif // BLOCKSSOURCE_H

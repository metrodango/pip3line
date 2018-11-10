#ifndef EXTERNALPROXYORCHESTRATOR_H
#define EXTERNALPROXYORCHESTRATOR_H

#include "sourcesorchestatorabstract.h"

class ExternalProxyOrchestrator : public SourcesOrchestatorAbstract
{
        Q_OBJECT
    public:
        explicit ExternalProxyOrchestrator(BlocksSource *inboundSource, BlocksSource *outboundSource, QObject *parent = nullptr);
        ~ExternalProxyOrchestrator();
        QList<Target<SourcesOrchestatorAbstract *> > getAvailableConnections();
        BlocksSource * getBlockSource(int index);
        int blockSourceCount() const;
    public slots:
        void postPacket(QSharedPointer<Packet> packet);
        bool start();
    private slots:
        void onBlockReceived(Block *block);
        void onInboundBlockSourceDestroyed();
        void onOutboundBlockSourceDestroyed();
        void checkForwarder();
    private:
        Q_DISABLE_COPY(ExternalProxyOrchestrator)
        Target<SourcesOrchestatorAbstract *> toOrchestratorTarget(Target<BlocksSource *> bst);
        BlocksSource * inboundSource{nullptr};
        BlocksSource * outboundSource{nullptr};
};

#endif // EXTERNALPROXYORCHESTRATOR_H

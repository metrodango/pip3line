#ifndef PROXYORCHESTRATOR_H
#define PROXYORCHESTRATOR_H

#include "sourcesorchestatorabstract.h"

class ProxyOrchestrator : public SourcesOrchestatorAbstract
{
        Q_OBJECT
    public:
        explicit ProxyOrchestrator(BlocksSource *serverSource, BlocksSource *clientSource, QObject *parent = 0);
        virtual ~ProxyOrchestrator();
        QList<Target<SourcesOrchestatorAbstract *> > getAvailableConnections();
        BlocksSource * getBlockSource(int index);
        int blockSourceCount() const;
    public slots:
        void postPacket(Packet * packet);
        bool start();
    private slots:
        void onBlockReceived(Block *block);
        void onServerSourceDestroyed();
        void onClientSourceDestroyed();
    private:
        BlocksSource * serverSource{nullptr};
        BlocksSource * clientSource{nullptr};
};

#endif // PROXYORCHESTRATOR_H

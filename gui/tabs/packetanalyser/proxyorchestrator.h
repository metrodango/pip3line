#ifndef PROXYORCHESTRATOR_H
#define PROXYORCHESTRATOR_H

#include "sourcesorchestatorabstract.h"

class ProxyOrchestrator : public SourcesOrchestatorAbstract
{
        Q_OBJECT
    public:
        explicit ProxyOrchestrator(BlocksSource *serverSource, BlocksSource *clientSource, QObject *parent = nullptr);
        virtual ~ProxyOrchestrator() override;
        QList<Target<SourcesOrchestatorAbstract *> > getAvailableConnections() override;
        BlocksSource * getBlockSource(int index) override;
        int blockSourceCount() const override;
    public slots:
        void postPacket(QSharedPointer<Packet> packet) override;
        bool start() override;
    private slots:
        void onBlockReceived(Block *block) override;
        void onServerSourceDestroyed();
        void onClientSourceDestroyed();
    private:
        Q_DISABLE_COPY(ProxyOrchestrator)
        BlocksSource * serverSource{nullptr};
        BlocksSource * clientSource{nullptr};
};

#endif // PROXYORCHESTRATOR_H

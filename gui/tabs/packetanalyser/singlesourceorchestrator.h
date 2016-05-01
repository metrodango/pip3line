#ifndef SINGLESOURCEORCHESTRATOR_H
#define SINGLESOURCEORCHESTRATOR_H

#include "sourcesorchestatorabstract.h"

class BlocksSource;

class SingleSourceOrchestrator : public SourcesOrchestatorAbstract
{
        Q_OBJECT
    public:
        SingleSourceOrchestrator(BlocksSource *source, QObject *parent = 0);
        ~SingleSourceOrchestrator();
        QList<Target<SourcesOrchestatorAbstract *> > getAvailableConnections();
        BlocksSource * getBlockSource(int index);
        int blockSourceCount() const;
    public slots:
        void postPacket(Packet * packet);
        bool start();
    private slots:
        void onBlockSourceDestroyed();
    private:
        BlocksSource * source{nullptr};
};

#endif // SINGLESOURCEORCHESTRATOR_H

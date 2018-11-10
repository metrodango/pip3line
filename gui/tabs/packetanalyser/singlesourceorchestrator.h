#ifndef SINGLESOURCEORCHESTRATOR_H
#define SINGLESOURCEORCHESTRATOR_H

#include "sourcesorchestatorabstract.h"

class BlocksSource;

class SingleSourceOrchestrator : public SourcesOrchestatorAbstract
{
        Q_OBJECT
    public:
        SingleSourceOrchestrator(BlocksSource *source, QObject *parent = nullptr);
        ~SingleSourceOrchestrator() override;
        QList<Target<SourcesOrchestatorAbstract *> > getAvailableConnections() override;
        BlocksSource * getBlockSource(int index) override;
        int blockSourceCount() const override;
        bool isTrackingChanges() override;
    public slots:
        void postPacket(QSharedPointer<Packet> packet) override;
        bool start() override;
    private slots:
        void onBlockSourceDestroyed();
    private:
        Q_DISABLE_COPY(SingleSourceOrchestrator)
        QWidget * requestControlGui(QWidget *parent) override;
        BlocksSource * source{nullptr};        
};

#endif // SINGLESOURCEORCHESTRATOR_H

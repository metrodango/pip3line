#ifndef SOCKSORCHESTRATOR_H
#define SOCKSORCHESTRATOR_H

#include "proxyorchestrator.h"
#include "sources/blocksources/tlsserverlistener.h"

class SocksOrchestrator : public ProxyOrchestrator
{
        Q_OBJECT
    public:
        explicit SocksOrchestrator(BlocksSource *serverSource, BlocksSource *clientSource);
        ~SocksOrchestrator();

    private:
        QWidget * requestConfGui(QWidget *parent);
        QList<quint16> tlsPorts;

};

#endif // SOCKSORCHESTRATOR_H

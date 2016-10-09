#include "socksorchestrator.h"

SocksOrchestrator::SocksOrchestrator(BlocksSource *serverSource, BlocksSource *clientSource) :
    ProxyOrchestrator(serverSource, clientSource)
{

}

SocksOrchestrator::~SocksOrchestrator()
{

}

QWidget *SocksOrchestrator::requestConfGui(QWidget *parent)
{
    return ProxyOrchestrator::requestConfGui(parent);
}

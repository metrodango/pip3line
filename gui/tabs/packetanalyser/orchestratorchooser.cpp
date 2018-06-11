#include "orchestratorchooser.h"
#include "sources/blocksources/blockssource.h"
#include "singlesourceorchestrator.h"
#include "sourcesorchestatorabstract.h"
#include <QDebug>
#include <QDialog>
#include <QHBoxLayout>
#include <QStylePainter>
#include "shared/guiconst.h"
#include <QStandardItemModel>
#include "sources/blocksources/udpserverlistener.h"
#include "sources/blocksources/udpclientlistener.h"
#include "sources/blocksources/tlsserverlistener.h"
#include "sources/blocksources/tlsclientlistener.h"
#include "guihelper.h"
#include "shared/defaultdialog.h"
#include "externalproxyorchestrator.h"
#include "proxyorchestrator.h"
#include "socksorchestrator.h"
#include "sources/blocksources/pipeclientlistener.h"

OrchestratorChooser::OrchestratorChooser(GuiHelper *guiHelper, SourcesOrchestatorAbstract *orchestrator, QWidget *parent) :
    QComboBox(parent),
    orchestrator(orchestrator),
    guiHelper(guiHelper)
{

    addItem(tr("Select IO type"));
    QStandardItem * item = qobject_cast<QStandardItemModel *>(model())->item(0); // there is one and only one item right now
    item->setEnabled( false );
    addItems(SourcesOrchestatorAbstract::OrchestratorsList);

    setToolTip(tr("Select a configuration"));

    if (orchestrator != nullptr) {
        setCurrentIndex(orchestrator->getType());
    }

    //connect(this, qOverload<int>(&OrchestratorChooser::currentIndexChanged), this, &OrchestratorChooser::onSelection);
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelection(int)));
}

OrchestratorChooser::~OrchestratorChooser()
{
    if (orchestrator != nullptr) {
        orchestrator->stop();
        orchestrator->deleteLater();
        orchestrator = nullptr;
    }
}

void OrchestratorChooser::setType(int type)
{
    if (type > count() || type < 0) {
        qWarning() << tr("[OrchestratorChooser::setType] invalid type, ignoring");
    } else {
        orchestrator = createOrchestratorFromType(type);
        if (orchestrator != nullptr) {
            blockSignals(true);
            setCurrentIndex(type);
            blockSignals(false);
            emit newOrchestrator(orchestrator);
        } else {
            blockSignals(true);
            setCurrentIndex(0);
            blockSignals(false);
        }
    }
}

SourcesOrchestatorAbstract *OrchestratorChooser::getOrchestrator()
{
    return orchestrator;
}

int OrchestratorChooser::showConfPanel(bool blocking)
{
    int ret = QDialog::Rejected;
    if (orchestrator != nullptr) {
        DefaultDialog *confDialog = new(std::nothrow) DefaultDialog(this);
        if (confDialog == nullptr) {
            qFatal("Cannot allocate memory for DefaultDialog X{");
        }

        confDialog->setWindowTitle(tr("Configuration"));
        confDialog->resize(400,300);

        QWidget * confWidget = orchestrator->getConfGui(nullptr);

        if (confWidget != nullptr) {
            confDialog->setMainWidget(confWidget);
        }
        if (blocking)
            ret = confDialog->exec();
        else {
            confDialog->raise();
            confDialog->show();
        }
    } else {
        qCritical() << tr("[OrchestratorChooser::showConfPanel] orchestrator is nullptr T_T");
    }

    return ret;
}

void OrchestratorChooser::onOrchestratorDeleted()
{
    orchestrator = nullptr;
}

void OrchestratorChooser::onSelection(int index)
{
    if (orchestrator != nullptr) {
        // need to disconnect due to the deferred deletion
        disconnect(destructorLink);
        orchestrator->stop();
        orchestrator->deleteLater();
        orchestrator = nullptr;
    }
    orchestrator = createOrchestratorFromType(index);
    if (orchestrator != nullptr) {
        if (QDialog::Accepted == showConfPanel(true)) {
            emit newOrchestrator(orchestrator);
        } else {
            blockSignals(true);
            setCurrentIndex(0); // resetting to nothing
            blockSignals(false);
            delete orchestrator;
        }
    }
}

SourcesOrchestatorAbstract *OrchestratorChooser::createOrchestratorFromType(int type)
{
    SourcesOrchestatorAbstract * orch = nullptr;
    BlocksSource *bs = nullptr;
    switch (type) {
        case SourcesOrchestatorAbstract::INVALID_TYPE:
            qDebug() << "invalid orchestrator type choosen";
            break;
        case SourcesOrchestatorAbstract::TCP_CLIENT:
            {
                qDebug() << "TCP client choosen";
                bs = new(std::nothrow) TLSClientListener();
                if (bs == nullptr) {
                    qFatal("Cannot allocate memory for RawTcpListener X{");
                }

                bs->setFlags(BlocksSource::REFLEXION_OPTIONS |
                             BlocksSource::TLS_OPTIONS |
                             BlocksSource::TLS_ENABLED |
                             BlocksSource::B64BLOCKS_OPTIONS |
                             BlocksSource::IP_OPTIONS);

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }

                ci->setType(SourcesOrchestatorAbstract::TCP_CLIENT);
                orch = ci;
            }
            break;
        case SourcesOrchestatorAbstract::UDP_CLIENT:
        {
            qDebug() << "UDP client choosen";
            bs = new(std::nothrow) UdpClientListener();
            if (bs == nullptr) {
                qFatal("Cannot allocate memory for UdpClientListener X{");
            }

            bs->setFlags(BlocksSource::REFLEXION_OPTIONS | BlocksSource::IP_OPTIONS);

            SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
            if (ci == nullptr) {
                qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
            }

            ci->setType(SourcesOrchestatorAbstract::UDP_CLIENT);
            orch = ci;

        }
            break;
        case SourcesOrchestatorAbstract::UDP_SERVER:
        {
            qDebug() << "UDP server choosen";
            bs = new(std::nothrow) UdpServerListener();
            if (bs == nullptr) {
                qFatal("Cannot allocate memory for UdpServerListener X{");
            }

            bs->setFlags(BlocksSource::REFLEXION_OPTIONS);

            SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
            if (ci == nullptr) {
                qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
            }
            ci->setType(SourcesOrchestatorAbstract::UDP_SERVER);
            orch = ci;
        }
            break;
        case SourcesOrchestatorAbstract::TCP_SERVER:
            {
                qDebug() << "TCP server choosen";
                bs = new(std::nothrow) TLSServerListener();
                if (bs == nullptr) {
                    qFatal("Cannot allocate memory for UdpServerListener X{");
                }

                bs->setFlags(BlocksSource::REFLEXION_OPTIONS |
                             BlocksSource::TLS_OPTIONS |
                             BlocksSource::TLS_ENABLED |
                             BlocksSource::B64BLOCKS_OPTIONS);

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::TCP_SERVER);
                orch = ci;
            }
                break;
        case SourcesOrchestatorAbstract::TCP_PROXY:
            qDebug() << "TCP proxy choosen";
            {
                TLSServerListener * server = new(std::nothrow) TLSServerListener();
                if (server == nullptr) {
                    qFatal("Cannot allocate memory for TcpServerListener X{");
                }

                server->setFlags(BlocksSource::TLS_OPTIONS | BlocksSource::TLS_ENABLED);

                TLSClientListener *client = new(std::nothrow) TLSClientListener();
                if (client == nullptr) {
                    qFatal("Cannot allocate memory for TLSClientListener X{");
                }

                client->setFlags(BlocksSource::TLS_OPTIONS |
                                 BlocksSource::TLS_ENABLED |
                                 BlocksSource::IP_OPTIONS);

                ProxyOrchestrator *ci = new(std::nothrow) ProxyOrchestrator(server,client);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for ProxyOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::TCP_PROXY);
                orch = ci;
            }
            break;
        case SourcesOrchestatorAbstract::UDP_PROXY:
        {
            qDebug() << "UDP proxy choosen";
            BlocksSource *udpServer = new(std::nothrow) UdpServerListener();
            if (udpServer == nullptr) {
                qFatal("Cannot allocate memory for UdpServerListener X{");
            }

            udpServer->setFlags(0);

            BlocksSource *udpClient = new(std::nothrow) UdpClientListener();
            if (udpClient == nullptr) {
                qFatal("Cannot allocate memory for UdpClientListener X{");
            }

            udpClient->setFlags(BlocksSource::IP_OPTIONS);

            ProxyOrchestrator *ci = new(std::nothrow) ProxyOrchestrator(udpServer,udpClient);
            if (ci == nullptr) {
                qFatal("Cannot allocate memory for ProxyOrchestrator X{");
            }
            ci->setType(SourcesOrchestatorAbstract::UDP_PROXY);
            orch = ci;
            break;
        }
        case SourcesOrchestatorAbstract::BLOCKS_EXTERNAL_PROXY_UDP:
        {
            qDebug() << "UDP external proxy choosen";
            BlocksSource * bs1 = new(std::nothrow) UdpServerListener(QHostAddress::LocalHost, 3000);
            if (bs1 == nullptr) {
                qFatal("Cannot allocate memory for UdpServerListener X{");
            }
            bs1->setFlags(BlocksSource::REFLEXION_OPTIONS |
                          BlocksSource::REFLEXION_ENABLED );

            BlocksSource * bs2 = new(std::nothrow) UdpServerListener(QHostAddress::LocalHost, 3001);
            if (bs2 == nullptr) {
                qFatal("Cannot allocate memory for UdpServerListener X{");
            }
            bs2->setFlags(BlocksSource::REFLEXION_OPTIONS |
                          BlocksSource::REFLEXION_ENABLED );

            ExternalProxyOrchestrator *epu = new(std::nothrow) ExternalProxyOrchestrator(bs1,bs2);
            if (epu == nullptr) {
                qFatal("Cannot allocate memory for ExternalProxyUDPOrchestrator X{");
            }
            epu->setType(SourcesOrchestatorAbstract::BLOCKS_EXTERNAL_PROXY_UDP);
            orch = epu;
            break;
        }
        case SourcesOrchestatorAbstract::BLOCKS_EXTERNAL_PROXY_TCP:
        {
            qDebug() << "TCP external proxy choosen";
            BlocksSource * bs1 = new(std::nothrow) TLSServerListener(QHostAddress::LocalHost, 3000);
            if (bs1 == nullptr) {
                qFatal("Cannot allocate memory for TLSServerListener X{");
            }
            bs1->setFlags(BlocksSource::REFLEXION_OPTIONS |
                          BlocksSource::REFLEXION_ENABLED |
                          BlocksSource::TLS_OPTIONS |
                          BlocksSource::B64BLOCKS_OPTIONS |
                          BlocksSource::B64BLOCKS_ENABLED);

            BlocksSource * bs2 = new(std::nothrow) TLSServerListener(QHostAddress::LocalHost, 3001);
            if (bs2 == nullptr) {
                qFatal("Cannot allocate memory for TLSServerListener X{");
            }
            bs2->setFlags(BlocksSource::REFLEXION_OPTIONS |
                          BlocksSource::REFLEXION_ENABLED |
                          BlocksSource::TLS_OPTIONS |
                          BlocksSource::B64BLOCKS_OPTIONS |
                          BlocksSource::B64BLOCKS_ENABLED);

            ExternalProxyOrchestrator *epu = new(std::nothrow) ExternalProxyOrchestrator(bs1,bs2);
            if (epu == nullptr) {
                qFatal("Cannot allocate memory for ExternalProxyUDPOrchestrator X{");
            }
            epu->setType(SourcesOrchestatorAbstract::BLOCKS_EXTERNAL_PROXY_TCP);
            orch = epu;
            break;
        }

        case SourcesOrchestatorAbstract::SOCKS5_PROXY:
            qDebug() << "SOCK5 proxy choosen";
            {
                TLSServerListener * server = new(std::nothrow) TLSServerListener();
                if (server == nullptr) {
                    qFatal("Cannot allocate memory for TcpServerListener X{");
                }

                server->setFlags(BlocksSource::TLS_OPTIONS | BlocksSource::TLS_ENABLED);
                server->setSocks5Proxy(true);

                TLSClientListener *client = new(std::nothrow) TLSClientListener();
                if (client == nullptr) {
                    qFatal("Cannot allocate memory for TLSClientListener X{");
                }

                client->setFlags(BlocksSource::TLS_OPTIONS |
                                 BlocksSource::TLS_ENABLED);

                connect(server, &TLSServerListener::newConnectionData,
                        client, &TLSClientListener::setSpecificConnection) ;

                SocksOrchestrator *ci = new(std::nothrow) SocksOrchestrator(server,client);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SocksOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::SOCKS5_PROXY);
                orch = ci;
            }
            break;
            case SourcesOrchestatorAbstract::PIPE_CLIENT:
            {
                PipeClientListener *pl = new(std::nothrow) PipeClientListener();
                if (pl == nullptr) {
                    qFatal("Cannot allocate memory for PipeClientListener X{");
                }

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(pl);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::PIPE_CLIENT);
                orch = ci;
            }
            break;
        default:
            qCritical() << tr("[OrchestratorChooser::createOrchestratorFromType] Unmanaged type: %1").arg(type);
    }

    if (orch != nullptr) {
        connect(orch, &SourcesOrchestatorAbstract::log, guiHelper, &GuiHelper::logMessage);
        destructorLink = connect(orch, &SourcesOrchestatorAbstract::destroyed, this, &OrchestratorChooser::onOrchestratorDeleted);
    }
    return orch;
}


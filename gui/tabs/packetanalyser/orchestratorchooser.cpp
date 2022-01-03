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
#include "sources/blocksources/dtlsclientlistener.h"
#include "sources/blocksources/dtlsserverlistener.h"
#include "sources/blocksources/sharedmemorysource.h"
#include "sources/blocksources/fileblocksource.h"
#include "sources/blocksources/pipeclientlistener.h"
#include "guihelper.h"
#include "shared/defaultdialog.h"
#include "externalproxyorchestrator.h"
#include "proxyorchestrator.h"
#include "socksorchestrator.h"
#include "myoproxy.h"


OrchestratorChooser::OrchestratorChooser(GuiHelper *guiHelper, SourcesOrchestatorAbstract *orchestrator, QWidget *parent) :
    QComboBox(parent),
    orchestrator(orchestrator),
    guiHelper(guiHelper),
    confDialog(nullptr)
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
            destructorLink = connect(orchestrator, &SourcesOrchestatorAbstract::destroyed, this, &OrchestratorChooser::onOrchestratorDeleted);
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

int OrchestratorChooser::showConfPanel(SourcesOrchestatorAbstract *targetOrchestrator, bool blocking)
{
    int ret = QDialog::Rejected;
    if (targetOrchestrator != nullptr) {
        if (confDialog == nullptr) {
            confDialog = new(std::nothrow) DefaultDialog(this);
            if (confDialog == nullptr) {
                qFatal("Cannot allocate memory for DefaultDialog X{");
            }

            confDialog->setWindowTitle(tr("Configuration"));
            confDialog->resize(400,300);
        }

        QWidget * confWidget = targetOrchestrator->getConfGui(nullptr);

        if (confWidget != nullptr) {
            confDialog->setMainWidget(confWidget);
        }
        if (blocking) {
            ret = confDialog->exec();
            delete confDialog;
            confDialog = nullptr;
        } else {
            confDialog->raise();
            confDialog->show();
        }
    } else {
        qCritical() << tr("[OrchestratorChooser::showConfPanel] orchestrator is nullptr T_T");
    }


    return ret;
}

void OrchestratorChooser::onGuiRequested()
{
    if (orchestrator != nullptr) {
        showConfPanel(orchestrator);
    }
}

void OrchestratorChooser::onOrchestratorDeleted()
{
    orchestrator = nullptr;
}

void OrchestratorChooser::onSelection(int index)
{
    SourcesOrchestatorAbstract * temporchestrator = createOrchestratorFromType(index);
    if (temporchestrator != nullptr) {
        if (QDialog::Accepted == showConfPanel(temporchestrator, true)) {
            if (orchestrator != nullptr) {
                // need to disconnect due to the deferred deletion
                disconnect(destructorLink);
                orchestrator->stop();
                orchestrator->deleteLater();
            }
            orchestrator = temporchestrator;
            destructorLink = connect(orchestrator, &SourcesOrchestatorAbstract::destroyed, this, &OrchestratorChooser::onOrchestratorDeleted);
            emit newOrchestrator(temporchestrator);
        } else { // cancelled
            blockSignals(true);
            if (orchestrator != nullptr) {
                setCurrentIndex(static_cast<int>(orchestrator->getType())); // resetting to the previous one
            } else {
                setCurrentIndex(0); // or not
            }
            blockSignals(false);
            delete temporchestrator;
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

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }

                ci->setType(SourcesOrchestatorAbstract::UDP_CLIENT);
                orch = ci;

            }
            break;
        case SourcesOrchestatorAbstract::DTLS_CLIENT:
            {
                qDebug() << "DTLS client choosen";
                bs = new(std::nothrow) DTLSClientListener();
                if (bs == nullptr) {
                    qFatal("Cannot allocate memory for DTLSClientListener X{");
                }

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }

                ci->setType(SourcesOrchestatorAbstract::DTLS_CLIENT);
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

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::UDP_SERVER);
                orch = ci;
            }
            break;
        case SourcesOrchestatorAbstract::DTLS_SERVER:
            {
                qDebug() << "DTLS server choosen";
                bs = new(std::nothrow) DtlsServerListener();
                if (bs == nullptr) {
                    qFatal("Cannot allocate memory for DtlsServerListener X{");
                }

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(bs);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::DTLS_SERVER);
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
                                 BlocksSource::GEN_IP_OPTIONS);

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

            udpClient->setFlags(BlocksSource::GEN_IP_OPTIONS);

            ProxyOrchestrator *ci = new(std::nothrow) ProxyOrchestrator(udpServer,udpClient);
            if (ci == nullptr) {
                qFatal("Cannot allocate memory for ProxyOrchestrator X{");
            }
            ci->setType(SourcesOrchestatorAbstract::UDP_PROXY);
            orch = ci;
            break;
        }
        case SourcesOrchestatorAbstract::DTLS_PROXY:
        {
            qDebug() << "DTLS proxy choosen";
            BlocksSource *dtlsServer = new(std::nothrow) DtlsServerListener();
            if (dtlsServer == nullptr) {
                qFatal("Cannot allocate memory for DtlsServerListener X{");
            }

            dtlsServer->setFlags(BlocksSource::TLS_OPTIONS | BlocksSource::TLS_ENABLED);

            BlocksSource *dtlsClient = new(std::nothrow) DTLSClientListener();
            if (dtlsClient == nullptr) {
                qFatal("Cannot allocate memory for DTLSClientListener X{");
            }

            dtlsClient->setFlags(BlocksSource::TLS_OPTIONS |
                                 BlocksSource::TLS_ENABLED |
                                 BlocksSource::GEN_IP_OPTIONS);

            ProxyOrchestrator *ci = new(std::nothrow) ProxyOrchestrator(dtlsServer,dtlsClient);
            if (ci == nullptr) {
                qFatal("Cannot allocate memory for ProxyOrchestrator X{");
            }
            ci->setType(SourcesOrchestatorAbstract::DTLS_PROXY);
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
            case SourcesOrchestatorAbstract::MYO_PROXY:
            {
                MYOProxy *myop = new(std::nothrow) MYOProxy(guiHelper);
                if (myop == nullptr) {
                    qFatal("Cannot allocate memory for MYOProxy X{");
                }

                myop->setType(SourcesOrchestatorAbstract::MYO_PROXY);
                orch = myop;
            }
            break;
            case SourcesOrchestatorAbstract::SHARED_MEM:
            {
                SharedMemorySource * shms = new(std::nothrow) SharedMemorySource();
                if (shms == nullptr) {
                    qFatal("Cannot allocate memory for SharedMemorySource X{");
                }

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(shms);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::SHARED_MEM);
                orch = ci;
            }
            break;
            case SourcesOrchestatorAbstract::FILE_SOURCE:
            {
                FileBlockSource * fileSource = new(std::nothrow) FileBlockSource(FileBlockSource::Reader);
                if (fileSource == nullptr) {
                    qFatal("Cannot allocate memory for FileBlockSource X{");
                }

                SingleSourceOrchestrator *ci = new(std::nothrow) SingleSourceOrchestrator(fileSource);
                if (ci == nullptr) {
                    qFatal("Cannot allocate memory for SingleSourceOrchestrator X{");
                }
                ci->setType(SourcesOrchestatorAbstract::FILE_SOURCE);
                orch = ci;
            }
            break;
        default:
            qCritical() << tr("[OrchestratorChooser::createOrchestratorFromType] Unmanaged type: %1").arg(type);
    }

    if (orch != nullptr) {

        for (int i = 0 ; i < orch->blockSourceCount(); i++) {
            BlocksSource * bs = orch->getBlockSource(i);
            qInfo() << bs << "requested";
            if (bs != nullptr) {
                connect(bs, &BlocksSource::inboundTranformSelectionRequested, guiHelper, &GuiHelper::onInboundTransformRequested);
                connect(bs, &BlocksSource::outboundTranformSelectionRequested, guiHelper, &GuiHelper::onOutboundTransformRequested);
            }
        }
    }

    return orch;
}


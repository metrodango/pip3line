#include "ipblockssources.h"
#include "ipnetworkserverwidget.h"
#include "ipnetworkclientwidget.h"
#include <QDebug>
#include "baseblocksourcewidget.h"
#include "shared/sslconf.h"
#include <commonstrings.h>

IPBlocksSources::IPBlocksSources(QHostAddress hostAddress, quint16 hostPort, QObject *parent) :
    BlocksSource(parent),
    hostAddress(hostAddress),
    hostPort(hostPort)
{
    flags = TLS_OPTIONS | TLS_ENABLED;
    sslConfiguration = new(std::nothrow) SslConf();
    if (sslConfiguration == nullptr) {
        qFatal("Cannot allocate memory for SslConf X{");
    }

    connect(sslConfiguration, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), this, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
}

IPBlocksSources::~IPBlocksSources()
{
    delete sslConfiguration;
}

QHostAddress IPBlocksSources::getHostAddress() const
{
    return hostAddress;
}

void IPBlocksSources::setHostAddress(const QHostAddress &addr)
{
    hostAddress = addr;
}

quint16 IPBlocksSources::getHostPort() const
{
    return hostPort;
}

void IPBlocksSources::setHostPort(const quint16 &port)
{
    hostPort = port;
}

QWidget *IPBlocksSources::requestGui(QWidget *parent)
{

    BaseBlockSourceWidget * base = static_cast<BaseBlockSourceWidget *>(BlocksSource::requestGui(parent));
    if (base == nullptr) {
        qCritical() << tr("[IPBlocksSources::requestGui] base widget is nullptr T_T");
        return nullptr;
    }

    if (flags & TLS_OPTIONS)
        base->setTLSWidget(sslConfiguration->getGui());

    if (type == BlocksSource::CLIENT) {
        IPNetworkClientWidget * widget = new(std::nothrow) IPNetworkClientWidget(this, parent);
        if (widget == nullptr) {
            qFatal("Cannot allocate memory for NetworkClientWidget X{");
        }
        connect(widget, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)));
        connect(base, SIGNAL(tlsEnabled(bool)), widget, SLOT(onTlsToggled(bool)));
        base->insertWidgetInGeneric(0,widget);
    } else if (type == BlocksSource::SERVER) {
        IPNetworkServerWidget * widget = new(std::nothrow) IPNetworkServerWidget(this, parent);
        if (widget == nullptr) {
            qFatal("Cannot allocate memory for IPNetworkServerWidget X{");
        }
        connect(widget, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)));
        connect(base, SIGNAL(tlsEnabled(bool)), widget, SLOT(onTlsToggled(bool)));
        base->insertWidgetInGeneric(0,widget);
    }

    return base;
}
QSslConfiguration IPBlocksSources::getSslConfiguration()
{
    return sslConfiguration->getSslConfiguration();
}

void IPBlocksSources::setSslConfiguration(QSslConfiguration conf)
{
    sslConfiguration->setSslConfiguration(conf);
}

QWidget *IPBlocksSources::getSSLConfGui(QWidget *parent)
{
    return sslConfiguration->getGui(parent);
}

QList<QSslCertificate> IPBlocksSources::getPeerCerts()
{
    return QList<QSslCertificate>();
}

QString IPBlocksSources::getCurrentCipher()
{
    return QString();
}

QHash<QString, QString> IPBlocksSources::getConfiguration()
{
    QHash<QString, QString> ret = BlocksSource::getConfiguration();
    ret.insert(GuiConst::STATE_IP_ADDRESS,hostAddress.toString());
    ret.insert(GuiConst::STATE_PORT, QString::number(hostPort));
    if (!hostName.isEmpty())
        ret.insert(GuiConst::STATE_NAME, hostName);

    // todo TLS X{

    return ret;
}

void IPBlocksSources::setConfiguration(QHash<QString, QString> conf)
{
    BlocksSource::setConfiguration(conf);

    bool ok = false;

    if (conf.contains(GuiConst::STATE_IP_ADDRESS)) {
        QHostAddress addr;
        addr.setAddress(conf.value(GuiConst::STATE_IP_ADDRESS));
        if (!addr.isNull())
            setHostAddress(addr);
    }

    if (conf.contains(GuiConst::STATE_PORT)) {
        quint16 val = conf.value(GuiConst::STATE_PORT).toUShort(&ok);
        if (ok && val > 1) { // yeah it's an unsigned 16bits no need for < 65535
            setHostPort(val);
        }
    }

    if (conf.contains(GuiConst::STATE_NAME)) {
        QString name = conf.value(GuiConst::STATE_NAME);
        if (!name.isEmpty()) {
            setHostname(name);
        }
    }

    // todo TLS conf X{
}

QString IPBlocksSources::getHostname() const
{
    return hostName;
}

void IPBlocksSources::setHostname(const QString &name)
{
    hostName = name;
    sslConfiguration->setSslPeerNameSNI(name);
}




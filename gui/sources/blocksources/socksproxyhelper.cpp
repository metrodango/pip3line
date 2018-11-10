#include "socksproxyhelper.h"

#include <QBuffer>
#include <QString>
#include <QHostAddress>
#include <QHostInfo>
#include <QDebug>

SocksProxyHelper::SocksProxyHelper(QHostAddress serverAddr, quint16 serverPort) :
    connectionState(INVALID),
    host(),
    port(0),
    serverAddr(serverAddr),
    serverPort(serverPort)
{

}

QByteArray SocksProxyHelper::processRequest(QByteArray data)
{
    QByteArray ret;
    char byteRead[1] = { '\0' };
    qint64 bread = 0;
    QBuffer buff;

    buff.setBuffer(&data);

    buff.open(QIODevice::ReadOnly);

    if (data.size() < 3) {
        qWarning() << "SOCKS 5 [base] connection invalid";
        if (connectionState != INVALID) // is not first packet processed
            connectionState = REJECTED;
        return ret;
    }
    // return connection refused by default
    ret.append('\x05');
    ret.append('\xff');


    bread = buff.read(byteRead, 1);
    if (bread != 1) {
        qCritical() << "SOCKS 5 [base] cannot read version buffer T_T";
        if (connectionState != INVALID) // is not first packet processed
            connectionState = REJECTED;
        return ret;
    }

    quint8 version = static_cast<quint8>(byteRead[0]);

    if (version != 0x05 ) {
        qWarning() << "SOCKS 5 [base] only support version 5";
        if (connectionState != INVALID) // meaning it is not first packet processed
            connectionState = REJECTED;
    } else if (connectionState == INVALID) { // if this part fail at any point, just return INVALID
        qDebug() << "SOCKS 5 [init] proxying initiated";
        bread = buff.read(byteRead, 1);
        if (bread != 1) {
            qCritical() << "SOCKS cannot read buffer T_T";
            return ret;
        }
        quint8 auth_method_count = static_cast<quint8>(byteRead[0]);
        if (data.size() < static_cast<int>(auth_method_count) + 2) {
            qWarning() << "SOCKS 5 [init] connection invalid (auth_method_count)";
        } else if (auth_method_count < 5) {
            bread = buff.read(byteRead, 1);
            if (bread != 1) {
                qCritical() << "SOCKS 5 [init] cannot read auth_choosen buffer T_T";
                return ret;
            }
            quint8 auth_choosen = static_cast<quint8>(byteRead[0]);
            for (quint8 i = 1; i < auth_method_count ; i++) {
                bread = buff.read(byteRead, 1);
                if (bread != 1) {
                    qCritical() << "SOCKS 5 [init] cannot read auth_choosen buffer T_T";
                    return ret;
                }
                quint8 value = static_cast<quint8>(byteRead[0]);
                qDebug() << QString("SOCKS 5: [auth method requested] 0x%1").arg(QString::number(auth_choosen,16));
                if (value < auth_choosen) {
                    auth_choosen = value;
                }
            }
            ret[1] = static_cast<char>(auth_choosen);
            if (auth_choosen == 0)
                connectionState = AUTHENTICATED;
            else
                connectionState = NEED_AUTH_DATA;
        } else {
            qWarning() << "SOCKS 5 [init] connection invalid (auth_method_count > 5)";
        }
    } else if (connectionState == NEED_AUTH_DATA){
        // return authentication success by default :p
        ret[1] = '\00';
        connectionState = AUTHENTICATED;
    } else if (connectionState == AUTHENTICATED) {
        if (data.size() < 4 ) {
            qWarning() << "SOCKS 5 [setup] connection invalid (invalid auth data: too small)";
            connectionState = REJECTED;
        } else {
            bread = buff.read(byteRead, 1);
            if (bread != 1) {
                qCritical() << "SOCKS [setup] cannot read type T_T";
                connectionState = REJECTED;
                return ret;
            }
            quint8 type = static_cast<quint8>(byteRead[0]);
            if (type == 1) { // TCP stream, we're good
                ret[1] = '\00';
                ret.append('\00'); //reserved
                bread = buff.read(byteRead, 1);
                if (bread != 1) {
                    qCritical() << "SOCKS 5 [setup] cannot read the reserved byte T_T";
                    connectionState = REJECTED;
                    return ret;
                }
                quint8 reserved = static_cast<quint8>(byteRead[0]);
                if (reserved != '\x00') {
                    qWarning() << "SOCKS 5 [setup] weird the reserved field is not null: " << static_cast<quint8>(data.at(2)) ;
                }

                bread = buff.read(byteRead, 1);
                if (bread != 1) {
                    qCritical() << "SOCKS 5 [setup] cannot read addr_type buffer T_T";
                    connectionState = REJECTED;
                    return ret;
                }
                quint8 addr_type = static_cast<quint8>(byteRead[0]);
                QByteArray addr_data;
                if (addr_type == 1) { // IPv4
                    addr_data.resize(sizeof(quint32)); // 4 bytes for IPv4 (surprisingly)
                    bread = buff.read(addr_data.data(), sizeof(quint32));
                    if (bread != sizeof(quint32)) {
                        qCritical() << "SOCKS 5 [setup] cannot read IPv4 buffer T_T";
                        connectionState = REJECTED;
                        return ret;
                    }

                    quint32 addr_num = 0;
                    memcpy(&addr_num, addr_data.constData(),sizeof(quint32));
                    host.setAddress(addr_num);
                } else if (addr_type == 3) { // domain name
                    bread = buff.read(byteRead, 1);
                    if (bread != 1) {
                        qCritical() << "SOCKS 5 [setup] cannot read buffer (DN size)T_T";
                        connectionState = REJECTED;
                        return ret;
                    }
                    quint8 nameSize = static_cast<quint8>(byteRead[0]);
                    addr_data.resize(nameSize); // does not really matter if this is zero
                    bread = buff.read(addr_data.data(), nameSize);
                    if (bread != nameSize) {
                        qCritical() << "SOCKS 5 [setup] cannot read domain buffer T_T";
                        connectionState = REJECTED;
                        return ret;
                    }
                    QHostInfo info = QHostInfo::fromName(QString::fromUtf8(addr_data));
                    QList<QHostAddress> addrList = info.addresses();
                    if (addrList.size() > 0) {
                        for (int i = 0; i < addrList.size(); i++) {
                            if (addrList.at(i).protocol() == QAbstractSocket::IPv4Protocol) {
                                host = addrList.at(i);
                                break;
                            }
                        }

                        if (host.isNull()) // no IPv4 addresses found, fallback to the first one, whatever that is
                          host = info.addresses().at(0);
                    } else {
                        connectionState = REJECTED;
                    }

                } else if (addr_type == 4) { // IPv6
                    addr_data.resize(sizeof(Q_IPV6ADDR)); // 16 bytes for IPv6 (surprisingly)
                    bread = buff.read(addr_data.data(), sizeof(Q_IPV6ADDR));
                    if (bread != sizeof(Q_IPV6ADDR)) {
                        qCritical() << "SOCKS 5 [setup] cannot read IPv6 buffer T_T";
                        connectionState = REJECTED;
                        return ret;
                    }

                    Q_IPV6ADDR addr_num;
                    memcpy(addr_num.c, addr_data.constData(),sizeof(Q_IPV6ADDR));
                    host.setAddress(addr_num);
                }
                addr_data.resize(sizeof(quint16));

                bread = buff.read(addr_data.data(), sizeof(quint16)); // yep two bytes for a port number
                if (bread != sizeof(quint16)) {
                    qCritical() << "SOCKS 5 [setup] cannot read port T_T";
                    connectionState = REJECTED;
                    return ret;
                }
                // swapping due to network bytes order
                byteRead[0] = addr_data.at(0);
                addr_data[0] = addr_data.at(1);
                addr_data[1] = byteRead[0];
                memcpy(&port, addr_data.constData(),sizeof(quint16));
                qDebug() << "SOCKS 5 [setup] new destination details: " << host.toString() << port;
                connectionState = DONE;

                // now setting the details of the server
                if (serverAddr.protocol() == QAbstractSocket::IPv4Protocol) {
                    addr_data.resize(sizeof(quint32));
                    quint32 ipv4 = serverAddr.toIPv4Address();
                    memcpy(addr_data.data(),&ipv4,sizeof(quint32));
                    ret.append('\01');
                    ret.append(addr_data);
                } else if (serverAddr.protocol() == QAbstractSocket::IPv6Protocol) {
                    addr_data.resize(sizeof(Q_IPV6ADDR));
                    Q_IPV6ADDR addr_num = serverAddr.toIPv6Address();
                    memcpy(addr_data.data(),&addr_num,sizeof(Q_IPV6ADDR));
                    ret.append('\04');
                    ret.append(addr_data);
                } else {
                    qCritical() << "SOCKS [setup] cannot identify the server address type T_T";
                    connectionState = REJECTED;
                    return ret;
                }
                addr_data.resize(sizeof(quint16));
                memcpy(addr_data.data(),&serverPort,sizeof(quint16));
                ret.append(addr_data);

            } else {
                ret[1] = '\07'; // not accepted or protocol error
                QString log;
                if (type == 2) {
                    log = "TCP binding";
                }
                else if (type == 3)
                    log = "UDP";
                else
                    log = QString("INVALID (%1)").arg(type);
                qWarning() << "SOCKS 5 [setup] proxy only accept tcp stream, requested is " << log ;
                connectionState = REJECTED;
            }
        }
    } else {
        connectionState = REJECTED;
    }

    return ret;
}

SocksProxyHelper::CONNECTION_STATE SocksProxyHelper::getState() const
{
    return connectionState;
}

QHostAddress SocksProxyHelper::getHost() const
{
    return host;
}

quint16 SocksProxyHelper::getPort() const
{
    return port;
}

#ifndef MYOPROXY_H
#define MYOPROXY_H

#include <QObject>
#include "sourcesorchestatorabstract.h"

class MYOProxy : public SourcesOrchestatorAbstract
{
        Q_OBJECT
    public:
        enum SERVERS {
            SERVER_INVALID = 0,
            SERVER_TCP = 1,
            SERVER_UDP =2
        };

        enum CLIENTS {
            CLIENTS_INVALID = 0,
            CLIENT_TCP = 1,
            CLIENT_UDP = 2,
            CLIENT_PIPE = 3,
            SHARED_MEM = 4
        };

        static const QStringList SERVERS_LIST;
        static const QStringList CLIENTS_LIST;

        explicit MYOProxy(QObject *parent = nullptr);
        ~MYOProxy() override;
        QList<Target<SourcesOrchestatorAbstract *> > getAvailableConnections() override;
        BlocksSource *getBlockSource(int index) override;
        int blockSourceCount() const override;
        void setServer(MYOProxy::SERVERS serverVal);
        void setClient(MYOProxy::CLIENTS clientVal);
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(QHash<QString, QString> conf) override;
        MYOProxy::SERVERS getServerType() const;
        MYOProxy::CLIENTS getClientType() const;

    public slots:
        void postPacket(QSharedPointer<Packet> packet) override;
        bool start() override;
    private slots:
        void onBlockReceived(Block *block) override;
    private:
        Q_DISABLE_COPY(MYOProxy)
        QWidget *requestConfGui(QWidget *parent) override;
        BlocksSource * serverSource{nullptr};
        BlocksSource * clientSource{nullptr};
        MYOProxy::SERVERS serverType;
        MYOProxy::CLIENTS clientType;
};

#endif // MYOPROXY_H

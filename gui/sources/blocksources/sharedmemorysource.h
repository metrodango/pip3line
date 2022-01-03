#ifndef SHAREDMEMORYSOURCE_H
#define SHAREDMEMORYSOURCE_H

#include "blockssource.h"
#include <QObject>

class SharedmemConnector;
class SharedMemorySourceBaseWidget;

class SharedMemorySource : public BlocksSource
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit SharedMemorySource(QObject *parent = nullptr);
        ~SharedMemorySource() override;
        QWidget *getAdditionnalCtrls(QWidget *parent) override;
        QString getName() override;
        QString getDescription() override;
        bool isStarted() override;
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(const  QHash<QString, QString> &conf) override;
        static QStringList getAvailableSHMTypes();
        void setShmType(int smtype);
        int getShmType() const;
        int getTargetIdFor(int sourceId) override;
    public slots:
        void sendBlock(Block *block) override;
        bool startListening() override;
        void stopListening() override;
        void onGuiDestroyed();
        void onConnectionClosed(int cid) override;
    protected:
        QWidget *requestGui(QWidget *parent) override;
        void internalUpdateConnectionsInfo() override;
    private slots:
        void checkData();
        void onShmReset();
    private:
        Q_DISABLE_COPY(SharedMemorySource)
        SharedmemConnector * memConn;
        bool running;
        QThread workerThread;
        QTimer checkTimer;
        QByteArray currentData;
        int timerInterval;
        SharedMemorySourceBaseWidget * localGui;
        int shmType;
        QList<int> extsources;
};

#endif // SHAREDMEMORYSOURCE_H

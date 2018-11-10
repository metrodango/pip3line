#ifndef SHAREDMEMPOSIX_H
#define SHAREDMEMPOSIX_H

#include "sharedmemconnector.h"
#include <QSharedMemory>
#include <QObject>

class SharedMemPosix : public SharedmemConnector
{
        Q_OBJECT
    public:
        static const QString ID;
        SharedMemPosix();
        ~SharedMemPosix() override;
        QString name() override;
        bool connectToMem() override;
        bool readData(QByteArray &data) override;
        void writeData(const QByteArray & data) override;
        void disconnectFromMem() override;
        int getSize() const override;
        void setSize(int size) override;
        QString getKey() const;
        void setKey(const QString &value);
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(const QHash<QString, QString> &conf) override;
        QString getCurrentID() override;
    private:
        Q_DISABLE_COPY(SharedMemPosix)
        static const int MAX_FILE_NAME;
        QWidget *internalGetGui(QWidget *parent) override;
        QSharedMemory mem;
        int size;
        QString key;
};

#endif // SHAREDMEMPOSIX_H

#ifndef SYSV_SHM_H
#define SYSV_SHM_H

#include "sharedmemconnector.h"

#ifdef Q_OS_UNIX
#include <QObject>
#include <sys/shm.h>

class SysV_Shm : public SharedmemConnector {
        Q_OBJECT
    public:
        static const QString ID;
        struct ShmData {
            key_t key;
            int shmid;
            QString perms;
            quint64 size;
            quint32 cpid;
            quint32 lpid;
            quint32 nattch;
            quint32 uid;
            quint32 gid;
            quint32 cuid;
            quint32 cgid;
            quint64 atime;
            quint64 dtime;
            quint64 ctime;
            quint32 rss;
            quint32 swap;
        };
        static QList<SysV_Shm::ShmData> getSHMList();
        explicit SysV_Shm();
        ~SysV_Shm() override;
        QString name() override;
        bool connectToMem() override;
        bool readData(QByteArray &data) override;
        void writeData(const QByteArray & data) override;
        void disconnectFromMem() override;
        int getSize() const override;
        void setSize(int size) override;
        key_t getKey() const;
        void setKey(const key_t &value);
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(const  QHash<QString, QString> &conf) override;
        int getPerms() const;
        void setPerms(int value);
        int getShmid() const;
        void setShmid(int value);
        QString getFilename() const;
        void setFilename(const QString &value);
        QString getCurrentID() override;
    private:
        Q_DISABLE_COPY(SysV_Shm)
        QWidget *internalGetGui(QWidget *parent) override;
        QString filename;
        int shmid;
        key_t key;
        int size;
        char *shm;
        int perms;
};

#endif //Q_OS_UNIX

#endif // SYSV_SHM_H

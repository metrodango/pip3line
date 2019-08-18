#ifndef SHAREDMEMCONNECTOR_H
#define SHAREDMEMCONNECTOR_H

#include "sources/blocksources/blockssource.h"
#include <QObject>

class SharedmemConnector : public QObject {
        Q_OBJECT
    public:
        enum MemoryAccessType {
            INVALID = 0,
            POSIX_NATIVE_SHM = 1,
            SYSV_SHM = 2
        };
        explicit SharedmemConnector(const MemoryAccessType &type);
        virtual ~SharedmemConnector() override;
        virtual QString name() = 0;
        virtual bool connectToMem() = 0;
        virtual bool readData(QByteArray &data) = 0;
        virtual void writeData(const QByteArray & data) = 0;
        virtual void disconnectFromMem() = 0;
        virtual int getSize() const = 0;
        virtual void setSize(int size) = 0;
        virtual QHash<QString, QString> getConfiguration();
        virtual void setConfiguration(const  QHash<QString, QString> &conf);
        QWidget * getGui(QWidget * parent = nullptr);
        MemoryAccessType getType() const;
        QString getLastError() const;
        bool hasError() const;
        virtual QString getCurrentID() = 0;
    public slots:
        void onGuiDestroyed();
    signals:
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
        void reset();
    protected:
        void setErrorMessage(const QString &message);
        virtual QWidget *internalGetGui(QWidget * parent) = 0;
        QWidget *gui;
        MemoryAccessType type;
        QString lastError;
    private:
        Q_DISABLE_COPY(SharedmemConnector)
};

#endif // SHAREDMEMCONNECTOR_H

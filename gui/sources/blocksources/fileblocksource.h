#ifndef FILEBLOCKSOURCE_H
#define FILEBLOCKSOURCE_H

#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QTimer>
#include <QString>
#include "blockssource.h"

class FileBlockSource : public BlocksSource
{
        Q_OBJECT
    public:
        static const QString ID;
        static const int MIN_DELAY;
        static const int MAX_DELAY;
        static const QString ID_SOURCE;
        static const QString ID_SINK;
        enum Type {
            Reader,
            Writer
        };
        explicit FileBlockSource(Type sourcefiletype, QObject *parent = nullptr);
        ~FileBlockSource() override;
        QString getName() override;
        QString getDescription() override;
        bool isStarted() override;
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(const QHash<QString, QString> &conf) override;
        QString getFilename() const;
        FileBlockSource::Type getSourcefiletype() const;
        void setSourcefiletype(const Type &value);
        int getReadDelay() const;
        int getTargetIdFor(int sourceId) override;
    public slots:
        void sendBlock(Block * block) override;
        bool startListening() override;
        void stopListening() override;
        void setFilename(const QString &value);
        void readNextBlock();
        void setReadDelay(int value);
    private:
        QWidget *requestGui(QWidget * parent = nullptr) override;
        void processIncomingDataBlock(QByteArray & data);
        FileBlockSource::Type sourcefiletype;
        QString filename;
        bool running;
        QFile file;
        QTextStream stream;
        QTimer timer;
        int readDelay;
};

#endif // FILEBLOCKSOURCE_H

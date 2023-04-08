/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef SCREENIODEVICE_H
#define SCREENIODEVICE_H

#include <QIODevice>
#include <QPlainTextEdit>

class ScreenIODevice : public QIODevice
{
        Q_OBJECT
    public:
        explicit ScreenIODevice(QPlainTextEdit *outWidget, QObject *parent = nullptr);
        
        bool atEnd() const;
        qint64 bytesAvailable() const;
        qint64 bytesToWrite() const;
        bool canReadLine() const;
        bool seek(qint64 pos);
        qint64 size() const;
        bool waitForBytesWritten(int);
        bool waitForReadyRead(int);

        void setWidget(QPlainTextEdit * outWidget);
    signals:
        void packet(QString packet);
    private slots:
        void processPacket(QString packet);

    private:
        Q_DISABLE_COPY(ScreenIODevice)
        qint64 writeData(const char * src, qint64 maxSize);
        qint64 readData(char * dest, qint64 maxSize);
        QPlainTextEdit * widget;
};

#endif // SCREENIODEVICE_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef STREAMPROCESSOR_H
#define STREAMPROCESSOR_H

#include "processor.h"
#include <QTcpSocket>
#include <QSemaphore>

class StreamProcessor : public Processor
{
        Q_OBJECT
    public:
        explicit StreamProcessor(TransformMgmt * tFactory,QObject *parent = nullptr);
        virtual void run();
    public slots:
        void stop();
    protected:
        explicit StreamProcessor() {}
        QSemaphore runSem;
    private:
        Q_DISABLE_COPY(StreamProcessor)

};

#endif // STREAMPROCESSOR_H

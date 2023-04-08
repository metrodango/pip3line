/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "streamprocessor.h"
#include <commonstrings.h>

StreamProcessor::StreamProcessor(TransformMgmt *tFactory, QObject *parent) :
    Processor(tFactory,parent)
{
}

void StreamProcessor::run()
{
    QByteArray data;
    QByteArray block;
    QList<QByteArray> dataList;

    while (true) {
        bool dataPresent = in->waitForReadyRead(3000);
        if (dataPresent) {
            data = in->readAll();
            if (data.size() == 0) {
                if (runSem.tryAcquire())
                    break;
                continue;
            }

            int count = data.count(separator);

            if (count > 0) {
                dataList = data.split(separator);
                block.append(dataList.takeFirst());

                writeBlock(block);

                count--;

                for (int i = 0 ; i < count ; i++) {
                    writeBlock(dataList.at(i));
                }

                if (count < dataList.size())
                    block = dataList.last();
                else
                    block.clear();

            } else {
                block.append(data);
                if (block.size() > BLOCK_MAX_SIZE) {
                    block.resize(BLOCK_MAX_SIZE);
                    emit error("Data received from the pipe is too large, the block has been truncated.","StreamProcessor");
                    writeBlock(block);
                    block.clear();
                }
            }
        }

        if (runSem.tryAcquire())
            break;

    }
    if (!block.isEmpty())
        writeBlock(block);

}

void StreamProcessor::stop()
{
    runSem.release();
}

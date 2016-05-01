/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "binaryprocessor.h"

const int BinaryProcessor::BLOCKSIZE = 4096;

BinaryProcessor::BinaryProcessor(TransformMgmt * tFactory, QObject *parent) :
    Processor(tFactory, parent)
{
}

BinaryProcessor::~BinaryProcessor()
{

}

void BinaryProcessor::run()
{
    QByteArray block;

    char buffer[BLOCKSIZE];
	
    if (tlist.isEmpty()) {
        emit error(tr("Error: No transformation configured, exiting the process."), "Binary Processor");
        return;
    }

    while (!in->atEnd()) {
        qint64 whatHasBeenDone = in->read(buffer,BLOCKSIZE);

        if (whatHasBeenDone == -1) {
            emit error(in->errorString(), "Binary Processor");
            break;
        }
        block.append(buffer,whatHasBeenDone);
        if (block.size() > BLOCK_MAX_SIZE) {
            block.resize(BLOCK_MAX_SIZE);
            emit error("Data received from the pipe is too large, the block has been truncated.","BinaryProcessor");
            break;
        }
    }

    writeBlock(block);

}

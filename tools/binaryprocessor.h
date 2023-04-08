/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef BINARYPROCESSOR_H
#define BINARYPROCESSOR_H

#include "processor.h"

class BinaryProcessor: public Processor
{
        Q_OBJECT
    public:
        static const int BLOCKSIZE;
        explicit BinaryProcessor(TransformMgmt *tFactory, QObject * parent = 0);
        ~BinaryProcessor();
        void run();
    private:
        Q_DISABLE_COPY(BinaryProcessor)
        BinaryProcessor() {}
};

#endif // BINARYPROCESSOR_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef TEXTPROCESSOR_H
#define TEXTPROCESSOR_H

#include "processor.h"

class TextProcessor: public Processor
{
        Q_OBJECT
    public:
        explicit TextProcessor(TransformMgmt *tFactory, QObject * parent = nullptr);
        ~TextProcessor();
        void run();
    protected:
        TextProcessor() {}
    private:
        Q_DISABLE_COPY(TextProcessor)
};

#endif // TEXTPROCESSOR_H

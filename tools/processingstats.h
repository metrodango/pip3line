/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PROCESSINGSTATS_H
#define PROCESSINGSTATS_H

#include <QtGlobal>

class ProcessingStats
{
    public:
        explicit ProcessingStats();
        ProcessingStats(const ProcessingStats& other);
        ProcessingStats &operator=(const ProcessingStats &other);
        ProcessingStats &operator+=(ProcessingStats & other);
        void reset();

        quint64 getOutBlocks() const;
        quint64 getInBlocks() const;
        quint64 getErrorsCount() const;
        quint64 getWarningsCount() const;
        quint64 getStatusCount() const;

        void incrOutBlocks(quint64 val = 1);
        void incrInBlocks(quint64 val = 1);
        void incrErrorsCount(quint64 val = 1);
        void incrWarningsCount(quint64 val = 1);
        void incrStatusCount(quint64 val = 1);
    private:
        quint64 outBlocks;
        quint64 inBlocks;
        quint64 errorCounts;
        quint64 warningCounts;
        quint64 statusCounts;
};

#endif // PROCESSINGSTATS_H

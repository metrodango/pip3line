/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "processingstats.h"

ProcessingStats::ProcessingStats()
{
    reset();
}

ProcessingStats::ProcessingStats(const ProcessingStats &other)
{
    *this = other;
}

ProcessingStats &ProcessingStats::operator=(const ProcessingStats &other)
{
    outBlocks = other.outBlocks;
    inBlocks = other.inBlocks;
    errorCounts = other.errorCounts;
    warningCounts = other.warningCounts;
    statusCounts = other.statusCounts;
    return *this;
}

ProcessingStats & ProcessingStats::operator+=(ProcessingStats &other)
{
    outBlocks += other.outBlocks;
    inBlocks += other.inBlocks;
    errorCounts += other.errorCounts;
    warningCounts += other.warningCounts;
    statusCounts += other.statusCounts;
    return *this;
}

void ProcessingStats::reset()
{
    outBlocks = 0;
    inBlocks = 0;
    errorCounts = 0;
    warningCounts = 0;
    statusCounts = 0;
}

quint64 ProcessingStats::getOutBlocks() const
{
    return outBlocks;
}

void ProcessingStats::incrOutBlocks(quint64 val)
{
    outBlocks += val;
}

quint64 ProcessingStats::getInBlocks() const
{
    return inBlocks;
}

void ProcessingStats::incrInBlocks(quint64 val)
{
    inBlocks += val;
}

quint64 ProcessingStats::getErrorsCount() const
{
    return errorCounts;
}

void ProcessingStats::incrErrorsCount(quint64 val)
{
    errorCounts += val;
}

quint64 ProcessingStats::getWarningsCount() const
{
    return warningCounts;
}

void ProcessingStats::incrWarningsCount(quint64 val)
{
    warningCounts += val;
}

quint64 ProcessingStats::getStatusCount() const
{
    return statusCounts;
}


void ProcessingStats::incrStatusCount(quint64 val)
{
    statusCounts += val;
}

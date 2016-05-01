/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef LARGEFILE_H
#define LARGEFILE_H

#include "largerandomaccesssource.h"
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMutex>

class FileSourceReader : public SourceReader
{
    public:
        explicit FileSourceReader(QString filename);
        ~FileSourceReader();
        bool seek(quint64 pos);
        int read(char * buffer, int maxLen);
        bool isReadable();
    private:
        bool open();
        QFile file;
};

class FileSearch : public SearchAbstract {
    Q_OBJECT
    public:
        explicit FileSearch(QString fileName);
        ~FileSearch();
        void setFileName(QString fileName);
    private:
        void internalStart();
        QString filename;
        static const quint32 MIN_SIZE_FOR_THREADS;
};

class LargeFile : public LargeRandomAccessSource
{
        Q_OBJECT
    public:
        explicit LargeFile(QObject *parent = 0);
        ~LargeFile();
        QString description();
        QString name();
        quint64 size();
        void fromLocalFile(QString fileName);
        void fromLocalFile(QString fileName,quint64 startOffset);
        QString fileName() const;
        void saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset);
        void saveToFile(QString destFilename);
        bool isOffsetValid(quint64 offset);
        bool tryMoveUp(int size);
        bool tryMoveDown(int size);
        bool tryMoveView(int size);
        BaseStateAbstract *getStateMngtObj();
    signals:
        void infoUpdated();
    private slots:
        void onFileChanged(QString path);
    private:
        Q_DISABLE_COPY(LargeFile)
        static const int BLOCKSIZE;
        static const qint64 MAX_COMPARABLE_SIZE;
        bool isFileReadable();
        bool isFileWriteable();
        bool seekFile(quint64 offset);
        QWidget * requestGui(QWidget *parent,ByteSourceAbstract::GUI_TYPE type);
        SearchAbstract *requestSearchObject(QObject *parent = 0);
        bool readData(quint64 offset, QByteArray &data, int size);
        bool writeData(quint64 offset, int length, const QByteArray &data, quintptr source);
        QFile file;
        qint64 fileSize;
        QFileInfo infoFile;
        QFileSystemWatcher watcher;
};

class LargeFileSourceStateObj : public LargeRandomAccessSourceStateObj
{
        Q_OBJECT
    public:
        explicit LargeFileSourceStateObj(LargeFile *lf);
        ~LargeFileSourceStateObj();
    protected:
        void internalRun();
};

#endif // LARGEFILE_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "largefile.h"
#include "filewidget.h"
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif
#include <QtConcurrentRun>
#include <QDebug>
#include <string.h>
#include <QHBoxLayout>
#include <QPushButton>
#include <climits>

FileSourceReader::FileSourceReader(QString filename):
    file(filename)
{

}

FileSourceReader::~FileSourceReader()
{
    file.close();
}

bool FileSourceReader::seek(quint64 pos)
{
    if (pos > LLONG_MAX) {
        emit log(tr("pos is too large for seeking, ignoring"),metaObject()->className(),Pip3lineConst::LERROR);
        return false;
    }
    return file.seek((qint64)pos);
}

int FileSourceReader::read(char *buffer, int maxLen)
{
    if (maxLen > MAX_READ_SIZE) { // we don't care if maxLen is over the file limit, as the QFile::read call will take care of it
        emit log(tr("maxLen is too large for reading, reducing it."),metaObject()->className(),Pip3lineConst::LWARNING);
        maxLen = MAX_READ_SIZE; // MAX_READ_SIZE is small
    }
    qint64 ret = file.read(buffer,(qint64) maxLen); // maxLen is small
    if (ret < 0) {
        emit log(tr("File read error: %1").arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
    }
    return (int)ret;
}

bool FileSourceReader::isReadable()
{
    if (!open())
        return false;

    return file.isOpen() && file.isReadable();
}

bool FileSourceReader::open()
{
    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly)) {
            emit log(tr("File open error: %1").arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
            return false;
        }
    }
    return true;
}

const quint32 FileSearch::MIN_SIZE_FOR_THREADS = 0x4000000;// ~ 67Mb

FileSearch::FileSearch(QString fileName) :
    filename(fileName)
{

}

FileSearch::~FileSearch()
{

}

void FileSearch::setFileName(QString nfileName)
{
    filename = nfileName;
}

void FileSearch::internalStart()
{
    if (sitem.isEmpty())
        return;
    QFileInfo info(filename);
    quint64 fsize = info.size();
    totalSearchSize = fsize;
    quint64 blocksize = 0;
    int threadCount;

    if (fsize < MIN_SIZE_FOR_THREADS)
        threadCount = 1;
    else {
        threadCount = QThread::idealThreadCount();
        if (threadCount < 1) {
            threadCount = 1; // just to be safe
        } else if (threadCount > 15) { // really ????
            threadCount = 10; // set a lower value
        }
    }

    blocksize = fsize / ((quint64)threadCount);
    int i = 0;
    for (i = 0; i < threadCount; i++) {
        SearchWorker * worker = nullptr;
        FileSourceReader * file = new(std::nothrow)FileSourceReader(filename);
        if (file == nullptr) {
            qFatal("Cannot allocate memory for QFile X{");
        }
        connect(file, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)));
        worker = new(std::nothrow) SearchWorker(file);

        if (worker == nullptr) {
            qFatal("Cannot allocate memory for SearchWorker X{");
        } else {
            worker->setStartOffset(i * blocksize);
            worker->setEndOffset(qMin((i+1) * blocksize + sitem.size(),fsize) - 1); // qMin is for the last block
            worker->setStatsStep((quint64)((double)totalSearchSize * 0.01)); // setting stats steps at 1% of the total size
            addSearchWorker(worker);
        }
    }
}

const int LargeFile::BLOCKSIZE = 1024;
const qint64 LargeFile::MAX_COMPARABLE_SIZE = 0x40000000; // 1G

LargeFile::LargeFile(QObject *parent) :
    LargeRandomAccessSource(parent)
{
    _name = tr("Large file source");
    capabilities = CAP_LOADFILE;
    chunksize = BLOCKSIZE;
    fileSize = 0;
    _readonly = true;
    connect(&watcher, SIGNAL(fileChanged(QString)), SLOT(onFileChanged(QString)));
}

LargeFile::~LargeFile()
{
    file.close();
}

QString LargeFile::description()
{
    return tr("File: %1").arg(file.fileName());
}

QString LargeFile::name()
{
    return file.fileName();
}

quint64 LargeFile::size()
{
    if (file.isReadable()) {
        return (quint64)fileSize; // this should be a non-negative value
    }
    return 0;
}

void LargeFile::fromLocalFile(QString fileName)
{
    fromLocalFile(fileName, 0);
}

void LargeFile::fromLocalFile(QString fileName, quint64 startOffset)
{
    if (fileName.isEmpty()) {
        qCritical() << tr("[LargeFile::fromLocalFile] file name string is empty, ignoring request T_T");
        return;
    }

    if (file.isOpen()) {
        watcher.removePath(infoFile.absoluteFilePath());
        file.close();
    }

    historyClear();
    clearAllMarkings();

    capabilities = CAP_LOADFILE;
    file.setFileName(fileName);
    infoFile.setFile(file);
    emit nameChanged(infoFile.fileName());
    if (!infoFile.isReadable()) {
        emit log(tr("File %1 is not readable").arg(fileName),metaObject()->className(),Pip3lineConst::LERROR);
        dataChunk.clear();
    } else {
        if (infoFile.isWritable()) {
            emit log(tr("File %1 is writeable").arg(fileName),metaObject()->className(),Pip3lineConst::LSTATUS);

            if (!file.open(QIODevice::ReadWrite)) {
                emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
                _readonly = true;
            } else {
                capabilities = capabilities | CAP_HISTORY | CAP_SEARCH | CAP_WRITE;
                _readonly = false;
            }
        } else {
            emit log(tr("File %1 is not writeable").arg(fileName),metaObject()->className(),Pip3lineConst::LSTATUS);
            _readonly = true;
            if (!file.open(QIODevice::ReadOnly)) {
                emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
            } else {
                capabilities = capabilities | CAP_SEARCH;
            }
        }

        if (file.isOpen()) {
            fileSize = infoFile.size();
            if (fileSize < 0) {
                emit log(tr("File size is negative, setting to zero"),metaObject()->className(),Pip3lineConst::LERROR);
                fileSize = 0;
            }
            if (fileSize < MAX_COMPARABLE_SIZE) {
                capabilities = capabilities | CAP_COMPARE;
            } else {
                emit log(tr("File is too large to be used as a comparable sample in its entirety. Comparison with selected bytes will still work however."),metaObject()->className(),Pip3lineConst::LWARNING);
            }
            refreshData(false);
            // file watcher
            watcher.addPath(infoFile.absoluteFilePath());
            if (searchObj != nullptr) {
                FileSearch * fsearch = qobject_cast<FileSearch *>(searchObj);
                if (fsearch != nullptr)
                    fsearch->setFileName(infoFile.absoluteFilePath());
            }
        }
        currentStartingOffset = startOffset;
    }
    _readonly = true; // set it to default , to avoid accidents

    emit infoUpdated();

    emit updated(INVALID_SOURCE);
    emit sizeChanged();
    emit reset();
}

QString LargeFile::fileName() const
{
    return (file.exists() ? infoFile.absoluteFilePath() : "INVALID");
}

void LargeFile::saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset)
{
    if (fileSize < 1)
        return;

    ByteSourceAbstract::saveToFile(destFilename, startOffset, endOffset);
}

void LargeFile::saveToFile(QString destFilename)
{
    if (!destFilename.isEmpty()) {
        QString sfileName = fileName();
        QFile sfile(sfileName);
        if (!sfile.copy(destFilename)) {
            emit log(tr("Failed to copy %1 to %2:\n %3").arg(sfileName).arg(destFilename).arg(sfile.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
        }
    } else {
        emit log(tr("Destination file is empty, ignoring save."),metaObject()->className(),Pip3lineConst::LERROR);
    }
}

bool LargeFile::isOffsetValid(quint64 offset)
{
    return offset < (quint64)fileSize; //  filesize should always be positive
}

bool LargeFile::tryMoveUp(int sizeToMove)
{
    return tryMoveView(-sizeToMove);
}

bool LargeFile::tryMoveDown(int sizeToMove)
{
    return tryMoveView(sizeToMove);
}

bool LargeFile::tryMoveView(int sizeToMove)
{
    // qDebug() << "[LargeFile::tryMoveView]" << sizeToMove;
    quint64 newOffset;

    if (sizeToMove < 0) {
        if (currentStartingOffset == 0)
            return false; // already at the beginning, nothing to see here

        if (currentStartingOffset < (quint64)(-1 * sizeToMove)) { // checking how much we can go up
            newOffset = 0;
        } else {
            newOffset = currentStartingOffset + sizeToMove;
        }
    } else {
        if (ULLONG_MAX - (quint64)sizeToMove - (quint64)chunksize < currentStartingOffset) {
            return false; // checking overflow
        }

        if (currentStartingOffset + (quint64)chunksize >= (quint64)fileSize) {
            return false; // no more data
        }

        newOffset = currentStartingOffset + sizeToMove;
    }

    int readsize = qMin((quint64)fileSize - newOffset,(quint64)chunksize);
    QByteArray temp;
    if (!readData(newOffset,temp,readsize)) {
        return false;
    }
  //  qDebug() << tr("[LargeFile::tryMoveView] size to move: %1 Current offset: %2 new offset: %3")
   //             .arg(sizeToMove).arg(currentStartingOffset,16, 16, QChar('0')).arg(newOffset,16, 16, QChar('0'));

    currentStartingOffset = newOffset;
    //qDebug() << "tryMoveView currentStartingOffset" << newOffset;
    dataChunk = temp;
    emit updated(INVALID_SOURCE);
    emit sizeChanged();
    return true;
}

BaseStateAbstract *LargeFile::getStateMngtObj()
{
    LargeFileSourceStateObj *stateObj = new(std::nothrow) LargeFileSourceStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for LargeFileSourceStateObj X{");
    }

    return stateObj;
}

void LargeFile::onFileChanged(QString path)
{
    qDebug() << "File updated externally: " << path;
    fromLocalFile(file.fileName(),currentStartingOffset);
}

bool LargeFile::isFileReadable()
{
    if (!file.isReadable()) {
        emit log(tr("File %1 is not readable").arg(file.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

bool LargeFile::isFileWriteable()
{
    if (!file.isWritable()) {
        emit log(tr("File %1 is not writable").arg(file.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

bool LargeFile::seekFile(quint64 offset)
{
    if (offset > LLONG_MAX) {
        emit log(tr("[seekFile] Hitting the LLONG_MAX limit for Qt, ignoring"),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    if (!file.seek(offset)) {
        emit log(tr("Error while seeking: %1").arg(file.errorString()),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    return true;
}

QWidget *LargeFile::requestGui(QWidget *parent, GUI_TYPE type)
{

    QWidget *fw = nullptr;
    if (type == GUI_CONFIG) {
        fw = new(std::nothrow)FileWidget(this,parent);
        if (fw == nullptr) {
            qFatal("Cannot allocate memory for FileWidget X{");
        }
    } else if (type == ByteSourceAbstract::GUI_BUTTONS) {
        fw = new(std::nothrow) QWidget(parent);
        if (fw == nullptr) {
            qFatal("Cannot allocate memory for QWidget X{");
        }

        QHBoxLayout * layout = new(std::nothrow) QHBoxLayout(fw);
        if (layout == nullptr) {
            qFatal("Cannot allocate memory for QHBoxLayout X{");
        }

        fw->setLayout(layout);

        QPushButton * openFile = new(std::nothrow) QPushButton(fw);
        if (openFile == nullptr) {
            qFatal("Cannot allocate memory for QPushButton X{");
        }

        openFile->setIcon(QIcon(":/Images/icons/document-open-8.png"));
        openFile->setMaximumWidth(25);
        openFile->setToolTip(tr("Open file"));
        openFile->setFlat(true);
        connect(openFile, SIGNAL(clicked()), SIGNAL(askFileLoad()));
        layout->addWidget(openFile);
    }

    return fw;
}

SearchAbstract *LargeFile::requestSearchObject(QObject *)
{
    qDebug() << "creating search object" << infoFile.fileName();
    FileSearch *sObj = new(std::nothrow) FileSearch(infoFile.absoluteFilePath());

    if (sObj == nullptr) {
        qFatal("Cannot allocate memory for FileSearch X{");
    }
    connect(sObj, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
    connect(sObj, SIGNAL(foundList(BytesRangeList*)), SLOT(setNewMarkings(BytesRangeList*)), Qt::QueuedConnection);

    return sObj;
}

bool LargeFile::readData(quint64 offset, QByteArray &data, int length)
{
    data.clear();
    if (!file.isReadable())
        return false;
   // qDebug() << "read Data: " << offset << length << fileSize;
    bool noError = true;
    if (length < 1 || offset > (quint64)fileSize)
        noError = false;
    else {

        if ((quint64)(LLONG_MAX - length) < offset ) {
            qDebug() << tr("Hitting LLONG_MAX limit");
            noError = false;
        } else if (seekFile(offset)) {
            char * buf = new(std::nothrow) char[length];
            if (buf == nullptr) {
                qFatal("Cannot allocate memory for the file buffer X{");
            }
            if (offset + (quint64)length > (quint64)fileSize) {
                length = (quint64)fileSize - offset; // just skip the last bytes if the requested block goes out-of-bound
                //qDebug() << "Reducing length";
            }
         //   qDebug() << "read Data(2): " << offset << length << fileSize;
            qint64 bytesRead = file.read(buf, length);
            if (bytesRead < 0) {
                emit log(tr("Cannot read file: %1").arg(file.errorString()), metaObject()->className(), Pip3lineConst::LERROR);
                noError = false;
            } else if (bytesRead == 0) {
                qDebug() << tr("No byte returned when reading file");
                noError = false;
            } else {
                if (bytesRead < length) {
                    qDebug() << tr("Length read (%1) inferior to length requested (%2) at %3").arg(bytesRead).arg(length).arg(offset);
                }
                data = QByteArray(buf, bytesRead);
            }

            delete [] buf;
        }
    }
    return noError;
}

bool LargeFile::writeData(quint64 offset, int length, const QByteArray &repData, quintptr source)
{
    bool noError = false;
    if (_readonly) return noError;

    if (fileSize < 0 ) {
        emit log(tr("File size is negative, X["), metaObject()->className(), Pip3lineConst::LERROR);
        return noError;
    }

    if (length < 0 )  {
        emit log(tr("length is negative, X["), metaObject()->className(), Pip3lineConst::LERROR);
        return noError;
    }

    if (isFileWriteable()) {

        if (offset > (quint64)fileSize) {
            emit log(tr("Offset is outside the file, ignoring"), metaObject()->className(), Pip3lineConst::LERROR);
            return noError;
        }

        if (repData.size() < length){
            emit log(tr("replace: target block length is superior to the new block length, only overwriting the first bytes"), metaObject()->className(), Pip3lineConst::LWARNING);
        }

        if (repData.size() > length){
            emit log(tr("replace: target block length is inferior to the new block length, overwriting the next bytes (without resizing the file)"), metaObject()->className(), Pip3lineConst::LWARNING);
            length = repData.size();
        }

        if (length > fileSize) { // stupidity check
            emit log(tr("Replace length is greater than the file size, ignoring"), metaObject()->className(), Pip3lineConst::LERROR);
            return noError;
        }

        if ((quint64) (fileSize - length) < offset) { // stupidity check
            emit log(tr("Offset + length is outside the file, ignoring"), metaObject()->className(), Pip3lineConst::LERROR);
            return noError;
        }

        QByteArray before = extract(offset,repData.size());

        if (seekFile(offset)) {
            watcher.removePath(infoFile.absoluteFilePath());
            qint64 bytesWritten = file.write(repData);
            file.flush();
            if (bytesWritten < 0) {
                emit log(tr("Cannot write to file: %1").arg(file.errorString()), metaObject()->className(), Pip3lineConst::LERROR);
            } else if (bytesWritten == 0) {
                emit log(tr("No byte written to the file"), metaObject()->className(), Pip3lineConst::LWARNING);
            } else {
                noError = true;
                if (bytesWritten < repData.size()) {
                    emit log(tr("Was not able to write all the bytes to the file"), metaObject()->className(), Pip3lineConst::LWARNING);
                }
                historyAddReplace(offset,before,repData);
                emit updated(source);
            }
            watcher.addPath(infoFile.absoluteFilePath());
        }
    }

    return noError;
}


LargeFileSourceStateObj::LargeFileSourceStateObj(LargeFile *lf) :
    LargeRandomAccessSourceStateObj(lf)
{
    name = metaObject()->className();
}

LargeFileSourceStateObj::~LargeFileSourceStateObj()
{

}

void LargeFileSourceStateObj::internalRun()
{

    LargeFile *lf = static_cast<LargeFile *>(bs);
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeAttribute(GuiConst::STATE_LARGE_FILE_NAME, lf->fileName());
    } else {
        QXmlStreamAttributes attrList = reader->attributes();

        QString temp = attrList.value(GuiConst::STATE_LARGE_FILE_NAME).toString();
        if (!temp.isEmpty()) {
            lf->fromLocalFile(temp);
        }
    }

    // safe because we haven't change the current XML tag
    LargeRandomAccessSourceStateObj::internalRun();

}

#include "fileblocksource.h"
#include <QDebug>
#include <QTextStream>
#include <commonstrings.h>
#include "../../shared/guiconst.h"
#include "baseblocksourcewidget.h"
#include "filesourcewidget.h"

const QString FileBlockSource::ID = QString("File");
const int FileBlockSource::MIN_DELAY = 100;
const int FileBlockSource::MAX_DELAY = 10000;
const QString FileBlockSource::ID_SOURCE = QString("File source, reading only");
const QString FileBlockSource::ID_SINK = QString("File sink, writing only");

FileBlockSource::FileBlockSource(Type sourcefiletype, QObject *parent) :
    BlocksSource(parent),
    sourcefiletype(sourcefiletype)
{
    flags = 0;
    readDelay = 200;

    connect(&timer, &QTimer::timeout, this, &FileBlockSource::readNextBlock);
}

FileBlockSource::~FileBlockSource()
{
    if (file.isOpen()) {
        file.close();
    }
}

QString FileBlockSource::getName()
{
    return ID;
}

QString FileBlockSource::getDescription()
{
    if (sourcefiletype == Reader) {
        return ID_SOURCE;
    } else {
        return ID_SINK;
    }
}

bool FileBlockSource::isStarted()
{
    return false;
}

QHash<QString, QString> FileBlockSource::getConfiguration()
{
    QHash<QString, QString> ret = BlocksSource::getConfiguration();
    ret.insert(GuiConst::STATE_FILE_NAME, filename);
    ret.insert(GuiConst::STATE_TYPE, QString::number(sourcefiletype));
    ret.insert(GuiConst::STATE_DELAY, QString::number(readDelay));
    return ret;
}

void FileBlockSource::setConfiguration(const QHash<QString, QString> &conf)
{
    BlocksSource::setConfiguration(conf);

    if (conf.contains(GuiConst::STATE_FILE_NAME)) {
        filename = conf.value(GuiConst::STATE_FILE_NAME);
    }

    if (conf.contains(GuiConst::STATE_TYPE)) {
        bool ok = false;
        int val = conf.value(GuiConst::STATE_TYPE).toInt(&ok);
        if (ok && (val == FileBlockSource::Reader || val == FileBlockSource::Writer)) {
            sourcefiletype = static_cast<FileBlockSource::Type>(val);
        }
    }

    if (conf.contains(GuiConst::STATE_DELAY)) {
        bool ok = false;
        int val = conf.value(GuiConst::STATE_DELAY).toInt(&ok);
        if (ok && (val > MIN_DELAY && val < MAX_DELAY)) {
            readDelay = val;
        }
    }
}

QWidget *FileBlockSource::requestGui(QWidget *parent)
{
    BaseBlockSourceWidget * base = static_cast<BaseBlockSourceWidget *>(BlocksSource::requestGui(parent));
    if (base == nullptr) {
        qCritical() << tr("[IPBlocksSources::requestGui] base widget is nullptr T_T");
        return nullptr;
    }

    FileSourceWidget * fs = new(std::nothrow) FileSourceWidget(this, base);

    base->insertWidgetInGeneric(0, fs);

    return base;
}

void FileBlockSource::readNextBlock()
{
    if (sourcefiletype == Reader && startListening()) {
        QByteArray block;

        if (stream.atEnd()) {
            return; // nothing to see here
        }

        block = stream.readLine().toUtf8();

        if (!block.isEmpty()) {
            processIncomingDataBlock(block);
        }
    } else {
        emit log(tr("File sink cannot be used for reading !"), ID, Pip3lineConst::LERROR);
    }
}

void FileBlockSource::processIncomingDataBlock(QByteArray &data)
{
    data = applyInboundTransform(data);

    if (!data.isEmpty()) {
        Block * datab = new(std::nothrow) Block(data,sid);
        if (datab == nullptr)
            qFatal("[PipeClientListener::dataReceived] Cannot allocate Block for PipeClientListener X{");
        emit blockReceived(datab);
    } else {
        emit log(tr("[::processIncomingDataBlock] Incoming data block is empty (after potential B64 decode or transform)"), ID, Pip3lineConst::LWARNING);
    }
}

int FileBlockSource::getReadDelay() const
{
    return readDelay;
}

void FileBlockSource::setReadDelay(int value)
{
    readDelay = value;
}

FileBlockSource::Type FileBlockSource::getSourcefiletype() const
{
    return sourcefiletype;
}

void FileBlockSource::setSourcefiletype(const Type &value)
{
    sourcefiletype = value;
}

QString FileBlockSource::getFilename() const
{
    return filename;
}

void FileBlockSource::sendBlock(Block *block)
{
    if (sourcefiletype == Writer && startListening()) {
        QByteArray data = block->getData();
        if (isB64Blocks()) {
            data = QByteArray::fromBase64(data);
        }

        data = applyOutboundTransform(data);

        stream << data << Qt::endl;
        stream.flush();
    } else {
        emit log(tr("File source cannot be used for writing !"), ID, Pip3lineConst::LERROR);
    }
}

bool FileBlockSource::startListening()
{
    if (!running) {
        file.setFileName(filename);
        if (sourcefiletype == Writer) {
            if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
                emit log(QString("[%1] : %2").arg(filename).arg(file.errorString()), ID, Pip3lineConst::LERROR);
            } else {
                running = true;
                stream.setDevice(&file);
            }
            // nothing else to do just wait for data to arrive
        } else {
            if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
                emit log(QString("[%1] : %2").arg(filename).arg(file.errorString()), ID, Pip3lineConst::LERROR);
            } else {
                running = true;
                stream.setDevice(&file);
                // need to launch the read cycle

                timer.start(readDelay);
            }
        }
    }

    return running;
}

void FileBlockSource::stopListening()
{
    if (running) {
        timer.stop();
        file.close();
        running = false;
    }
}

void FileBlockSource::setFilename(const QString &value)
{
    filename = value;
}



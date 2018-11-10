#include "sharedmemposix.h"
#include "sharedmemposixwidget.h"
#include "shared/guiconst.h"

#ifdef Q_OS_UNIX
const QString SharedMemPosix::ID = "Shared Mem POSIX";
const int SharedMemPosix::MAX_FILE_NAME = NAME_MAX;
#elif defined(Q_OS_WIN)
#include <windows.h>
const QString SharedMemPosix::ID = "Shared Mem";
const int SharedMemPosix::MAX_FILE_NAME = MAX_PATH;
#endif

SharedMemPosix::SharedMemPosix() :
    SharedmemConnector(SharedmemConnector::POSIX_NATIVE_SHM)
{
    size = 4096;
}

SharedMemPosix::~SharedMemPosix()
{

}

QString SharedMemPosix::name()
{
    return ID;
}

bool SharedMemPosix::connectToMem()
{
    bool ret = false;
    if (!key.isEmpty()) {
        mem.setNativeKey(key);
        if (!mem.create(size, QSharedMemory::ReadWrite)) {
            if (mem.error() == QSharedMemory::AlreadyExists) {
                if (!mem.attach(QSharedMemory::ReadWrite)) {
                    emit log(tr("Attach: %1 %2").arg(mem.errorString()).arg(mem.error()), ID, Pip3lineConst::LERROR);
                }
            } else {
                emit log(tr("Create: %1").arg(mem.errorString()), ID, Pip3lineConst::LERROR);
            }
        } else {
            emit log(tr("Created [%1]").arg(key), ID, Pip3lineConst::PLSTATUS);
        }

        if (mem.isAttached()) {
            int cs = mem.size();
            emit log(tr("Attached to [%1 / %2]").arg(key).arg(cs), ID, Pip3lineConst::PLSTATUS);
            ret = true;
            if (cs != size) {
                //overriding the size to the effective one
                emit log("Overriding the Shared memory segment's size to the effective one.", ID, Pip3lineConst::PLSTATUS);
                size = cs;
            }
        }
    } else {
        emit log(tr("Key is empty"), ID, Pip3lineConst::LERROR);
    }

    return ret;
}

bool SharedMemPosix::readData(QByteArray &data)
{
    bool ret = false;
    if (mem.isAttached()) {
        // using native key, no point in locking
        int finalSize = mem.size();

        data.resize(finalSize);

        char * mpoin = reinterpret_cast<char *>(mem.data());
        memcpy(data.data(), mpoin, static_cast<size_t>(finalSize));

        ret = true;
    }
    return ret;
}

void SharedMemPosix::writeData(const QByteArray &data)
{
    if (mem.isAttached()) {
        int finalSize;
        // using native key, no point in locking
        if (data.size() > mem.size()) {
            emit log("Input data too large, truncating", ID, Pip3lineConst::LWARNING);
            finalSize = mem.size();
        } else {
            finalSize = data.size();
        }

        char * mpoin = reinterpret_cast<char *>(mem.data());
        memcpy(mpoin, data.constData(), static_cast<size_t>(finalSize));
    }
}

void SharedMemPosix::disconnectFromMem()
{
    if (mem.isAttached()) {
        // The return value has only meaning when using locks, and we aren't
        mem.detach();
    }
}

int SharedMemPosix::getSize() const
{
    return size;
}

void SharedMemPosix::setSize(int nsize)
{
    if (size != nsize) {
        // The size is only useful when creating a memory segment
        disconnectFromMem();
        size = nsize;
        connectToMem();
    }
}

QString SharedMemPosix::getKey() const
{
    return key;
}

void SharedMemPosix::setKey(const QString &value)
{
    qDebug() << tr("Key %1").arg(value);

    if (value != key) {
        key = value;
    }
}

QHash<QString, QString> SharedMemPosix::getConfiguration()
{
    QHash<QString, QString> conf = SharedmemConnector::getConfiguration();
    conf.insert(GuiConst::STATE_KEY, key);
    conf.insert(GuiConst::STATE_SIZE, QString::number(size));
    return conf;
}

void SharedMemPosix::setConfiguration(const QHash<QString, QString> &conf)
{
     SharedmemConnector::setConfiguration(conf);

     if (conf.contains(GuiConst::STATE_KEY)) {
         QString val = conf.value(GuiConst::STATE_KEY);
         if (val.size() < MAX_FILE_NAME) {
             key = val;
         } else {
             emit log("Key value is too large, ignoring", ID, Pip3lineConst::LERROR);
         }

     }

     if (conf.contains(GuiConst::STATE_SIZE)) {
         bool ok = false;
         int val = conf.value(GuiConst::STATE_SIZE).toInt(&ok);
         if (val) {
             size = val;
         }
     }
}

QString SharedMemPosix::getCurrentID()
{
    QString ret;
    if (mem.isAttached()) {
        ret = QString("[%1] %2").arg(key).arg(size);

    }  else {
        ret = QString("[%1] %2 (detached)").arg(key).arg(size);
    }
    return ret;
}

QWidget *SharedMemPosix::internalGetGui(QWidget *parent)
{
    SharedMemPosixWidget *ssw = new(std::nothrow) SharedMemPosixWidget(this, parent);
    if (ssw == nullptr)
        qFatal("Cannot allocate memory");
    return ssw;
}

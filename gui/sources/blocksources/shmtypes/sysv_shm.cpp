#include "sysv_shm.h"

#ifdef Q_OS_UNIX
#include <QDebug>
#include <QFile>
#include "shmsysvwidget.h"
#include "shared/guiconst.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

const QString SysV_Shm::ID = "System V Shared Mem";

SysV_Shm::SysV_Shm() :
    SharedmemConnector(SharedmemConnector::SYSV_SHM)
{
    shmid = -1;
    key = 0; // IPC_PRIVATE
    shm = nullptr;
    perms = 0600;
    size = getpagesize();
}

SysV_Shm::~SysV_Shm()
{
    disconnectFromMem();
    shmid = 0;
    key = 0;
}

QString SysV_Shm::name()
{
    return SysV_Shm::ID;
}

bool SysV_Shm::connectToMem()
{
    int ret  = 0;
    lastError.clear();
    if (key == 0) { // IPC_PRIVATE
        ret = shmget(key, static_cast<size_t>(size), IPC_CREAT | perms);
    } else { // either we connect to an existing key, or create a new one
        if ((ret = shmget(key, static_cast<size_t>(size), IPC_CREAT | IPC_EXCL | perms)) == -1) {
            if (errno == EEXIST) {
                emit log(tr("Memory segment already exist, try attaching to it"), this->metaObject()->className(), Pip3lineConst::PLSTATUS);
                ret = shmget(key, static_cast<size_t>(size), perms);
            }
        }
    }

    if (ret == -1) {
        switch(errno) {
            case EACCES:
                setErrorMessage(tr("shmget: access denied [%1 / %2]")
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key));
                break;
            case EINVAL:
                setErrorMessage(tr("shmget: Requested size (%3) does not match the requested shm's size [%1 / %2]")
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key)
                           .arg(size));
                break;
            case ENFILE:
                setErrorMessage(tr("shmget: system-wide file limit was reached [%1 / %2]")
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key));
                break;
            case ENOMEM:
                setErrorMessage(tr("shmget: memory could not be allocated [%1 / %2]")
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key));
                break;
            case ENOENT:
                setErrorMessage(tr("shmget: need IPC_CREAT when creating a new segment [%1 / %2] T_T")
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key));
                break;
            case ENOSPC:
                setErrorMessage(tr("shmget: all shm id have been exhausted [%1 / %2]")
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key));
                break;
            case EPERM:
                setErrorMessage(tr("shmget: not enough privileges for HUGETLB request [%1 / %2] T_T")
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key));
                break;
            default:
                setErrorMessage(tr("shmget: unmanaged shmget error %1 [%2 / %3] T_T")
                           .arg(errno)
                           .arg(QString("0x%1").arg(key , 0, 16))
                           .arg(key));
        }

        return false;
    }

    shmid = ret;

    if (shmid == -1) {
        emit log(QString("shmid is invalid"), this->metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    } else {
        emit log(QString("Got shmid: %1").arg(shmid), this->metaObject()->className(), Pip3lineConst::PLSTATUS);
    }

    if ((shm = reinterpret_cast<char *>(shmat(shmid, nullptr, 0))) == reinterpret_cast<char *>(-1)) {
        switch (errno) {
            case EACCES:
                setErrorMessage(tr("shamt: Access Denied [%1 / %2]")
                           .arg(QString("0x%1").arg(shmid , 0, 16))
                           .arg(shmid));
                break;
            case EIDRM:
                setErrorMessage(tr("shamt: shmid does not exist anymore [%1 / %2]")
                           .arg(QString("0x%1").arg(shmid , 0, 16))
                           .arg(shmid));
                break;
            case EINVAL:
                setErrorMessage(tr("shamt: shmid is invalid [%1 / %2]")
                           .arg(QString("0x%1").arg(shmid , 0, 16))
                           .arg(shmid));
                break;
            case ENOMEM:
                setErrorMessage(tr("shamt: Could not allocate memory [%1 / %2]")
                           .arg(QString("0x%1").arg(shmid , 0, 16))
                           .arg(shmid));
                break;
            default:
               setErrorMessage(tr("unmanaged shmat error %1 [%2 / %3] T_T")
                          .arg(errno)
                          .arg(QString("0x%1").arg(shmid , 0, 16))
                          .arg(shmid));
        }

        return false;
    }

    emit log(QString("Attached to %1").arg(shmid), this->metaObject()->className(), Pip3lineConst::PLSTATUS);

    return true;
}

void SysV_Shm::writeData(const QByteArray &data)
{
    if (shm != nullptr) {
        memcpy(shm,data.constData(), static_cast<size_t>(qMin(size,data.size())));
    }
}

bool SysV_Shm::readData(QByteArray &data)
{
    bool ret = false;
    if (shm != nullptr) {
        data = QByteArray(shm, size);
        ret = true;
    }

    return ret;
}

void SysV_Shm::disconnectFromMem()
{
    if (shm != nullptr) {
        int ret = shmdt(shm);
        if (ret < 0 ) {
            switch (errno) {
                case EINVAL:
                    emit log(tr("shmdt: Invalid address [0x%1]")
                               .arg(QString("0x%1").arg(reinterpret_cast<quintptr>(shm) , 0, 16)), this->metaObject()->className(), Pip3lineConst::LERROR);
                    break;
                default:
                   emit log(tr("shmdt: unmanaged error %1 [0x%2] T_T")
                              .arg(errno)
                              .arg(QString("0x%1").arg(reinterpret_cast<quintptr>(shm) , 0, 16)), this->metaObject()->className(), Pip3lineConst::LERROR);
            }
        }

        shm = nullptr;
    }
}

QList<SysV_Shm::ShmData> SysV_Shm::getSHMList()
{
    QString ProcFile = QString("/proc/sysvipc/shm");

    QList<SysV_Shm::ShmData> ret;
    QFile shmFile(ProcFile);
    if (shmFile.open(QIODevice::ReadOnly)) {
        QList<QByteArray> lines = shmFile.readAll().split('\n');
        for (int i = 0; i < lines.size(); i++) {
            QList<QByteArray> tokens = lines.at(i).split(' ');
            if (tokens.size() > 0) {
                bool ok = false;
                qint32 key = tokens.at(0).trimmed().toInt(&ok);
                if (! ok) {
                    continue;
                }

                if (tokens.size() >= 16) {
                    ShmData data;
                    data.key = key;
                    data.shmid = tokens.at(1).trimmed().toInt();
                    data.perms = QString::fromUtf8(tokens.at(2).trimmed());
                    data.size = tokens.at(1).trimmed().toUInt();
                    data.cpid = tokens.at(1).trimmed().toUInt();
                    data.lpid = tokens.at(1).trimmed().toUInt();
                    data.nattch = tokens.at(1).trimmed().toUInt();
                    data.uid = tokens.at(1).trimmed().toUInt();
                    data.gid = tokens.at(1).trimmed().toUInt();
                    data.cuid = tokens.at(1).trimmed().toUInt();
                    data.cgid = tokens.at(1).trimmed().toUInt();
                    data.atime = tokens.at(1).trimmed().toULongLong();
                    data.dtime = tokens.at(1).trimmed().toULongLong();
                    data.ctime = tokens.at(1).trimmed().toULongLong();
                    data.rss = tokens.at(1).trimmed().toUInt();
                    data.swap = tokens.at(1).trimmed().toUInt();

                    ret.append(data);
                }
            }
        }
    } else {
        qCritical() << QString("[SharedmemConnector::getSHMList] cannot open the shm proc file: %1").arg(shmFile.errorString());
    }

    return ret;
}

void SysV_Shm::setSize(int nsize)
{
    int page_size = getpagesize();
    if (nsize % page_size != 0) {
        emit log(tr("Size (%1) not aligned with system PAGE_SIZE (%2)").arg(nsize).arg(page_size), this->metaObject()->className(), Pip3lineConst::LWARNING);
    }

    size = nsize;
}

key_t SysV_Shm::getKey() const
{
    return key;
}

void SysV_Shm::setKey(const key_t &value)
{
    key = value;
}

int SysV_Shm::getShmid() const
{
    return shmid;
}

void SysV_Shm::setShmid(int value)
{
    shmid = value;
}

QHash<QString, QString> SysV_Shm::getConfiguration()
{
    QHash<QString, QString> conf = SharedmemConnector::getConfiguration();
    if (!filename.isEmpty()) {
        conf.insert(GuiConst::STATE_FILE_NAME, filename);
    } else {
        conf.insert(GuiConst::STATE_KEY, QString::number(key));
        conf.insert(GuiConst::STATE_SHMID, QString::number(shmid));
    }

    conf.insert(GuiConst::STATE_SIZE, QString::number(size));
    return conf;
}

void SysV_Shm::setConfiguration(const QHash<QString, QString> &conf)
{
    SharedmemConnector::setConfiguration(conf);

    if (conf.contains(GuiConst::STATE_FILE_NAME)) {
        QString val = conf.value(GuiConst::STATE_FILE_NAME);
        if (!val.isEmpty()) {
            setFilename(val);
        }
    }

    if (filename.isEmpty()) { // no need for the rest if the filename is specified
        if (conf.contains(GuiConst::STATE_KEY)) {
            bool ok = false;
            int val = conf.value(GuiConst::STATE_KEY).toInt(&ok);
            if (ok) {
                key = val;
            } else {
                emit log("Key value invalid, ignoring", ID, Pip3lineConst::LERROR);
            }
        }

        if (conf.contains(GuiConst::STATE_SHMID)) {
            bool ok = false;
            int val = conf.value(GuiConst::STATE_SHMID).toInt(&ok);
            if (ok) {
                shmid = val;
            } else {
                emit log("Shmid value invalid, ignoring", ID, Pip3lineConst::LERROR);
            }
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

QWidget *SysV_Shm::internalGetGui(QWidget *parent)
{
    ShmSysVWidget *ssw = new(std::nothrow) ShmSysVWidget(this, parent);
    if (ssw == nullptr)
        qFatal("Cannot allocate memory");

    connect(ssw, &ShmSysVWidget::requestReset, this, &SysV_Shm::reset);
    return ssw;
}

QString SysV_Shm::getFilename() const
{
    return filename;
}

void SysV_Shm::setFilename(const QString &value)
{
    lastError.clear();
    if (value.isEmpty())
        return;

    int tkey = ftok(value.toUtf8().constData(), 'R');
    if (tkey < 0) {
        switch (errno) {
            case EACCES:
                setErrorMessage(tr("File Access Denied [%1]").arg(value));
                break;
            case ELOOP:
                setErrorMessage(tr("Too many symbolic links [%1]").arg(value));
                break;
            case ENAMETOOLONG:
                setErrorMessage(tr("File path name is too long [%1]").arg(value));
                break;
            case ENOENT:
                setErrorMessage(tr("File or Path does not exist [%1]").arg(value));
                break;
            case ENOMEM:
                setErrorMessage(tr(" Out of Memory [%1]").arg(value));
                break;
            case ENOTDIR:
                setErrorMessage(tr("[Path is not a path ... [%1]").arg(value));
                break;
            case EOVERFLOW:
                setErrorMessage(tr("Overflow (yay!), are you running the 32bits binary on a 64 bits (file)system ? [%1]").arg(value));
                break;
            default:
                setErrorMessage(tr("ftok unmanaged error [%1]").arg(value));
        }
    } else {
       filename = value;
       key = tkey;
       shmid = 0;
    }
}

QString SysV_Shm::getCurrentID()
{
    QString ret;
    if (shm != nullptr) {
        ret = QString("[%1/%2] %3").arg(key,0,16).arg(shmid).arg(reinterpret_cast<quintptr>(shm), 0,16);
    } else {
        ret = QString("Not attached");
    }

    return ret;
}

int SysV_Shm::getPerms() const
{
    return perms;
}

void SysV_Shm::setPerms(int value)
{
    perms = value;
}

int SysV_Shm::getSize() const
{
    return size;
}

#endif

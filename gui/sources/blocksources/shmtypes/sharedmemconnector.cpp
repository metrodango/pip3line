#include "sharedmemconnector.h"
#include <QWidget>
#include "shared/guiconst.h"

SharedmemConnector::SharedmemConnector(const MemoryAccessType &type) :
    QObject(nullptr),
    gui(nullptr),
    type(type)
{
    qDebug() << "type" << type;
}

SharedmemConnector::~SharedmemConnector()
{
    delete gui;
    gui = nullptr;
}

QHash<QString, QString> SharedmemConnector::getConfiguration()
{
    QHash<QString, QString> ret;
    ret.insert(GuiConst::STATE_TYPE, QString::number(type));
    return ret;
}

void SharedmemConnector::setConfiguration(const QHash<QString, QString> &conf)
{
    Q_UNUSED(conf) // nothing to set here, the type was auto set during instanciation
}

QWidget *SharedmemConnector::getGui(QWidget *parent)
{
    if (gui == nullptr) {
        gui = internalGetGui(parent);
        if (gui != nullptr) {
            connect(gui, &SharedmemConnector::destroyed, this , &SharedmemConnector::onGuiDestroyed);
        }
    }

    return gui;
}

void SharedmemConnector::onGuiDestroyed()
{
    gui = nullptr;
}

void SharedmemConnector::setErrorMessage(const QString &message)
{
    emit log(message, this->metaObject()->className(), Pip3lineConst::LERROR);
    if (!lastError.isEmpty())
        lastError.append(" | ");
    lastError.append(message);
}

QString SharedmemConnector::getLastError() const
{
    return lastError;
}

bool SharedmemConnector::hasError() const
{
    return !lastError.isEmpty();
}

SharedmemConnector::MemoryAccessType SharedmemConnector::getType() const
{
    return type;
}

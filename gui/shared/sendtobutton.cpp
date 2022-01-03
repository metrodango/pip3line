#include "sendtobutton.h"
#include <QDebug>
#include "tabs/tababstract.h"
#include "sources/blocksources/blockssource.h"
#include "tabs/packetanalyser/sourcesorchestatorabstract.h"
#include "guihelper.h"
#include "targetswidget.h"
#include "defaultdialog.h"
#include <pipelinecommon.h>
using namespace Pip3lineCommon;

SendToModel::SendToModel(GuiHelper *guiHelper, QObject *parent):
    QAbstractTableModel(parent),
    guiHelper(guiHelper)
{
    sendToTabList = guiHelper->getTabs();
    sendToOrchestratorList = guiHelper->getRegisteredOrchestrators();
    sendToBlockSourceList = guiHelper->getRegisteredBlockSources();
    connect(guiHelper, &GuiHelper::tabsUpdated, this, &SendToModel::updateTabs,Qt::QueuedConnection);
    connect(guiHelper, &GuiHelper::registeredBlockSourcesUpdated, this, &SendToModel::updateBlockSource,Qt::QueuedConnection);
    connect(guiHelper, &GuiHelper::registeredOrchestratorsUpdated, this, &SendToModel::updateOrchestrator, Qt::QueuedConnection);
}

SendToModel::~SendToModel()
{

}

int SendToModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return 2;
    }
}

int SendToModel::rowCount(const QModelIndex &parent) const
{
    int ret = 0;

    if (!parent.isValid()) {
        ret = sendToTabList.size() + sendToBlockSourceList.size() + sendToOrchestratorList.size();
    }
    return ret;
}

QVariant SendToModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if (index.isValid()) {
        int row = index.row();
        int col = index.column();
        SendToModel::RowType type = translateRow(row);
        if (role == Qt::DisplayRole) {
            if (type == Orchestrator) {
                if (col == 0) {
                    ret = sendToOrchestratorList.at(row)->getName().trimmed();
                } else if (col == 1) {
                    ret = QString("Connections: %1").arg(sendToOrchestratorList.at(row)->getAvailableConnections().size());
                }
            } else if (type == Blocksource) {
                if (col == 0) {
                    ret = sendToBlockSourceList.at(row)->getName();
                } else if (col == 1) {
                    ret = sendToBlockSourceList.at(row)->getDescription();
                }
            } else if (type == Tab) {
                if (col == 0) {
                    ret = sendToTabList.at(row)->getName();
                }  else if (col == 1) {
                    ret = QString("Tab");
                }
            } else {
                ret = QString("Invalid");
            }

        }
    }
    return ret;
}

QVariant SendToModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return QString("target");
            case 1:
                return QString ("Info");

        }
    }
    return QVariant();
}

void SendToModel::updateTabs()
{
    beginResetModel();
    sendToTabList.clear();
    sendToTabList = guiHelper->getTabs();
    endResetModel();
}

void SendToModel::updateBlockSource()
{
    beginResetModel();
    sendToBlockSourceList.clear();
    sendToBlockSourceList = guiHelper->getRegisteredBlockSources();
    endResetModel();
}

void SendToModel::updateOrchestrator()
{
    beginResetModel();
    sendToOrchestratorList.clear();
    sendToOrchestratorList = guiHelper->getRegisteredOrchestrators();
    endResetModel();
}

SendToModel::RowType SendToModel::translateRow(int &row) const
{
    if (row < sendToOrchestratorList.size()) {
        return Orchestrator;
    } else if (row < sendToOrchestratorList.size() + sendToBlockSourceList.size()) {
        row = row - sendToOrchestratorList.size();
        return Blocksource;
    } else if (row < sendToOrchestratorList.size() + sendToBlockSourceList.size() + sendToTabList.size()) {
        row = row - sendToOrchestratorList.size() - sendToBlockSourceList.size();
        return Tab;
    } else {
        return Invalid;
    }
}

SendToButton::SendToButton(GuiHelper *guiHelper, QWidget *parent) :
    QPushButton(parent),
    guiHelper(guiHelper),
    targetsDialog(nullptr),
    targetsControl(nullptr)
{
    setIcon(QIcon(":/Images/icons/icons8-curly-arrow-48.png"));
    setMaximumWidth(25);
    setFlat(true);

    connect(this, &QPushButton::clicked, this, &SendToButton::onClicked);

    model = new(std::nothrow) SendToModel(guiHelper);
    if (model == nullptr) {
        qFatal("[SendToButton::SendToButton] Cannot allocate memory for SendToButton X{");
    }
}

SendToButton::~SendToButton()
{
    delete model;
}

void SendToButton::onClicked()
{
    if (targetsDialog == nullptr) {
        targetsDialog = new(std::nothrow) DefaultDialog(this);
        if (targetsDialog == nullptr) {
            qFatal("SendToButton::onClicked Cannot allocate memory for DefaultDialog X{");
        }
        targetsDialog->setWindowTitle("Potential receivers");
        targetsControl = new(std::nothrow) TargetsWidget(model, targetsDialog);
        if (targetsControl == nullptr) {
            qFatal("SendToButton::onClicked Cannot allocate memory for TargetsWidget X{");
        }

        targetsDialog->setMainWidget(targetsControl);

        int ret = targetsDialog->exec();
        if (ret == QDialog::Accepted) {
            targetsControl->onSendData(QByteArray("test"));
        }
    } else {
        targetsDialog->show();
        targetsDialog->raise();
        targetsDialog->activateWindow();
        targetsControl->onSendData(QByteArray("test"));
    }

    // if a target has been defined, send payload to the target
    // otherwise open the dialog to select a target
}


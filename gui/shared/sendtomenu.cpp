#include "sendtomenu.h"
#include "tabs/tababstract.h"
#include "sources/blocksources/blockssource.h"
#include "tabs/packetanalyser/sourcesorchestatorabstract.h"
#include "tabs/packetanalyser/packet.h"
#include "guihelper.h"
#include <QAction>
#include <QList>
#include <QDebug>

SendToMenu::SendToMenu(GuiHelper *guiHelper, QString title, QWidget *parent) :
    QMenu(title,parent),
    guiHelper(guiHelper)
{

    sendToNewTabAction = new(std::nothrow) QAction("Send to New Tab", nullptr);
    if (sendToNewTabAction == nullptr) {
        qFatal("Cannot allocate memory for sendToNewTabAction X{");
    }

    sendToNewHexEditorAction = new(std::nothrow) QAction("Send to New Hex Editor", nullptr);
    if (sendToNewHexEditorAction == nullptr) {
        qFatal("Cannot allocate memory for sendToNewHexEditorAction X{");
    }

    update();

    connect(guiHelper, &GuiHelper::tabsUpdated, this, &SendToMenu::update,Qt::QueuedConnection);
    connect(guiHelper, &GuiHelper::registeredBlockSourcesUpdated, this, &SendToMenu::update,Qt::QueuedConnection);
    connect(guiHelper, &GuiHelper::registeredOrchestratorsUpdated, this, &SendToMenu::update, Qt::QueuedConnection);
}

SendToMenu::~SendToMenu()
{
    delete sendToNewTabAction;
    delete sendToNewHexEditorAction;
}

void SendToMenu::update()
{
    QAction * action = nullptr;
    for (int i = 0; i < subMenus.size(); i++) {
        delete subMenus.at(i);
    }
    subMenus.clear();

    clear(); // actions created on the fly should be automatically deleted
    // clearing mappings
    sendToTabMapping.clear();
    sendToBlockSourceMapping.clear();
    sendToOrchestratorMapping.clear();
    addAction(sendToNewTabAction);
    addAction(sendToNewHexEditorAction);

    QList<TabAbstract *> list = guiHelper->getTabs();
    addSeparator();
    if (list.size() > 0) {
        for (int i = 0; i < list.size(); i++) {
            TabAbstract * tab = list.at(i);
            if (tab->getPreTabType() != GuiConst::INVALID_PRETAB) {
                if (tab->canReceiveData()) {
                    action = new(std::nothrow) QAction(tab->getName().trimmed(),this);
                    if (action == nullptr) {
                        qFatal("Cannot allocate memory for action updateSendToMenu X{");
                    }
                    sendToTabMapping.insert(action, tab);
                    addAction(action);
                }
            }
        }
    }

    QList<BlocksSource *> blist = guiHelper->getRegisteredBlockSources();
    if (blist.size() > 0) {
        addSeparator();
        for (int i = 0; i < blist.size(); i++) {
            BlocksSource * bs = blist.at(i);
            QList<Target<BlocksSource *> > conns = bs->getAvailableConnections();
            if (conns.size() == 0 )
                continue; // nothing to see here
            if (conns.size() == 1) {
                Target<BlocksSource *> single = conns.at(0);
                // single is garanteed to have at least one item
                action = new(std::nothrow) QAction(single.getDescription(),this);
                if (action == nullptr) {
                    qFatal("Cannot allocate memory for action updateSendToMenu X{");
                }

                sendToBlockSourceMapping.insert(action,single);
                addAction(action);
            } else {
                QMenu * newMenu = new(std::nothrow) QMenu(bs->getName(),this);
                if (newMenu == nullptr) {
                    qFatal("Cannot allocate memory for QMenu Orchestrator X{");
                }

                for (int i = 0; i < conns.size(); i++) {
                    action = new(std::nothrow) QAction(conns.at(i).getDescription(),this);
                    if (action == nullptr) {
                        qFatal("Cannot allocate memory for action updateSendToMenu X{");
                    }

                    sendToBlockSourceMapping.insert(action,conns.at(i));
                    newMenu->addAction(action);
                }

                addMenu(newMenu);
                subMenus.append(newMenu);
            }
        }
    }

    QList<SourcesOrchestatorAbstract *> olist = guiHelper->getRegisteredOrchestrators();
    if (olist.size() > 0) {
        addSeparator();
        for (int i = 0; i < olist.size(); i++) {
            SourcesOrchestatorAbstract *orch = olist.at(i);
            QList<Target<SourcesOrchestatorAbstract *> > conns = orch->getAvailableConnections();
            if (conns.size() == 0 )
                continue; // nothing to see here

            QMenu * newMenu = new(std::nothrow) QMenu(orch->getName(),this);
            if (newMenu == nullptr) {
                qFatal("Cannot allocate memory for QMenu Orchestrator X{");
            }

            for (int i = 0; i < conns.size(); i++) {
                action = new(std::nothrow) QAction(conns.at(i).getDescription(),this);
                if (action == nullptr) {
                    qFatal("Cannot allocate memory for action updateSendToMenu X{");
                }

                sendToOrchestratorMapping.insert(action,conns.at(i));
                newMenu->addAction(action);
                if (action->text() == BlocksSource::NEW_CONNECTION_STRING) {
                    // there should be only one , hopefully -_-
                    newMenu->addSeparator();
                }
            }

            addMenu(newMenu);
            subMenus.append(newMenu);
        }
    }
}

void SendToMenu::processingAction(QAction *action, const QByteArray &data)
{
    if (action == sendToNewTabAction) {
        guiHelper->sendToNewTab(data);
    } else if (action == sendToNewHexEditorAction) {
        guiHelper->sendToNewHexEditor(data);
    } else {
        if (sendToTabMapping.contains(action)) {
            TabAbstract * tg = sendToTabMapping.value(action);
            tg->setData(data);
            tg->bringFront();
        } else if (sendToBlockSourceMapping.contains(action)) {
            Target<BlocksSource *> ta = sendToBlockSourceMapping.value(action);
         //   qDebug() << "sending to " << ta.getSource()->getName() << ta.getDescription();
            Block * datab = new(std::nothrow) Block(data,ta.getConnectionID());
            if (datab == nullptr) qFatal("Cannot allocate Block for sendTo X{");
            ta.getSource()->postBlockForSending(datab);
        } else if (sendToOrchestratorMapping.contains(action)) {
            Target<SourcesOrchestatorAbstract *> ta = sendToOrchestratorMapping.value(action);
        //    qDebug() << "sending to " << ta.getSource()->getName() << ta.getDescription();
            QSharedPointer<Packet> datap = QSharedPointer<Packet> (new(std::nothrow) Packet(data,ta.getConnectionID()));
            if (datap == nullptr) qFatal("Cannot allocate Packet for sendTo X{");
            datap->setInjected(true);
            ta.getSource()->postPacket(datap);
        } else {
            qCritical() << tr("Action not found for sending T_T");
        }
    }
}

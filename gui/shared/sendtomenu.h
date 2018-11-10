#ifndef SENDTOMENU_H
#define SENDTOMENU_H

#include <QMenu>
#include <QHash>
#include "sources/blocksources/target.h"

class QAction;
class TabAbstract;
class BlocksSource;
class SourcesOrchestatorAbstract;
class GuiHelper;

class SendToMenu : public QMenu
{
        Q_OBJECT
    public:
        explicit SendToMenu(GuiHelper *guiHelper, QString title = QString(), QWidget *parent = nullptr);
        ~SendToMenu();
    public slots:
        void update();
        void processingAction(QAction *action, const QByteArray &data);
    signals:
        void newTabRequested(QByteArray data);
        void sendToRequest(QAction * action);

    private:
        Q_DISABLE_COPY(SendToMenu)
        GuiHelper *guiHelper;
        QHash<QAction *, TabAbstract *> sendToTabMapping;
        QHash<QAction *, Target<BlocksSource *> > sendToBlockSourceMapping;
        QHash<QAction *, Target<SourcesOrchestatorAbstract *> > sendToOrchestratorMapping;
        QAction * sendToNewTabAction;
        QAction * sendToNewHexEditorAction;
        QList<QMenu *> subMenus;
};

#endif // SENDTOMENU_H

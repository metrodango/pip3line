#ifndef NEWVIEWMENU_H
#define NEWVIEWMENU_H

#include <QObject>
#include <QMenu>
#include "tabs/tababstract.h"

class SingleViewAbstract;
class GuiHelper;
class ByteSourceAbstract;

class NewViewMenu : public QMenu
{
        Q_OBJECT
    public:
        explicit NewViewMenu(GuiHelper *guiHelper, QWidget *parent = 0);
        ~NewViewMenu();
        SingleViewAbstract *getView(ByteSourceAbstract *bytesource, QWidget *parent);
        TabAbstract::ViewTab getTabData() const;
        void setTabData(TabAbstract::ViewTab newdata);
    signals:
        void newViewRequested();
    private slots:
        void onNewViewTab(QAction * action);
    private:
        QAction * newHexViewAction;
        QAction * newTextViewAction;
        QAction * newDefaultTextViewAction;
        TabAbstract::ViewTab viewData;
        GuiHelper *guiHelper;
};

#endif // NEWVIEWMENU_H

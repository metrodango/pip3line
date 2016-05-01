#ifndef NEWVIEWBUTTON_H
#define NEWVIEWBUTTON_H

#include <QObject>
#include <QPushButton>
#include "tabs/tababstract.h"

class SingleViewAbstract;
class GuiHelper;
class ByteSourceAbstract;
class NewViewMenu;

class NewViewButton : public QPushButton
{
        Q_OBJECT
    public:
        static const QString TEXT_TEXT;
        explicit NewViewButton(GuiHelper *guiHelper, QWidget *parent = 0);
        ~NewViewButton();
        SingleViewAbstract *getView(ByteSourceAbstract *bytesource, QWidget *parent);
        TabAbstract::ViewTab getTabData() const;
        void setTabData(TabAbstract::ViewTab newdata);
    signals:
        void newViewRequested();
    private:
        NewViewMenu * menu;
};

#endif // NEWVIEWBUTTON_H

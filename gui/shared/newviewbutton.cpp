#include "newviewbutton.h"
#include <QMenu>
#include <QAction>
#include <QDebug>
#include "quickviewitemconfig.h"
#include "views/singleviewabstract.h"
#include "sources/intermediatesource.h"
#include "sources/bytesourceabstract.h"
#include "guihelper.h"
#include "views/hexview.h"
#include "views/textview.h"
#include "newviewmenu.h"

const QString NewViewButton::TEXT_TEXT = "Text";

NewViewButton::NewViewButton(GuiHelper *guiHelper, QWidget *parent) :
    QPushButton(parent)
{
    setIcon(QIcon(":/Images/icons/dialog-more.png"));
    setMaximumWidth(25);
    setFlat(true);

    menu = new(std::nothrow)NewViewMenu(guiHelper,this);
    if (menu == nullptr) {
        qFatal("Cannot allocate memory for NewViewMenu X{");
    }

    setMenu(menu);
    connect(menu, SIGNAL(newViewRequested()), this, SIGNAL(newViewRequested()));
}

NewViewButton::~NewViewButton()
{

}

SingleViewAbstract * NewViewButton::getView(ByteSourceAbstract *bytesource, QWidget * parent)
{
    return menu->getView(bytesource, parent);
}

TabAbstract::ViewTab NewViewButton::getTabData() const
{
    return menu->getTabData();
}

void NewViewButton::setTabData(TabAbstract::ViewTab newdata)
{
    menu->setTabData(newdata);
}


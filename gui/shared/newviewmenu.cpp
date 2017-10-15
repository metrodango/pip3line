#include "newviewmenu.h"
#include <QAction>
#include "quickviewitemconfig.h"
#include "views/singleviewabstract.h"
#include "sources/intermediatesource.h"
#include "sources/bytesourceabstract.h"
#include "guihelper.h"
#include <QDebug>
#include "views/hexview.h"
#include "views/textview.h"
#include "shared/guiconst.h"
#include "shared/transformguibutton.h"

NewViewMenu::NewViewMenu(GuiHelper *guiHelper, QWidget *parent) :
    QMenu(parent),
    guiHelper(guiHelper)
{
    setTitle(tr("New View"));
    QAction * descAction = new(std::nothrow)QAction(tr("View type"),this);
    if (descAction == nullptr) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    descAction->setDisabled(true);
    addAction(descAction);

    newHexViewAction = new(std::nothrow)QAction(GuiConst::HEXADECIMAL_STRING,this);
    if (newHexViewAction == nullptr) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    addAction(newHexViewAction);

    newTextViewAction = new(std::nothrow)QAction(GuiConst::TEXT_TEXT,this);
    if (newTextViewAction == nullptr) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    addAction(newTextViewAction);
    addSeparator();

    newDefaultTextViewAction = new(std::nothrow)QAction(tr("Default Text view"),this);
        if (newDefaultTextViewAction == nullptr) {
            qFatal("Cannot allocate memory for newDefaultTextViewAction X{");
        }
    addAction(newDefaultTextViewAction);

    connect(this,SIGNAL(triggered(QAction*)), SLOT(onNewViewTab(QAction*)));
}

NewViewMenu::~NewViewMenu()
{

}


SingleViewAbstract * NewViewMenu::getView(ByteSourceAbstract *bytesource, QWidget * parent)
{
    SingleViewAbstract * newView = nullptr;
    QPushButton * configButton = nullptr;
    switch (viewData.type) {
        case (TabAbstract::HEXVIEW) :
            {
                IntermediateSource * is = new(std::nothrow) IntermediateSource(guiHelper,bytesource,viewData.transform);
                if (is == nullptr) {
                    qFatal("Cannot allocate memory for IntermediateSource X{");
                }

                newView = new(std::nothrow) HexView(is,guiHelper,parent,true);
                if (newView == nullptr) {
                    qFatal("Cannot allocate memory for HexView X{");
                }
                configButton = new(std::nothrow) TransformGuiButton(viewData.transform);
                if (configButton == nullptr) {
                    qFatal("Cannot allocate memory for TransformGuiButton X{");
                }
            }
            break;
        case (TabAbstract::TEXTVIEW) :
            {
                IntermediateSource * is = new(std::nothrow) IntermediateSource(guiHelper,bytesource,viewData.transform);
                if (is == nullptr) {
                    qFatal("Cannot allocate memory for IntermediateSource X{");
                }

                newView = new(std::nothrow) TextView(is,guiHelper,parent,true);
                if (newView == nullptr) {
                    qFatal("Cannot allocate memory for TextView X{");
                }
                configButton = new(std::nothrow) TransformGuiButton(viewData.transform);
                if (configButton == nullptr) {
                    qFatal("Cannot allocate memory for TransformGuiButton X{");
                }
            }
            break;
        case (TabAbstract::DEFAULTTEXT) :
            {
                newView = new(std::nothrow) TextView(bytesource,guiHelper,parent);
                if (newView == nullptr) {
                    qFatal("Cannot allocate memory for TextView X{");
                }
            }
            break;
        default:
        {
            qCritical() << tr("[NewViewMenu::getView] View Type undefined");
        }
    }

    newView->setConfigButton(configButton);
    return newView;
}

TabAbstract::ViewTab NewViewMenu::getTabData() const
{
    return viewData;
}

void NewViewMenu::setTabData(TabAbstract::ViewTab newdata)
{
    viewData = newdata;
}

void NewViewMenu::onNewViewTab(QAction *action)
{
    TabAbstract::ViewTab vt;
    if (action == newDefaultTextViewAction) {  // usual Text View (no input transform)
        vt.type = TabAbstract::DEFAULTTEXT;
        vt.transform = nullptr;
        vt.tabName = GuiConst::TEXT_TEXT;
    } else {
        QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
        if (itemConfig == nullptr) {
            qFatal("Cannot allocate memory for QuickViewItemConfig X{");
        }
        itemConfig->setWayBoxVisible(true);
        itemConfig->setFormatVisible(false);
        itemConfig->setOutputTypeVisible(false);
        int ret = itemConfig->exec();
        if (ret == QDialog::Accepted) {
            TransformAbstract * ta = itemConfig->getTransform();
            vt.transform = ta;
            vt.tabName = itemConfig->getName();
            if (action == newHexViewAction) {
                vt.type = TabAbstract::HEXVIEW;
            } else if (action == newTextViewAction) {
                vt.type = TabAbstract::TEXTVIEW;
            } else {
                qWarning("New View Action not managed T_T");
                vt.type = TabAbstract::UNDEFINED;
            }

            delete itemConfig;
        } else { // action cancelled
            delete itemConfig;
            return;
        }
    }
    viewData = vt;

    emit newViewRequested();
}


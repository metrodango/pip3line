/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "universalreceiverbutton.h"
#include "../tabs/tababstract.h"
#include "../guihelper.h"
#include <QState>

CommonUniversalReceiverListener UniversalReceiverButton::staticListener;

UniversalReceiverButton::UniversalReceiverButton(TabAbstract *tab, GuiHelper *nguiHelper) :
    QPushButton(tab)
{
    setCheckable(true);
    setIcon(QIcon(":/Images/icons/object-rotate-right.png"));
    setMaximumWidth(25);
    setFlat(true);
    setToolTip(tr("Make this tab receiver for all \"Send to Pip3line\""));
    guiHelper = nguiHelper;
    attachedTab = tab;
    taken = false;
    connect(this, SIGNAL(clicked(bool)), SLOT(onClicked(bool)));
    connect(&staticListener, SIGNAL(taken()), SLOT(reset()));
    connect(this, SIGNAL(tookReceiver()), &staticListener, SLOT(onReceiverConfigured()), Qt::UniqueConnection);
}

UniversalReceiverButton::~UniversalReceiverButton()
{
    if (isChecked()){
        guiHelper->setUniveralReceiver(nullptr);
    }
}

void UniversalReceiverButton::reset()
{
    if (!taken) {
        setChecked(false);
    }
    taken = false;
}

void UniversalReceiverButton::onClicked(bool checked)
{
    if (checked) {
        guiHelper->setUniveralReceiver(attachedTab);
        taken = true;
        emit tookReceiver();
    } else {
        guiHelper->setUniveralReceiver(nullptr);
    }
}




CommonUniversalReceiverListener::CommonUniversalReceiverListener():
    QObject(0)
{

}

void CommonUniversalReceiverListener::onReceiverConfigured()
{
    emit taken();
}

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "detachtabbutton.h"
#include "../tabs/tababstract.h"

DetachTabButton::DetachTabButton(TabAbstract * tab) :
    QPushButton(tab)
{
    curtab = tab;
    setIcon(QIcon(":/Images/icons/tab-duplicate-2.png"));
    setFlat(true);
    setMaximumWidth(25);
    setToolTip(tr("Detach/attach tab"));
    connect(this, &DetachTabButton::clicked, curtab, &TabAbstract::onDetach);
}

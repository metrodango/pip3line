/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "clearallmarkingsbutton.h"
#include "../sources/bytesourceabstract.h"
#include <QKeySequence>

ClearAllMarkingsButton::ClearAllMarkingsButton(ByteSourceAbstract *bytesource, QWidget *parent) :
    QPushButton(parent)
{
    setIcon(QIcon(":/Images/icons/irc-close-channel.png"));
    setFlat(true);
    setMaximumWidth(25);
    setToolTip(tr("Clear all markings"));
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    byteSource = bytesource;
    connect(this, SIGNAL(clicked()), byteSource, SLOT(clearAllMarkings()));
}

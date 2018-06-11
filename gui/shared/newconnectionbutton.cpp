#include "newconnectionbutton.h"

NewConnectionButton::NewConnectionButton(QWidget *parent) :
    QPushButton(parent)
{
    setFlat(true);
    setMaximumWidth(24);
    setIcon(QIcon(":/Images/icons/go-last-2.png"));
    setToolTip(tr("Initiate new connection (without sending data)"));
}

NewConnectionButton::~NewConnectionButton()
{

}

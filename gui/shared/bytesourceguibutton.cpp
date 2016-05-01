/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "bytesourceguibutton.h"
#include "../floatingdialog.h"
#include "../sources/bytesourceabstract.h"
#include "../guihelper.h"
#include <QDebug>
#include <QIcon>
#include <QAction>

ByteSourceGuiButton::ByteSourceGuiButton(ByteSourceAbstract *bytesource, GuiHelper *nguiHelper, QWidget *parent) :
    QPushButton(parent)
{
    byteSource = bytesource;
    guiHelper = nguiHelper;

    guidia = nullptr;
    localAction = nullptr;


    localAction = new(std::nothrow) QAction(tr(""), this);
    if (localAction == nullptr) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    localAction->setCheckable(true);
    setCheckable(true);
    setToolTip(tr("Configuration panel"));
    setIcon(QIcon(":/Images/icons/configure-5.png"));
    setMaximumWidth(25);
    //setDefaultAction(localAction);

    setFlat(true);
    gui = byteSource->getGui();
    if (gui != nullptr) {
        connect(localAction, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
        connect(this, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
    } else {
        localAction->setToolTip(tr("No settings available for this source"));
        setDisabled(true);
        setVisible(false);
    }
}

ByteSourceGuiButton::~ByteSourceGuiButton()
{
    delete guidia;
}

void ByteSourceGuiButton::refreshState()
{
    gui = byteSource->getGui();

    if (gui != nullptr) {
        setDisabled(false);
        setVisible(true);
        connect(localAction, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
        connect(this, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
    } else {
        qDebug() << tr("No gui available for this source %1").arg(byteSource->metaObject()->className());
        setDisabled(true);
        setVisible(false);
    }

}

void ByteSourceGuiButton::onGuiDelete()
{
    gui = nullptr;
    localAction->setChecked(false);
}

void ByteSourceGuiButton::onToggle(bool enabled)
{
    if (enabled) {
        if (guidia == nullptr) {
            if (gui != nullptr) {
                guidia = new(std::nothrow) FloatingDialog(guiHelper, gui);
                if (guidia == nullptr) {
                    qFatal("Cannot allocate memory for FloatingDialog X{");
                }
                gui->setParent(guidia);

                guidia->attachAction(localAction);
                guidia->setWindowTitle(byteSource->description());
                guidia->setAllowReject(true);
                guidia->resize(gui->sizeHint());
                guidia->raise();
                guidia->show();
                connect(gui, SIGNAL(destroyed()), SLOT(onGuiDelete()));
                connect(guidia, SIGNAL(hiding()), SLOT(onGuiHiding()));
            }
        } else {
            guidia->raise();
            guidia->show();
        }
    } else if (guidia != nullptr) {
        guidia->hide();
    }
}

void ByteSourceGuiButton::onGuiHiding()
{
    setChecked(false);
    localAction->setChecked(false);
}

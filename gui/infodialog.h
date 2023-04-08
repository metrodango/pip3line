/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include "appdialog.h"

namespace Ui {
class InfoDialog;
}

class TransformAbstract;

class InfoDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit InfoDialog(GuiHelper * guiHelper, TransformAbstract *transform, QWidget *parent = nullptr);
        ~InfoDialog();
        TransformAbstract * getTransform();
        
    private:
        Q_DISABLE_COPY(InfoDialog)
        Ui::InfoDialog *ui;
        TransformAbstract * transform;
};

#endif // INFODIALOG_H

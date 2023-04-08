/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "appdialog.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit AboutDialog(GuiHelper * guiHelper, QWidget *parent = nullptr);
        ~AboutDialog();
        
    private:
        Q_DISABLE_COPY(AboutDialog)
        Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H

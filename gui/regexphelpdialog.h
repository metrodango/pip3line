/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef REGEXPHELPDIALOG_H
#define REGEXPHELPDIALOG_H

#include "appdialog.h"

namespace Ui {
class RegExpHelpDialog;
}

class RegExpHelpDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit RegExpHelpDialog(GuiHelper *nguiHelper, QWidget *parent = nullptr);
        ~RegExpHelpDialog();
        
    private:
        Q_DISABLE_COPY(RegExpHelpDialog)
        Ui::RegExpHelpDialog *ui;
};

#endif // REGEXPHELPDIALOG_H

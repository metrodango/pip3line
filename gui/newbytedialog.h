/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NEWBYTEDIALOG_H
#define NEWBYTEDIALOG_H

#include "appdialog.h"

namespace Ui {
class NewByteDialog;
}

class NewByteDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit NewByteDialog(GuiHelper * guiHelper, QWidget *parent = 0, bool isSimple = false);
        ~NewByteDialog();
        int byteCount();
        char getChar();
        void closeEvent(QCloseEvent *event);
        
    private:
        Q_DISABLE_COPY(NewByteDialog)
        Ui::NewByteDialog *ui;
};

#endif // NEWBYTEDIALOG_H

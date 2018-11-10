/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TEXTINPUTDIALOG_H
#define TEXTINPUTDIALOG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class textInputDialog;
}

class TextInputDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit TextInputDialog(QWidget *parent = nullptr);
        ~TextInputDialog();
        void setTextLabel(QString text);
        void setPixLabel(QPixmap pix);
        QString getInputText() const;
        void setText(QString text);
        
    private:
        Q_DISABLE_COPY(TextInputDialog)
        Ui::textInputDialog *ui;
};

#endif // TEXTINPUTDIALOG_H

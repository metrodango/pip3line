/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef CUTWIDGET_H
#define CUTWIDGET_H

#include <QWidget>
#include "../cut.h"

namespace Ui {
class CutWidget;
}

class CutWidget : public QWidget
{
    Q_OBJECT
    
    public:
        explicit CutWidget(Cut *ntransform, QWidget *parent = nullptr);
        ~CutWidget();
    private slots:
        void onFromChange(int value);
        void onLengthChange(int value);
        void onEverythingChange(bool);
        void onCutToggled(bool checked);
        void onLineByLineToggled(bool checked);
    private:
        Ui::CutWidget *ui;
        Cut *transform;
};

#endif // CUTWIDGET_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ROTXWIDGET_H
#define ROTXWIDGET_H

#include <QWidget>
#include "../rotx.h"

namespace Ui {
class RotXWidget;
}

class RotXWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit RotXWidget(Rotx *ntransform, QWidget *parent = nullptr);
        ~RotXWidget();
    public slots:
        void onVariantChanged(int val);
    private:
        Ui::RotXWidget *ui;
        Rotx *transform;
};

#endif // ROTXWIDGET_H

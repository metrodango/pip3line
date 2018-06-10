/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASEXWIDGET_H
#define BASEXWIDGET_H

#include <QWidget>
#include "../basex.h"

namespace Ui {
class BaseXWidget;
}

class BaseXWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit BaseXWidget(BaseX *transform, QWidget *parent = 0);
        ~BaseXWidget();
    public slots:
        void onBaseChanged(int val);
    private:
        Ui::BaseXWidget *ui;
        BaseX *transform;
};

#endif // BASEXWIDGET_H

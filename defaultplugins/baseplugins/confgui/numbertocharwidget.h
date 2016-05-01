/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NUMBERTOCHARWIDGET_H
#define NUMBERTOCHARWIDGET_H

#include <QWidget>
#include "../numbertochar.h"

namespace Ui {
class NumberToCharWidget;
}

class NumberToCharWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit NumberToCharWidget(NumberToChar * transform,QWidget *parent = 0);
        ~NumberToCharWidget();

    private slots:
        void onSeparatorChanged(QString val);
        
    private:
        Ui::NumberToCharWidget *ui;
        NumberToChar * transform;
};

#endif // NUMBERTOCHARWIDGET_H

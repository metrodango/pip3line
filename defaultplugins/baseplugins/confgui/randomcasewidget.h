/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef RANDOMCASEWIDGET_H
#define RANDOMCASEWIDGET_H

#include <QWidget>
#include "../randomcase.h"

namespace Ui {
class RandomCaseWidget;
}

class RandomCaseWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit RandomCaseWidget(RandomCase *ntransform, QWidget *parent = 0);
        ~RandomCaseWidget();
    private:
        Ui::RandomCaseWidget *ui;
        RandomCase *transform;
};

#endif // RANDOMCASEWIDGET_H

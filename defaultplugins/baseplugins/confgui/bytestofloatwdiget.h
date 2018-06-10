/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESTOFLOATWDIGET_H
#define BYTESTOFLOATWDIGET_H

#include <QWidget>
#include "../bytestofloat.h"

namespace Ui {
class BytesToFloatWdiget;
}

class BytesToFloatWdiget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit BytesToFloatWdiget(BytesToFloat *transform,QWidget *parent = 0);
        ~BytesToFloatWdiget();
    public slots:
        void onPrecisionChanged(int val);
    private:
        Ui::BytesToFloatWdiget *ui;
        BytesToFloat *transform;
};

#endif // BYTESTOFLOATWDIGET_H

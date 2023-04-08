/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef BINARYWIDGET_H
#define BINARYWIDGET_H

#include <QWidget>
#include "../binary.h"

namespace Ui {
class BinaryWidget;
}

class BinaryWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit BinaryWidget(Binary *ntransform, QWidget *parent = nullptr);
        ~BinaryWidget();
    private slots:
        void onChangeGroupBy(int value);
    private:
        Ui::BinaryWidget *ui;
        Binary *transform;
};

#endif // BINARYWIDGET_H

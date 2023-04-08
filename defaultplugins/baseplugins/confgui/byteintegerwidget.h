/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef BYTEINTEGERWIDGET_H
#define BYTEINTEGERWIDGET_H

#include <QWidget>
#include "../bytesinteger.h"

namespace Ui {
class ByteIntegerWidget;
}

class ByteIntegerWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ByteIntegerWidget(BytesInteger * transform,QWidget *parent = nullptr);
        ~ByteIntegerWidget();

    private slots:
        void sizeChanged();
        
    private:
        Ui::ByteIntegerWidget *ui;
        BytesInteger * transform;
};

#endif // BYTEINTEGERWIDGET_H

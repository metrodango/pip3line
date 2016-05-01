/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
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
        explicit ByteIntegerWidget(BytesInteger * transform,QWidget *parent = 0);
        ~ByteIntegerWidget();

    private slots:
        void signedChanged(bool val);
        void bytesOrderChanged(bool val);
        void sizeChanged();
        
    private:
        Ui::ByteIntegerWidget *ui;
        BytesInteger * transform;
};

#endif // BYTEINTEGERWIDGET_H

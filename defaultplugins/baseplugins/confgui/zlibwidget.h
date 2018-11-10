/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ZLIBWIDGET_H
#define ZLIBWIDGET_H

#include <QWidget>
#include "../zlib.h"

namespace Ui {
class ZlibWidget;
}

class ZlibWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ZlibWidget(Zlib * transform, QWidget *parent = nullptr);
        ~ZlibWidget();
        
    private slots:
        void onCompressionChanged(int value);

    private:
        Ui::ZlibWidget *ui;
        Zlib * transform;
};

#endif // ZLIBWIDGET_H

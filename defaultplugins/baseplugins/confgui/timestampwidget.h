/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TIMESTAMPWIDGET_H
#define TIMESTAMPWIDGET_H

#include <QWidget>
#include "../timestamp.h"

namespace Ui {
class TimestampWidget;
}

class TimestampWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit TimestampWidget(TimeStamp *transform, QWidget *parent = nullptr);
        ~TimestampWidget();
        
    private:
        Ui::TimestampWidget *ui;
        TimeStamp *transform;
};

#endif // TIMESTAMPWIDGET_H

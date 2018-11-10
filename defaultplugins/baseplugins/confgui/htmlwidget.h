/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HTMLWIDGET_H
#define HTMLWIDGET_H

#include <QWidget>
#include "../html.h"

namespace Ui {
class HtmlWidget;
}

class HtmlWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit HtmlWidget(Html *ntransform, QWidget *parent = nullptr);
        ~HtmlWidget();
    private:
        Ui::HtmlWidget *ui;
        Html *transform;
};

#endif // HTMLWIDGET_H

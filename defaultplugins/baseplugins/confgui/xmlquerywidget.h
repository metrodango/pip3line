/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef XMLQUERYWIDGET_H
#define XMLQUERYWIDGET_H

#include <QWidget>
#include "../xmlquery.h"

namespace Ui {
class XmlQueryWidget;
}

class XmlQueryWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit XmlQueryWidget(XmlQuery *transform, QWidget *parent = nullptr);
        ~XmlQueryWidget();
    public slots:
        void onQuerySubmit();
        
    private:
        Ui::XmlQueryWidget *ui;
        XmlQuery *transform;
};

#endif // XMLQUERYWIDGET_H

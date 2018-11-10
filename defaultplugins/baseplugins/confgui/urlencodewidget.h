/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/
#ifndef URLENCODEWIDGET_H
#define URLENCODEWIDGET_H

#include <QWidget>
#include "../urlencode.h"

namespace Ui {
class UrlEncodeWidget;
}

class UrlEncodeWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit UrlEncodeWidget(UrlEncode *ntransform, QWidget *parent = nullptr);
        ~UrlEncodeWidget();
    private slots:
        void onIncludeAll();
        void onExcludeChange(QString vals);
        void onIncludeChange(QString vals);
        void onPercentChange(QString character);
    private:
        Ui::UrlEncodeWidget *ui;
        UrlEncode *transform;
};

#endif // URLENCODEWIDGET_H

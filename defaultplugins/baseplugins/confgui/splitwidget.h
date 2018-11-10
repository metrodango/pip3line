/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SPLITWIDGET_H
#define SPLITWIDGET_H

#include <QWidget>
#include "../split.h"

namespace Ui {
class SplitWidget;
}

class SplitWidget : public QWidget
{
    Q_OBJECT
    
    public:
        explicit SplitWidget(Split *ntransform, QWidget *parent = nullptr);
        ~SplitWidget();
    private slots:
        void onSeparatorChange(char);
        void onGroupChange(int);
        void onAllGroupChange(bool);
        void onTrimChange(bool val);
        void onProcLineChanged(bool val);
    private:
        Ui::SplitWidget *ui;
        Split *transform;
};

#endif // SPLITWIDGET_H

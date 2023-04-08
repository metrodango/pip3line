/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef REGULAREXPWIDGET_H
#define REGULAREXPWIDGET_H

#include <QWidget>
#include "../regularexp.h"

namespace Ui {
class RegularExpWidget;
}

class RegularExpWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit RegularExpWidget(RegularExp *ntransform, QWidget *parent = nullptr);
        ~RegularExpWidget();
    private slots:
        void onActionTypeChange(int val);
        void onGreedyChange(bool val);
        void onRegularExpressionChange(QString val);
        void onCaseInsensitiveChange(bool val);
        void onGroupChange(int val);
        void onAllGroupsChange(bool val);
        void onReplacementStringChange(QString val);
        void onLinByLineChange(bool val);
    private:
        Ui::RegularExpWidget *ui;
        RegularExp *transform;
        bool eventFilter(QObject *o, QEvent *e);
};

#endif // REGULAREXPWIDGET_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HEXENCODEWIDGET_H
#define HEXENCODEWIDGET_H

#include <QWidget>
#include "../hexencode.h"

namespace Ui {
class HexEncodeWidget;
}

class HexEncodeWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit HexEncodeWidget(HexEncode *ntransform, QWidget *parent = nullptr);
        ~HexEncodeWidget();
    private slots:
        void onTypeChange();
        void onAddPrefixChanged(bool val);
    private:
        Ui::HexEncodeWidget *ui;
        HexEncode *transform;
};

#endif // HEXENCODEWIDGET_H

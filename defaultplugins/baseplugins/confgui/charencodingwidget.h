/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CHARENCODINGWIDGET_H
#define CHARENCODINGWIDGET_H

#include <QWidget>
#include "../charencoding.h"

namespace Ui {
class CharEncodingWidget;
}

class CharEncodingWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit CharEncodingWidget(CharEncoding *ntransform, QWidget *parent = 0);
        ~CharEncodingWidget();
    private slots:
            void on_UTF16PushButton_clicked();
            void on_latin1PushButton_clicked();
            void onUTF8Clicked();
            void onCodecChanged();
    private:
        Ui::CharEncodingWidget *ui;
        CharEncoding *transform;
};

#endif // CHARENCODINGWIDGET_H

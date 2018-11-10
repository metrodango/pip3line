/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTEROTWIDGET_H
#define BYTEROTWIDGET_H

#include <QWidget>

namespace Ui {
class ByteRotWidget;
}

class ByteRot;

class ByteRotWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ByteRotWidget(ByteRot * transf, QWidget *parent = nullptr);
        ~ByteRotWidget();
    public slots:
        void onRotationChanged(int val);
    private:
        Ui::ByteRotWidget *ui;
        ByteRot *transform;
};

#endif // BYTEROTWIDGET_H

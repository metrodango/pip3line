/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef HMACTRANSFORMWIDGET_H
#define HMACTRANSFORMWIDGET_H

#include <QWidget>

namespace Ui {
class HMACTransformWidget;
}

class HMACTransform;

class HMACTransformWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit HMACTransformWidget(HMACTransform *transform, QWidget *parent = nullptr);
        ~HMACTransformWidget();
    public slots:
        void onHashChanged(int val);
    private slots:
        void onKeyLineReturn();

    private:
        Ui::HMACTransformWidget *ui;
        HMACTransform *transform;
};

#endif // HMACTRANSFORMWIDGET_H

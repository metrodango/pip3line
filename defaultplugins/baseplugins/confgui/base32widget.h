/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASE32WIDGET_H
#define BASE32WIDGET_H

#include <QWidget>
#include "../base32.h"

namespace Ui {
class Base32Widget;
}

class Base32Widget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit Base32Widget(Base32 *transform, QWidget *parent = 0);
        ~Base32Widget();

    private slots:
        void onVariantchange(int newVariant);
        void onPaddingChange(QString val);
        void onIncludePaddingToggled(bool val);
    private:
        Ui::Base32Widget *ui;
        Base32 *transform;
};

#endif // BASE32WIDGET_H

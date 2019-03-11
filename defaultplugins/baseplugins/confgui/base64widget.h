/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASE64WIDGET_H
#define BASE64WIDGET_H

#include <QWidget>
#include "../base64.h"

namespace Ui {
class Base64Widget;
}

class Base64Widget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit Base64Widget(Base64 *transform, QWidget *parent = nullptr);
        ~Base64Widget();

    private slots:
        void onChangeVariant(int variant);
        void verifyChar62(QString nchar);
        void verifyChar63(QString nchar);
        void verifyCharPadding(QString nchar);
        void onCustomConf();
        void onInvalidSepToggled(bool value);
    private:
        Ui::Base64Widget *ui;
        Base64 *transform;
};

#endif // BASE64WIDGET_H

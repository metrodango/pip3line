/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include "libtransform_global.h"
#include <QValidator>
#include <QWidget>

namespace Ui {
class HexWidget;
}

class CharValidator : public QValidator
{
        Q_OBJECT
    public:
        explicit CharValidator(QObject *parent = nullptr);
        QValidator::State validate(QString & input, int & pos) const;
};

class LIBTRANSFORMSHARED_EXPORT HexWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit HexWidget(QWidget *parent = nullptr);
        ~HexWidget();
        char getChar();
        void setChar(char c);
    signals:
        void charChanged(char c);
    private slots:
        void onHexChanged();
        void onCharChanged(QString val);
    private:
        Q_DISABLE_COPY(HexWidget)
        Ui::HexWidget *ui;
};

#endif // HEXWIDGET_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DISTORMWIDGET_H
#define DISTORMWIDGET_H

#include <QWidget>
#include <QStringList>
#include "distormtransf.h"

namespace Ui {
class DistormWidget;
}

class DistormWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit DistormWidget(DistormTransf * transform, QWidget *parent = nullptr);
        ~DistormWidget();
    private slots:
        void onMaxInstruc(int val);
        void onOffset(QString val);
        void onType(int val);
        void onShowOffset(bool val);
        void onShowOpcodes(bool val);
    private:
        Q_DISABLE_COPY(DistormWidget)
        static const QStringList asmTypeStrings;
        Ui::DistormWidget *ui;
        DistormTransf * transform;
};

#endif // DISTORMWIDGET_H

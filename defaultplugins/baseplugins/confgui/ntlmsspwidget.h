/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef NTLMSSPWIDGET_H
#define NTLMSSPWIDGET_H

#include <QWidget>
#include "../ntlmssp.h"

namespace Ui {
class NtlmsspWidget;
}

class NtlmsspWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit NtlmsspWidget(Ntlmssp * transform, QWidget *parent = nullptr);
        ~NtlmsspWidget();
    private:
        Ui::NtlmsspWidget *ui;
        Ntlmssp * transform;
};

#endif // NTLMSSPWIDGET_H

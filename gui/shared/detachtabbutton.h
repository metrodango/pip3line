/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DETACHTABBUTTON_H
#define DETACHTABBUTTON_H

#include <QPushButton>
class TabAbstract;

class DetachTabButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit DetachTabButton(TabAbstract *tab);
    private:
        Q_DISABLE_COPY(DetachTabButton)
        TabAbstract * curtab;
};

#endif // DETACHTABBUTTON_H

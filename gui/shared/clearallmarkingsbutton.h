/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CLEARALLMARKINGSBUTTON_H
#define CLEARALLMARKINGSBUTTON_H

#include <QPushButton>
class ByteSourceAbstract;

class ClearAllMarkingsButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit ClearAllMarkingsButton(ByteSourceAbstract *bytesource, QWidget *parent = nullptr);

    private:
        Q_DISABLE_COPY(ClearAllMarkingsButton)
        ByteSourceAbstract *byteSource;

};

#endif // CLEARALLMARKINGSBUTTON_H

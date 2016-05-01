/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESOURCEGUIBUTTON_H
#define BYTESOURCEGUIBUTTON_H

#include <QToolButton>
#include <QPushButton>

class ByteSourceAbstract;
class GuiHelper;
class FloatingDialog;
class QAction;

class ByteSourceGuiButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit ByteSourceGuiButton(ByteSourceAbstract *bytesource,GuiHelper *guiHelper, QWidget *parent = 0);
        ~ByteSourceGuiButton();

    public slots:
        void refreshState();

    private slots:
        void onGuiDelete();
        void onToggle(bool val);
        void onGuiHiding();
    private:
        ByteSourceAbstract *byteSource;
        GuiHelper *guiHelper;
        QWidget * gui;
        FloatingDialog *guidia;
        QAction *localAction;

};

#endif // BYTESOURCEGUIBUTTON_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef UNIVERSALRECEIVERBUTTON_H
#define UNIVERSALRECEIVERBUTTON_H

#include <QPushButton>
#include <QObject>

class TabAbstract;
class GuiHelper;

class CommonUniversalReceiverListener : public QObject
{
        Q_OBJECT
    public:
        explicit CommonUniversalReceiverListener();
    public slots:
        void onReceiverConfigured();
    signals:
        void taken();
    private:
        Q_DISABLE_COPY(CommonUniversalReceiverListener)
};

class UniversalReceiverButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit UniversalReceiverButton(TabAbstract *tab, GuiHelper *guiHelper);
        ~UniversalReceiverButton();
    public slots:
        void reset();
    private slots:
        void onClicked(bool checked);
    signals:
        void tookReceiver();
    private:
        GuiHelper * guiHelper;
        TabAbstract *attachedTab;
        static  CommonUniversalReceiverListener staticListener;
        bool taken;

};

#endif // UNIVERSALRECEIVERBUTTON_H

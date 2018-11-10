#ifndef NEWCONNECTIONBUTTON_H
#define NEWCONNECTIONBUTTON_H

#include <QObject>
#include <QPushButton>

class NewConnectionButton : public QPushButton
{
    public:
        NewConnectionButton(QWidget *parent);
        ~NewConnectionButton();
    private:
        Q_DISABLE_COPY(NewConnectionButton)
};

#endif // NEWCONNECTIONBUTTON_H

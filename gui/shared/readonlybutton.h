/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef READONLYBUTTON_H
#define READONLYBUTTON_H

#include <QPushButton>

class ByteSourceAbstract;

class ReadOnlyButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit ReadOnlyButton(ByteSourceAbstract * bytesource, QWidget *parent = nullptr);
    signals:
        void logError(QString message);
    public slots:
        void refreshStateValue();
        void onToggle(bool val);

    private:
        Q_DISABLE_COPY(ReadOnlyButton)
        ByteSourceAbstract *byteSource;
        static const QString ReadWrite;
        static const QString ReadOnly;
        static const QString ButtonToolTipRW;
        static const QString ButtonToolTipRO;
        static const QString CAP_WRITE_DISABLED_ToolTip;
};

#endif // READONLYBUTTON_H

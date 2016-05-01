/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DELETEABLELISTITEM_H
#define DELETEABLELISTITEM_H

#include "libtransform_global.h"
#include <QWidget>
#include <QPixmap>

namespace Ui {
class DeleteableListItem;
}

class LIBTRANSFORMSHARED_EXPORT DeleteableListItem : public QWidget
{
        Q_OBJECT
        
    public:
        explicit DeleteableListItem(const QString &text, const QPixmap &im = QPixmap(), QWidget *parent = 0);
        ~DeleteableListItem();
        QString getName();
        void setEnableDelete(bool val);
    signals:
        void itemDeleted(QString name);
    private slots:
        void onDelete();
    private:
        Q_DISABLE_COPY(DeleteableListItem)
        Ui::DeleteableListItem *ui;
};

#endif // DELETEABLELISTITEM_H

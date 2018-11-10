/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QWidget>
#include <QSize>
#include <QModelIndex>
#include <QBitArray>

namespace Ui {
    class MemoryWidget;
}

class CurrentMemorysource;
class QMenu;
class QAction;

class MemoryWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit MemoryWidget(CurrentMemorysource *source, QWidget *parent = nullptr);
        ~MemoryWidget();
        void setProcSelection(bool val);
        QSize sizeHint() const;
    private slots:
        void onDoubleClick(QModelIndex index);
        void onRefreshToggled(bool val);
        void onRightClick(QPoint pos);
        void contextMenuAction(QAction * action);
        void onSearch(QByteArray item, QBitArray mask, bool maybetext);
    private:
        Q_DISABLE_COPY(MemoryWidget)
        static const QString GOTOSTART;
        static const QString GOTOEND;
        void initContextMenu();
        Ui::MemoryWidget *ui;
        CurrentMemorysource * msource;
        QMenu * contextMenu;
};

#endif // MEMORYWIDGET_H

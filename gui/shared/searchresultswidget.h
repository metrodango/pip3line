#ifndef SEARCHRESULTSWIDGET_H
#define SEARCHRESULTSWIDGET_H

#include <QModelIndex>
#include <QWidget>
#include <QPoint>

namespace Ui {
class SearchResultsWidget;
}

class FoundOffsetsModel;
class QAction;
class QMenu;
class GuiHelper;

class SearchResultsWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SearchResultsWidget(FoundOffsetsModel *offsetModel, GuiHelper *nguiHelper,  QWidget *parent = 0);
        ~SearchResultsWidget();
    public slots:
        void clearResults();
    signals:
        void jumpTo(quint64 start, quint64 end);
    private slots:
        void onSelectionChanged(QModelIndex index);
        void onModelUpdated();
        void onRightClick(QPoint pos);
        void contextMenuAction(QAction * action);
        void onClear();
        void savetoFile(QString format, int base);
        void onSaveToFileHexAction();
        void onSaveToFileDecAction();
        void setCurrentSelection(int pos);
    private:
        bool eventFilter(QObject *obj, QEvent *ev);
        Ui::SearchResultsWidget *ui;
        FoundOffsetsModel * itemModel;
        QMenu * globalContextMenu;
        QAction * copyAsHexadecimal;
        QAction * copyAsDecimal;
        QAction * saveListToFileAsHexadecimal;
        QAction * saveListToFileAsDecimal;
        QAction * copyListAsHexadecimal;
        QAction * copyListAsDecimal;
        QMenu * saveToFileMenu;
        QMenu * saveToClipboardMenu;
        GuiHelper *guiHelper;

};

#endif // SEARCHRESULTSWIDGET_H

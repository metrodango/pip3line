/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QBitArray>
#include <QHash>
#include <QJsonObject>
#include <QTimer>
#include "singleviewabstract.h"
#include "../sources/blocksources/blockssource.h"

namespace Ui {
    class HexView;
}

class QAction;
class GuiHelper;
class LoggerWidget;
class ByteSourceAbstract;
class SourcesOrchestatorAbstract;
class ByteItemModel;
class ByteTableView;
class QMenu;
class SendToMenu;

class HexView : public SingleViewAbstract
{
        Q_OBJECT
    public:
        explicit HexView(ByteSourceAbstract *dataModel,GuiHelper *guiHelper, QWidget *parent = nullptr, bool takeByteSourceOwnership = false);
        ~HexView();
        void addCustomMenuActions(QAction * action);
        ByteTableView *getHexTableView();
        quint64 getLowPos();
        QJsonObject createFuzzingTemplate(ByteSourceAbstract *bs);
    public slots:
        void onClearAllMArkings();
        bool goTo(quint64 offset, bool absolute,bool negative, bool select = false);
        void gotoSearch(quint64 soffset, quint64 eoffset);
        void search(QByteArray item, QBitArray mask);
        void optionGuiRequested();
    private slots:
        void updateStats();
        void onRightClick(QPoint pos);
        void updateMarkMenu();
        void updateImportExportMenus();
        void onCopy(QAction * action);
        void onLoad(QAction * action);
        void onReplace(QAction * action);
        void onInsertAfter(QAction * action);
        void onInsertBefore(QAction * action);
        void onSendToTriggered(QAction * action);
        void onMarkMenu(QAction * action);
        void onSelectFromSizeMenu(QAction * action);
        void onGotoFromOffsetMenu(QAction * action);
        void onCopySelectionSize(QAction * action);
        void onCopyCurrentOffset(QAction *action);
        void onClearSelectionMarkings();
        void onLoadFile();
        void onExportForFuzzing();
        void onSaveToFile(QAction *action);
        void onSelectAll();
        void onKeepOnlySelection();
        void onNewByteArray();
        void onDeleteSelection();
        void onSelectionChanged();

    private:
        Q_DISABLE_COPY(HexView)
        void buildContextMenus();
        quint64 normalizeSelectedInt(bool bigEndian);
        Ui::HexView *ui;
        QMenu * globalContextMenu;
        QMenu * markMenu;
        QMenu * selectFromSizeMenu;
        QMenu * gotoFromOffsetMenu;
        QMenu * copyMenu;
        QMenu * copySelectionSizeMenu;
        QMenu * copyCurrentOffsetMenu;
        QMenu * loadMenu;
        QMenu * replaceMenu;
        QMenu * insertAfterMenu;
        QMenu * insertBeforeMenu;
        QMenu * saveToFileMenu;
        SendToMenu * sendToMenu;
        QAction * newMarkingAction;
        QAction * fuzzingExportAction;
        ByteItemModel * dataModel;
        ByteTableView * hexTableView;
        qint64 selectedSize;
        quint64 startOffset;
        QTimer updateTimer;
};

#endif // HEXVIEW_H

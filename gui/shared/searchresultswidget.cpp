#include "searchresultswidget.h"
#include "searchwidget.h"
#include <QAbstractListModel>
#include <QAction>
#include <QMenu>
#include <QItemSelectionModel>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QEvent>
#include "ui_searchresultswidget.h"
#include "../sources/searchabstract.h"
#include "../sources/bytesourceabstract.h"
#include "../guihelper.h"
#include "guiconst.h"

using namespace GuiConst;

SearchResultsWidget::SearchResultsWidget(FoundOffsetsModel * offsetModel, GuiHelper *nguiHelper, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchResultsWidget)
{
    guiHelper = nguiHelper;
    saveToFileMenu = nullptr;
    saveToClipboardMenu = nullptr;
    ui->setupUi(this);
    itemModel = offsetModel;
    if (itemModel->rowCount() > 0) {
        ui->clearResultsPushButton->setEnabled(true);
        ui->savePushButton->setEnabled(true);
    } else {
        ui->clearResultsPushButton->setEnabled(false);
        ui->savePushButton->setEnabled(false);
    }
    QAbstractItemModel * oldModel = ui->listView->model();
    ui->listView->setModel(itemModel);
    ui->listView->setUniformItemSizes(true);
    ui->listView->setLayoutMode(QListView::Batched);
    ui->listView->setFont(guiHelper->getRegularFont());
    ui->listView->installEventFilter(this);
    delete oldModel;

    QAction * action = nullptr;

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);

    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == nullptr) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
    }

    copyAsHexadecimal = new(std::nothrow) QAction(HEXADECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == nullptr) {
        qFatal("Cannot allocate memory for HEXADECIMAL X{");
    }

    copyAsDecimal = new(std::nothrow) QAction(DECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == nullptr) {
        qFatal("Cannot allocate memory for DECIMAL X{");
    }

    saveListToFileAsHexadecimal = new(std::nothrow) QAction(HEXADECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == nullptr) {
        qFatal("Cannot allocate memory for HEXADECIMAL X{");
    }

    saveListToFileAsDecimal = new(std::nothrow) QAction(DECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == nullptr) {
        qFatal("Cannot allocate memory for DECIMAL X{");
    }

    copyListAsHexadecimal = new(std::nothrow) QAction(HEXADECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == nullptr) {
        qFatal("Cannot allocate memory for HEXADECIMAL X{");
    }

    copyListAsDecimal = new(std::nothrow) QAction(DECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == nullptr) {
        qFatal("Cannot allocate memory for DECIMAL X{");
    }

    action = new(std::nothrow) QAction(COPYOFFSET, globalContextMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for COPYOFFSET X{");
    }
    action->setDisabled(true);
    globalContextMenu->addAction(action);
    globalContextMenu->addAction(copyAsHexadecimal);
    globalContextMenu->addAction(copyAsDecimal);

    onModelUpdated();

    saveToFileMenu = new(std::nothrow) QMenu(tr("Save to file"),ui->savePushButton);
    if (saveToFileMenu == nullptr) {
        qFatal("Cannot allocate memory for saveToFileMenu X{");
    }
    saveToFileMenu->addAction(saveListToFileAsHexadecimal);
    saveToFileMenu->addAction(saveListToFileAsDecimal);
    ui->savePushButton->setMenu(saveToFileMenu);

    saveToClipboardMenu = new(std::nothrow) QMenu(tr("Save to clipboard"),ui->savePushButton);
    if (saveToClipboardMenu == nullptr) {
        qFatal("Cannot allocate memory for saveToClipboardMenu X{");
    }

    saveToClipboardMenu->addAction(copyListAsHexadecimal);
    saveToClipboardMenu->addAction(copyListAsDecimal);
    ui->copyPushButton->setMenu(saveToClipboardMenu);

    connect(ui->listView, &QListView::customContextMenuRequested, this, &SearchResultsWidget::onRightClick);
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &SearchResultsWidget::onSelectionChanged);
    connect(globalContextMenu, &QMenu::triggered, this, &SearchResultsWidget::contextMenuAction);
    connect(itemModel, &FoundOffsetsModel::updated, this, &SearchResultsWidget::onModelUpdated);
    connect(ui->clearResultsPushButton, &QPushButton::clicked, this, &SearchResultsWidget::onClear);
    connect(saveListToFileAsHexadecimal, &QAction::triggered, this, &SearchResultsWidget::onSaveToFileHexAction);
    connect(saveListToFileAsDecimal, &QAction::triggered, this, &SearchResultsWidget::onSaveToFileDecAction);
    connect(saveToClipboardMenu, &QMenu::triggered, this, &SearchResultsWidget::contextMenuAction);

    // if there is a last item selected

    setCurrentSelection(itemModel->getLastSelected());

    connect(itemModel, &FoundOffsetsModel::lastUpdated, this, &SearchResultsWidget::setCurrentSelection);
}

SearchResultsWidget::~SearchResultsWidget()
{
    delete ui;
    itemModel = nullptr;
}

void SearchResultsWidget::clearResults()
{
    itemModel->clear();
}

void SearchResultsWidget::onSelectionChanged(QModelIndex index)
{
    emit jumpTo(itemModel->getStartingOffset(index), itemModel->getEndOffset(index));
}

void SearchResultsWidget::onModelUpdated()
{
    if (itemModel->rowCount() > 0) {
        ui->resultsLabel->setText(tr("Results: %1 (%2 ms)").arg(itemModel->rowCount()).arg(itemModel->elapsed()));
        ui->clearResultsPushButton->setEnabled(true);
        ui->savePushButton->setEnabled(true);
        ui->copyPushButton->setEnabled(true);
    } else {
        ui->resultsLabel->setText(tr("No results"));
        ui->clearResultsPushButton->setEnabled(false);
        ui->savePushButton->setEnabled(false);
        ui->copyPushButton->setEnabled(false);
    }

    ui->resultsLabel->setVisible(true);
}

void SearchResultsWidget::onRightClick(QPoint pos)
{
    globalContextMenu->exec(this->mapToGlobal(pos));
}

void SearchResultsWidget::contextMenuAction(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text;
    QModelIndex index;
    if (action == copyAsHexadecimal) {
        index = ui->listView->selectionModel()->currentIndex();
        clipboard->setText(index.data().toString());
    } else if (action == copyAsDecimal) {
        index = ui->listView->selectionModel()->currentIndex();
        clipboard->setText(QString::number(itemModel->getStartingOffset(index)));
    } else if (action == copyListAsHexadecimal) {
        BytesRangeList * list = itemModel->getRanges();
        for (int i = 0; i < list->size(); i++) {
            text.append(tr("0x%1\n").arg(list->at(i)->getLowerVal(), 0, 16));
        }
        text.chop(1);
        clipboard->setText(text);
    } else if (action == copyListAsDecimal) {
        BytesRangeList * list = itemModel->getRanges();
        for (int i = 0; i < list->size(); i++) {
            text.append(tr("%1\n").arg(list->at(i)->getLowerVal()));
        }
        text.chop(1);
        clipboard->setText(text);
    } else {
        qCritical() << "[SearchResultsWidget::contextMenuAction] QAction object not found" << action;
    }
}

void SearchResultsWidget::onClear()
{
    itemModel->clear();
    ui->resultsLabel->setText(tr("Results: 0"));
}

void SearchResultsWidget::savetoFile(QString format, int base)
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose a file to save to"), GuiConst::GLOBAL_LAST_PATH);
    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
        BytesRangeList * list = itemModel->getRanges();
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QString mess = tr("Failed to open file ");
            QMessageBox::critical(this, mess , file.errorString());
            guiHelper->logError(mess.append(file.errorString()));
            return;
        }
        for (int i = 0; i < list->size(); i++) {
            qint64 written = 0;
            QByteArray data = format.arg(list->at(i)->getLowerVal(), 0, base).toUtf8();
            while ((written = file.write(data)) > 0) {
                if (written == data.length())
                    break;
                else
                    data = data.mid(static_cast<int>(written) - 1); // we assume that the write call will return something sensible
            }
            if (i < list->size() - 1)
                file.write("\n");
        }

        file.close();
    }
}

void SearchResultsWidget::onSaveToFileHexAction()
{
    savetoFile("0x%1",16);
}

void SearchResultsWidget::onSaveToFileDecAction()
{
    savetoFile("%1",10);
}

void SearchResultsWidget::setCurrentSelection(int pos)
{
    if (pos != FoundOffsetsModel::INVALID_POS && pos < itemModel->size()) {
        QModelIndex selectedIndex = itemModel->index(pos);
        ui->listView->setCurrentIndex(selectedIndex);
    }
}

bool SearchResultsWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->listView && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Return) {
            QModelIndex index = ui->listView->currentIndex();
            if (index.isValid()) {
                emit jumpTo(itemModel->getStartingOffset(index), itemModel->getEndOffset(index));
            } else {
                qCritical() << "[SearchResultsWidget::eventFilter] index is not valid";
            }
            return true;
        }
    }
    return false;
}

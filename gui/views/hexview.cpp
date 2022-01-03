/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QClipboard>
#include <QFileDialog>
#include <QColorDialog>
#include <QMenu>
#include <QMessageBox>
#include <QAction>
#include <QDebug>
#include <QTextEncoder>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "textinputdialog.h"
#include "newbytedialog.h"
#include "hexview.h"
#include "ui_hexview.h"
#include "tabs/tababstract.h"
#include "sources/bytesourceabstract.h"
#include "byteitemmodel.h"
#include "bytetableview.h"
#include "loggerwidget.h"
#include "guihelper.h"
#include "shared/sendtomenu.h"
#include <climits>
#include <transformabstract.h>
#include "shared/guiconst.h"
#include <tabs/packetanalyser/sourcesorchestatorabstract.h>
#include <pipelinecommon.h>
using namespace Pip3lineCommon;
using namespace GuiConst;

HexView::HexView(ByteSourceAbstract *nbyteSource, GuiHelper *nguiHelper, QWidget *parent, bool takeByteSourceOwnership) :
    SingleViewAbstract(nbyteSource, nguiHelper, parent, takeByteSourceOwnership)
{
    ui = new(std::nothrow) Ui::HexView;
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::HexView X{");
    }

    updateTimer.setInterval(150);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, &QTimer::timeout, this, &HexView::onSelectionChanged);

    globalContextMenu = nullptr;
    sendToMenu = nullptr;
    markMenu = nullptr;
    copyMenu = nullptr;
    loadMenu = nullptr;
    copySelectionSizeMenu = nullptr;
    insertAfterMenu = nullptr;
    insertBeforeMenu = nullptr;
    replaceMenu = nullptr;
    selectFromSizeMenu = nullptr;
    gotoFromOffsetMenu = nullptr;
    copyCurrentOffsetMenu = nullptr;
    selectedSize = 0;
    startOffset = 0;
    ui->setupUi(this);
    hexTableView = new(std::nothrow) ByteTableView(this);
    if (hexTableView == nullptr) {
        qFatal("Cannot allocate memory for ByteTableView X{");
    }
    connect(guiHelper, &GuiHelper::hexTableSizesUpdated, hexTableView, &ByteTableView::updateTableSizes);
    connect(hexTableView, &ByteTableView::error, logger, &LoggerWidget::logError);
    connect(hexTableView, &ByteTableView::warning, logger, &LoggerWidget::logWarning);


    dataModel = new(std::nothrow) ByteItemModel(byteSource,hexTableView);
    if (dataModel == nullptr) {
        delete hexTableView;
        hexTableView = nullptr;
        qFatal("Cannot allocate memory for ByteItemModel X{");
    }

    connect(byteSource, &ByteSourceAbstract::updated, this, &HexView::updateStats);

    connect(dataModel, &ByteItemModel::error, logger, [=](const QString &message) { logger->logError(message);});
    connect(dataModel, &ByteItemModel::warning, logger, [=](const QString &message) { logger->logWarning(message);});
    hexTableView->setModel(dataModel);

    ui->mainLayout->insertWidget(0,hexTableView);
    //connect(hexTableView, &ByteTableView::newSelection,&updateTimer, qOverload<>(&QTimer::start));
    connect(hexTableView, SIGNAL(newSelection()), &updateTimer, SLOT(start()));

    // creating context menus
    buildContextMenus();
    updateImportExportMenus();
    updateMarkMenu();
    connect(guiHelper, &GuiHelper::importExportUpdated, this, &HexView::updateImportExportMenus);
    connect(guiHelper, &GuiHelper::markingsUpdated, this, &HexView::updateMarkMenu);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &HexView::customContextMenuRequested, this, &HexView::onRightClick);

    updateStats();
}

HexView::~HexView()
{
    delete hexTableView;
    dataModel = nullptr; //no need to delete it, the TableView should take care of it
    // byteSource = nullptr; parent does that already
    // QActions should already be taken care of by the Qmenu they belong to
    delete fuzzingExportAction;
    delete sendToMenu;
    delete markMenu;
    delete copyMenu;
    delete loadMenu;
    delete copySelectionSizeMenu;
    delete insertAfterMenu;
    delete insertBeforeMenu;
    delete replaceMenu;
    delete selectFromSizeMenu;
    delete copyCurrentOffsetMenu;
    delete gotoFromOffsetMenu;
    delete saveToFileMenu;
    delete globalContextMenu;
    delete ui;
    logger = nullptr;
    guiHelper = nullptr;
   // qDebug() << "Destroyed" << this;
}

void HexView::addCustomMenuActions(QAction *action)
{
    QAction * prev = globalContextMenu->actionAt(QPoint(0,0));
    globalContextMenu->insertAction(prev,action);
    globalContextMenu->insertSeparator(prev);
}

ByteTableView *HexView::getHexTableView()
{
    return hexTableView;
}

quint64 HexView::getLowPos()
{
    return static_cast<quint64>(hexTableView->getLowerSelected());
}

void HexView::updateStats()
{
    // updating various stats
    QString ret;
    quint64 size = byteSource->size();

    if (size != ULLONG_MAX) {
        ret.append("Size: ");
        // Updating Hex info
        ret.append(QString::number(size)).append("|x").append(QString::number(size,16)).append(" bytes");

        if (size >= 1000) {
            ret.append(QString(" (%1)").arg(GuiConst::convertSizetoBytesString(size)));
        }
    }


    int scount = static_cast<int>(hexTableView->getSelectedBytesCount());
    //hexTableView->verticalHeader()->adjustSize();
    if ( scount != 0) {
        ret.append(tr(" [ %1|x%2 selected ]").arg(scount).arg(QString::number(scount,16)));
    }

    ui->statsLabel->setText(ret);

    //Updating offset

    ret = "Offset: ";
    int offset =  hexTableView->getCurrentPos();
    if (offset < 0) {
        ret.append("NA");
    } else {
        quint64 finaloffset = static_cast<quint64>(offset) + byteSource->startingRealOffset();
        ret.append(QString::number(finaloffset));
        ret.append(" | x");
        ret.append(QString::number(finaloffset,16));
        ret.append(" | o");
        ret.append(QString::number(finaloffset,8));
    }
    ui->offsetLabel->setText(ret);
}

void HexView::onRightClick(QPoint pos)
{
    if (hexTableView->getSelectedBytes().isEmpty()) { // No selection
        copyMenu->setEnabled(false);
        ui->deleteSelectionAction->setEnabled(false);
        ui->keepOnlySelectionAction->setEnabled(false);
        sendToMenu->setEnabled(false);
        markMenu->setEnabled(false);
        ui->clearMarkingsAction->setEnabled(false);
        replaceMenu->setEnabled(false);
        selectFromSizeMenu->setEnabled(false);
        gotoFromOffsetMenu->setEnabled(false);
        copySelectionSizeMenu->setEnabled(false);
        ui->saveSelectedToFileAction->setEnabled(false);
    } else { // selected bytes
        copyMenu->setEnabled(true);
        ui->deleteSelectionAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
        ui->keepOnlySelectionAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
        sendToMenu->setEnabled(true);
        markMenu->setEnabled(true);
        replaceMenu->setEnabled(!byteSource->isReadonly());
        selectFromSizeMenu->setEnabled(true);
        gotoFromOffsetMenu->setEnabled(true);
        ui->clearMarkingsAction->setEnabled(byteSource->hasMarking());
        copySelectionSizeMenu->setEnabled(true);
        ui->saveSelectedToFileAction->setEnabled(true);
    }
    ui->newByteArrayAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESET) && !byteSource->isReadonly());
    loadMenu->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESET) && !byteSource->isReadonly());

    if (byteSource->size() <= 0) {
        ui->selectAllAction->setEnabled(false);
        ui->saveToFileAction->setEnabled(false);
        saveToFileMenu->setEnabled(false);
        copyCurrentOffsetMenu->setEnabled(false);
        insertAfterMenu->setEnabled(false);
        insertBeforeMenu->setEnabled(false);
    } else {
        ui->selectAllAction->setEnabled(true);
        ui->saveToFileAction->setEnabled(true);
        saveToFileMenu->setEnabled(true);
        copyCurrentOffsetMenu->setEnabled(true);
        insertAfterMenu->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
        insertBeforeMenu->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
    }
    ui->importFileAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_LOADFILE));

    globalContextMenu->exec(this->mapToGlobal(pos));
}

void HexView::updateMarkMenu()
{
    markMenu->clear(); // action created on the fly should be automatically deleted
    markMenu->addAction(ui->newMarkingAction);
    markMenu->addSeparator();
    QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
    QHashIterator<QString, QColor> i(colors);
    while (i.hasNext()) {
        i.next();
        QPixmap pix(48,48);
        pix.fill(i.value());
        QAction *  action = new(std::nothrow) QAction(QIcon(pix),i.key(), markMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateMarkMenu X{");
        }
        markMenu->addAction(action);
    }
}

void HexView::updateImportExportMenus()
{

    guiHelper->updateCopyContextMenu(copyMenu);
    guiHelper->updateLoadContextMenu(loadMenu);

    QStringList list = guiHelper->getImportExportFunctions();

    replaceMenu->clear();
    QAction * action = new(std::nothrow) QAction(NEW_BYTE_ACTION, replaceMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu new byte X{");
    }
    replaceMenu->addAction(action);
    replaceMenu->addSeparator();
    action = new(std::nothrow) QAction(tr("From clipboard as"), replaceMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu clipboard X{");
    }
    action->setDisabled(true);
    replaceMenu->addAction(action);
    action = new(std::nothrow) QAction(UTF8_STRING_ACTION, replaceMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu clipboard X{");
    }

    replaceMenu->addAction(action);
    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), replaceMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu user X{");
        }
        replaceMenu->addAction(action);
    }

    insertAfterMenu->clear();
    action = new(std::nothrow) QAction(NEW_BYTE_ACTION, insertAfterMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertAfterMenu new byte X{");
    }
    insertAfterMenu->addAction(action);
    insertAfterMenu->addSeparator();
    action = new(std::nothrow) QAction(UTF8_STRING_ACTION, insertAfterMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertAfterMenu UTF8 X{");
    }
    insertAfterMenu->addAction(action);

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), insertAfterMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateImportExportMenus insertAfterMenu user's X{");
        }
        insertAfterMenu->addAction(action);
    }

    insertBeforeMenu->clear();
    action = new(std::nothrow) QAction(NEW_BYTE_ACTION, insertBeforeMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertBeforeMenu new byte X{");
    }
    insertBeforeMenu->addAction(action);
    insertBeforeMenu->addSeparator();
    action = new(std::nothrow) QAction(UTF8_STRING_ACTION, insertBeforeMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertBeforeMenu UTF8 X{");
    }
    insertBeforeMenu->addAction(action);

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), insertBeforeMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateImportExportMenus insertBeforeMenu user's X{");
        }
        insertBeforeMenu->addAction(action);
    }
}

void HexView::onCopy(QAction *action)
{
    guiHelper->copyAction(action->text(), hexTableView->getSelectedBytes());
}

void HexView::onLoad(QAction *action)
{
    guiHelper->loadAction(action->text(), byteSource);
}

void HexView::onReplace(QAction *action)
{
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this,true);
        if (dialog == nullptr) {
            qFatal("Cannot allocate memory for onReplace NewByteDialog X{");
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            hexTableView->replaceSelectedBytes(dialog->getChar());
        }
        delete dialog;
    } else {
        QClipboard *clipboard = QApplication::clipboard();
        QString input = clipboard->text();

        if (action->text() == UTF8_STRING_ACTION) {
            hexTableView->replaceSelectedBytes(input.toUtf8());
        } else {
            TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
            if (ta != nullptr) {
                ta->setWay(TransformAbstract::OUTBOUND);
                hexTableView->replaceSelectedBytes(ta->transform(input.toUtf8()));
            }
        }
    }
}

void HexView::onInsertAfter(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    int pos = hexTableView->getHigherSelected() + 1;
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this);
        if (dialog == nullptr) {
            qFatal("Cannot allocate memory for onInsertAfter NewByteDialog X{");
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            dataModel->insert(pos, QByteArray(dialog->byteCount(),dialog->getChar()));
        }
        delete dialog;
    } else if (action->text() == UTF8_STRING_ACTION) {
        dataModel->insert(pos,input.toUtf8());
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != nullptr) {
            ta->setWay(TransformAbstract::OUTBOUND);
            dataModel->insert(pos,ta->transform(input.toUtf8()));
        }
    }
}

void HexView::onInsertBefore(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    int pos = hexTableView->getLowerSelected();
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this);
        if (dialog == nullptr) {
            qFatal("Cannot allocate memory for onInsertBefore NewByteDialog X{");
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            dataModel->insert(pos, QByteArray(dialog->byteCount(),dialog->getChar()));
        }
        delete dialog;
    } else if (action->text() == UTF8_STRING_ACTION) {
        dataModel->insert(pos,input.toUtf8());
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != nullptr) {
            ta->setWay(TransformAbstract::OUTBOUND);
            dataModel->insert(pos,ta->transform(input.toUtf8()));
        }
    }
}

void HexView::onSendToTriggered(QAction *action)
{
    sendToMenu->processingAction(action, hexTableView->getSelectedBytes());
}

void HexView::onMarkMenu(QAction *action)
{
    QString name;
    if (action == ui->newMarkingAction) {
        if (hexTableView->hasSelection()) {
            QColor choosenColor = QColorDialog::getColor(Qt::yellow, this);

            QPixmap pix(20,20);
            pix.fill(choosenColor);
            TextInputDialog *nameDialog = new(std::nothrow) TextInputDialog(this);
            if (nameDialog == nullptr) {
                qFatal("Cannot allocate memory for textInputDialog X{");
            }
            nameDialog->setPixLabel(pix);
            int ret = nameDialog->exec();
            if (ret == QDialog::Accepted) {
                name = nameDialog->getInputText();
                if (!name.isEmpty())
                    guiHelper->addNewMarkingColor(name,choosenColor);
                hexTableView->markSelected(choosenColor, name);
            }
        }

    } else {
        name = action->text();
        QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
        if (colors.contains(name)) {
            hexTableView->markSelected(colors.value(name), name);
        } else {
            qCritical("Unknown marking color T_T");
        }
    }
}

void HexView::onSelectFromSizeMenu(QAction *action)
{
    quint64 val = normalizeSelectedInt(action->text() == BIG_ENDIAN_STRING);
    if (val == 0) {
        return;
    }

    qint64 inter = static_cast<qint64>(byteSource->size() - static_cast<quint64>(hexTableView->getHigherSelected()));
    if (val >= static_cast<quint64>(inter < 0 ? 0 : inter) ) {
        QString mess = tr("This size value would select out-of-bound (maybe the selected value is a signed int)");
        logger->logError(mess);
        QMessageBox::warning(this, tr("Value too large"), mess, QMessageBox::Ok);
    } else {
        qint64 pos = hexTableView->getHigherSelected() + 1;
        hexTableView->selectBytes(static_cast<int>(pos), static_cast<int>(val));
    }
}

void HexView::onGotoFromOffsetMenu(QAction *action)
{
    bool absolute = true;
    quint64 val = 0;
    if (action->text() == ABSOLUTE_BIG_ENDIAN_STRING || action->text() == ABSOLUTE_LITTLE_ENDIAN_STRING) {
        val = normalizeSelectedInt(action->text() == ABSOLUTE_BIG_ENDIAN_STRING);
        if (val == 0) {
            return;
        }
    } else {
        val = normalizeSelectedInt(action->text() == RELATIVE_BIG_ENDIAN_STRING);
        if (val == 0) {
            return;
        }
        qint64 inter = static_cast<qint64>(byteSource->size() - static_cast<quint64>(hexTableView->getHigherSelected()));
        if (val >= static_cast<quint64>(inter < 0 ? 0 : inter)) {
            QString mess = tr("This offset value would go out-of-bound (maybe the selected value is a signed int)");
            logger->logError(mess);
            QMessageBox::warning(this, tr("Value too large"), mess, QMessageBox::Ok);
            return;
        }
        absolute = false;
    }

    if (!hexTableView->goTo(val, absolute,false)) {
        QMessageBox::warning(this, tr("Offset error"), tr("Offset value error"), QMessageBox::Ok);
    }
}

void HexView::onCopySelectionSize(QAction *action)
{
    selectedSize = hexTableView->getSelectedBytesCount();
    QString text;
    if (action->text() == OCTAL_STRING) {
        text = QString::number(selectedSize,8);
    } else if (action->text() == DECIMAL_STRING) {
        text = QString::number(selectedSize,10);
    } else if (action->text() == HEXADECIMAL_STRING) {
        text = QString::number(selectedSize,16);
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void HexView::onCopyCurrentOffset(QAction *action)
{
    if (hexTableView->getCurrentPos() >= 0) {
        quint64 currenOffset = byteSource->getRealOffset(hexTableView->getCurrentPos());
        QString text;
        if (action->text() == OCTAL_STRING) {
            text = QString::number(currenOffset,8);
        } else if (action->text() == DECIMAL_STRING) {
            text = QString::number(currenOffset,10);
        } else if (action->text() == HEXADECIMAL_STRING) {
            text = QString::number(currenOffset,16);
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(text);
    }
}

quint64 HexView::normalizeSelectedInt(bool bigEndian)
{
    QByteArray bytesdata = hexTableView->getSelectedBytes();
    quint64 size = static_cast<quint64>(bytesdata.size()); // converting int to quint64, no drama here
    if (size > 8) {
        QString mess = tr("Too many bytes selected for an uint 64 (max 8 bytes)");
        logger->logError(mess);
        QMessageBox::warning(this, tr("Too many bytes"), mess, QMessageBox::Ok);
        return 0;
    }


    bool reverseByteOrdering = false;

#ifdef Q_LITTLE_ENDIAN
    bool currentSystemLittleEndian = true;
#else
    bool currentSystemLittleEndian = false;
#endif

    if (currentSystemLittleEndian == bigEndian) {
        reverseByteOrdering = true;
    }
    if (size < sizeof(quint64)) {
        if (reverseByteOrdering)
            bytesdata.prepend(QByteArray(static_cast<int>(sizeof(quint64) - static_cast<quint64>(bytesdata.size())),'\x00'));
        else
            bytesdata.append(QByteArray(static_cast<int>(sizeof(quint64) - static_cast<quint64>(bytesdata.size())),'\x00'));
    }

    if (reverseByteOrdering) {
        QByteArray temp2;
        for (int i = bytesdata.size() - 1; i >= 0; i--) {
            temp2.append(bytesdata.at(i));
        }
        bytesdata = temp2;
    }
    quint64 val = 0;

    memcpy(&val,bytesdata.constData(),sizeof(quint64));

    return val;
}

void HexView::onClearSelectionMarkings()
{
    if (hexTableView->hasSelection()) {
        hexTableView->clearMarkOnSelected();
    }
}

void HexView::onClearAllMArkings()
{
    byteSource->clearAllMarkings();
}

bool HexView::goTo(quint64 offset, bool absolute, bool negative, bool select)
{
    return hexTableView->goTo(offset,absolute, negative, select);
}

void HexView::gotoSearch(quint64 soffset, quint64 eoffset)
{
    hexTableView->gotoSearch(soffset, eoffset);
}

void HexView::search(QByteArray item, QBitArray mask)
{
    hexTableView->search(item, mask);
}

void HexView::optionGuiRequested()
{

}

void HexView::onLoadFile()
{
    emit askForFileLoad();
}

QJsonObject HexView::createFuzzingTemplate(ByteSourceAbstract *bs)
{
    QJsonObject fuzzConfiguration;
    if (bs->size() < 1024 * 1024) {
        if (bs->hasMarking()) {
            BytesRangeList * brl = bs->getUserMarkingsRanges();
            QJsonValue jsData = QString::fromUtf8(bs->getRawData().toBase64());
            fuzzConfiguration.insert(GuiConst::STATE_DATA,jsData);
            QJsonArray injectionPoints;
            for (int i = 0 ; i < brl->size(); i++) {
                QJsonObject injp;
                QSharedPointer<BytesRange> br = brl->at(i);
                injp.insert(GuiConst::STATE_TYPE, br->getDescription());
                injp.insert(GuiConst::START_STR,QJsonValue(static_cast<qint64>(br->getLowerVal()))); // size should be reasonable, so it should not matter
                injp.insert(GuiConst::STATE_SIZE, QJsonValue(static_cast<qint64>(br->getSize())));
                injectionPoints.append(injp);
            }
            fuzzConfiguration.insert("injectionsPoints", injectionPoints);
        } else {
            QString mess = tr("No markings set, ignoring action.");
            logger->logError(mess);
            QMessageBox::warning(this, tr("No markings"), mess, QMessageBox::Ok);
        }
    } else {
        QString mess = tr("Data size is too big, if you want to process something bigger, change the max value in the settings, and try to be realistic.");
        logger->logError(mess);
        QMessageBox::warning(this, tr("Too large"), mess, QMessageBox::Ok);
    }

    return fuzzConfiguration;
}

void HexView::onExportForFuzzing()
{
    QJsonDocument::JsonFormat format = QJsonDocument::Indented;
    ByteSourceAbstract *bs = dataModel->getSource();
    QJsonObject finalObj = createFuzzingTemplate(bs);
    if (!finalObj.isEmpty()) {
        QJsonDocument jdoc;
        jdoc.setObject(finalObj);
        QString fileName = QFileDialog::getSaveFileName(this,tr("Choose a file to save to"), GuiConst::GLOBAL_LAST_PATH);
        if (!fileName.isEmpty()) {
            QFileInfo fi(fileName);
            GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                QString mess = tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString());
                logger->logError(mess);
                QMessageBox::warning(this, tr("File error"), mess, QMessageBox::Ok);
            } else {
                QByteArray data = jdoc.toJson(format);
                qint64 written = 0;
                while ((written = file.write(data)) > 0) {
                    if (written == data.length())
                        break;
                    else
                        data = data.mid(static_cast<int>(written) - 1);
                };

                file.close();
            }
        }
    }
}

void HexView::onSaveToFile(QAction *action)
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose a file to save to"), GuiConst::GLOBAL_LAST_PATH);
    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
        if (action == ui->saveSelectedToFileAction) {
            byteSource->saveToFile(fileName,byteSource->getRealOffset(hexTableView->getLowerSelected()), byteSource->getRealOffset(hexTableView->getHigherSelected()));
        }
        else
            byteSource->saveToFile(fileName);
    }
}

void HexView::onSelectAll()
{
    hexTableView->selectAllBytes();
}

void HexView::onKeepOnlySelection()
{
    byteSource->setData(hexTableView->getSelectedBytes());
}

void HexView::onNewByteArray()
{
    NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this);
    if (dialog == nullptr) {
        qFatal("Cannot allocate memory for onNewByteArray NewByteDialog X{");
    }
    dialog->setModal(true);
    int ret = dialog->exec();
    if (ret == QDialog::Accepted) {
        byteSource->setData(QByteArray(dialog->byteCount(),dialog->getChar()));
    }
    delete dialog;
}

void HexView::onDeleteSelection()
{
    hexTableView->deleteSelectedBytes();
}

void HexView::onSelectionChanged()
{
    guiHelper->sendNewSelection(hexTableView->getSelectedBytes());
    updateStats();
}

void HexView::buildContextMenus()
{
    sendToMenu = new(std::nothrow) SendToMenu(guiHelper, tr("Send selection to"));
    if (sendToMenu == nullptr) {
        qFatal("Cannot allocate memory for sendToMenu X{");
    }
    connect(sendToMenu, &SendToMenu::triggered, this, &HexView::onSendToTriggered);

    markMenu = new(std::nothrow) QMenu(tr("Mark as"));
    if (markMenu == nullptr) {
        qFatal("Cannot allocate memory for markMenu X{");
    }
    connect(markMenu, &QMenu::triggered, this, &HexView::onMarkMenu);

    copyMenu = new(std::nothrow) QMenu(tr("Copy as"));
    if (copyMenu == nullptr) {
        qFatal("Cannot allocate memory for copyMenu X{");
    }
    connect(copyMenu, &QMenu::triggered, this, &HexView::onCopy);

    loadMenu = new(std::nothrow) QMenu(tr("Load from clipboard"));
    if (loadMenu == nullptr) {
        qFatal("Cannot allocate memory for importMenu X{");
    }
    connect(loadMenu, &QMenu::triggered, this, &HexView::onLoad);

    insertAfterMenu = new(std::nothrow) QMenu(tr("Insert after"));
    if (insertAfterMenu == nullptr) {
        qFatal("Cannot allocate memory for insertAfterMenu X{");
    }
    connect(insertAfterMenu, &QMenu::triggered, this, &HexView::onInsertAfter);

    replaceMenu = new(std::nothrow) QMenu(tr("Replace selection "));
    if (replaceMenu == nullptr) {
        qFatal("Cannot allocate memory for replaceMenu X{");
    }
    connect(replaceMenu, &QMenu::triggered, this, &HexView::onReplace);

    insertBeforeMenu = new(std::nothrow) QMenu(tr("Insert before"));
    if (insertBeforeMenu == nullptr) {
        qFatal("Cannot allocate memory for insertBeforeMenu X{");
    }
    connect(insertBeforeMenu, &QMenu::triggered, this, &HexView::onInsertBefore);

    selectFromSizeMenu = new(std::nothrow) QMenu(tr("Select from size"));
    if (selectFromSizeMenu == nullptr) {
        qFatal("Cannot allocate memory for selectFromSizeMenu X{");
    }
    connect(selectFromSizeMenu, &QMenu::triggered, this, &HexView::onSelectFromSizeMenu);

    QAction * action = new(std::nothrow) QAction(LITTLE_ENDIAN_STRING, selectFromSizeMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SELECT_LE_ACTION X{");
    }
    selectFromSizeMenu->addAction(action);

    action = new(std::nothrow) QAction(BIG_ENDIAN_STRING, selectFromSizeMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SELECT_BE_ACTION X{");
    }
    selectFromSizeMenu->addAction(action);

    gotoFromOffsetMenu = new(std::nothrow) QMenu(tr("Goto selected offset"));
    if (gotoFromOffsetMenu == nullptr) {
        qFatal("Cannot allocate memory for gotoFromOffsetMenu X{");
    }
    connect(gotoFromOffsetMenu, &QMenu::triggered, this, &HexView::onGotoFromOffsetMenu);

    action = new(std::nothrow) QAction(ABSOLUTE_LITTLE_ENDIAN_STRING, gotoFromOffsetMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SELECT_LE_ACTION X{");
    }
    gotoFromOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(ABSOLUTE_BIG_ENDIAN_STRING, gotoFromOffsetMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SELECT_LE_ACTION X{");
    }
    gotoFromOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(RELATIVE_LITTLE_ENDIAN_STRING, gotoFromOffsetMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SELECT_BE_ACTION X{");
    }
    gotoFromOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(RELATIVE_BIG_ENDIAN_STRING, gotoFromOffsetMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SELECT_BE_ACTION X{");
    }
    gotoFromOffsetMenu->addAction(action);

    copySelectionSizeMenu = new(std::nothrow) QMenu(tr("Copy selection size"));
    if (copySelectionSizeMenu == nullptr) {
        qFatal("Cannot allocate memory for copySelectionSizeMenu X{");
    }
    connect(copySelectionSizeMenu, &QMenu::triggered, this, &HexView::onCopySelectionSize);

    action = new(std::nothrow) QAction(OCTAL_STRING, copySelectionSizeMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SIZE_OCTAL_ACTION X{");
    }
    copySelectionSizeMenu->addAction(action);

    action = new(std::nothrow) QAction(DECIMAL_STRING, copySelectionSizeMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SIZE_DECIMAL_ACTION X{");
    }
    copySelectionSizeMenu->addAction(action);

    action = new(std::nothrow) QAction(HEXADECIMAL_STRING, copySelectionSizeMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SIZE_HEXADECIMAL_ACTION X{");
    }
    copySelectionSizeMenu->addAction(action);

    copyCurrentOffsetMenu = new(std::nothrow) QMenu(tr("Copy offset value"));
    if (copyCurrentOffsetMenu == nullptr) {
        qFatal("Cannot allocate memory for copyCurrentOffsetMenu X{");
    }
    connect(copyCurrentOffsetMenu, &QMenu::triggered, this, &HexView::onCopyCurrentOffset);

    action = new(std::nothrow) QAction(OCTAL_STRING, copyCurrentOffsetMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SIZE_OCTAL_ACTION X{");
    }
    copyCurrentOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(DECIMAL_STRING, copyCurrentOffsetMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SIZE_DECIMAL_ACTION X{");
    }
    copyCurrentOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(HEXADECIMAL_STRING, copyCurrentOffsetMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for SIZE_HEXADECIMAL_ACTION X{");
    }
    copyCurrentOffsetMenu->addAction(action);

    saveToFileMenu = new(std::nothrow) QMenu(tr("Save to file"));
    if (saveToFileMenu == nullptr) {
        qFatal("Cannot allocate memory for saveToFile X{");
    }
    connect(saveToFileMenu, &QMenu::triggered, this, &HexView::onSaveToFile);
    saveToFileMenu->addAction(ui->saveToFileAction);
    saveToFileMenu->addAction(ui->saveSelectedToFileAction);

    fuzzingExportAction = new(std::nothrow) QAction("Export Fuzzing config", globalContextMenu);
    if (fuzzingExportAction == nullptr) {
        qFatal("Cannot allocate memory for SIZE_HEXADECIMAL_ACTION X{");
    }

    connect(fuzzingExportAction, &QAction::triggered, this, &HexView::onExportForFuzzing);

    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == nullptr) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
    }

    globalContextMenu->addAction(ui->selectAllAction);
    connect(ui->selectAllAction, &QAction::triggered, this, &HexView::onSelectAll);
    globalContextMenu->addAction(ui->keepOnlySelectionAction);
    globalContextMenu->addMenu(selectFromSizeMenu);
    globalContextMenu->addMenu(gotoFromOffsetMenu);
    connect(ui->keepOnlySelectionAction, &QAction::triggered, this, &HexView::onKeepOnlySelection);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(ui->newByteArrayAction);
    connect(ui->newByteArrayAction, &QAction::triggered, this, &HexView::onNewByteArray);
    globalContextMenu->addAction(ui->importFileAction);
    connect(ui->importFileAction, &QAction::triggered, this, &HexView::onLoadFile);
    globalContextMenu->addMenu(loadMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(replaceMenu);
    globalContextMenu->addMenu(insertAfterMenu);
    globalContextMenu->addMenu(insertBeforeMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(markMenu);
    globalContextMenu->addAction(ui->clearMarkingsAction);
    connect(ui->clearMarkingsAction, &QAction::triggered, this, &HexView::onClearSelectionMarkings);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(saveToFileMenu);
    globalContextMenu->addMenu(copyMenu);
    globalContextMenu->addMenu(copySelectionSizeMenu);
    globalContextMenu->addMenu(copyCurrentOffsetMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(ui->deleteSelectionAction);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(fuzzingExportAction);
    connect(ui->deleteSelectionAction, &QAction::triggered, this, &HexView::onDeleteSelection);
}

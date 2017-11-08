/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "textview.h"
#include "ui_textview.h"
#include <QFont>
#include <QMimeData>
#include <QScrollBar>
#include <QDebug>
#include <QTextCodec>
#include <QMessageBox>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QTextEncoder>
#include <QTextDecoder>
#include <QMenu>
#include <QAction>
#include <transformabstract.h>
#include <QClipboard>
#include "tabs/tababstract.h"
#include "sources/bytesourceabstract.h"
#include "loggerwidget.h"
#include "guihelper.h"
#include "shared/sendtomenu.h"
#include "shared/guiconst.h"

#ifdef SCINTILLA
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexeryaml.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexerproperties.h>
#else
#include <QTextCursor>
#endif

using namespace GuiConst;

// We need to set this limit as QTextEdit has difficulties with large input
const int TextView::MAX_TEXT_VIEW = 100000;
const QString TextView::DEFAULT_CODEC = "UTF-8";
const QString TextView::LOGID = "TextView";
const QString TextView::COPY_AS_TEXT = "Text";

TextView::TextView(ByteSourceAbstract *nbyteSource, GuiHelper *nguiHelper, QWidget *parent, bool takeByteSourceOwnership) :
    SingleViewAbstract(nbyteSource, nguiHelper, parent, takeByteSourceOwnership)
{
    connect(byteSource,SIGNAL(updated(quintptr)), this, SLOT(updateText(quintptr)), Qt::UniqueConnection);
    ui = new(std::nothrow) Ui::TextView();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::TextView X{");
    }

    updateTimer.setInterval(150);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, SIGNAL(timeout()), SLOT(updateStats()));
    globalContextMenu = nullptr;
    sendToMenu = nullptr;
    loadMenu = nullptr;
    copyMenu = nullptr;
    loadFileAction = nullptr;
    saveToFileMenu = nullptr;
    selectAllAction = nullptr;
    keepOnlySelectedAction = nullptr;
    copyAsTextAction = nullptr;
    currentCodec = nullptr;
    errorNotReported = true;
    autoCopyToClipboard = false;
    ui->setupUi(this);

#ifdef SCINTILLA
    lexerCombobox = nullptr;
    scintEditor = new(std::nothrow) QsciScintilla();
    if (scintEditor == nullptr) {
        qFatal("Cannot allocate memory for QsciScintilla X{");
    }
    ui->mainLayout->insertWidget(0,scintEditor);
    scintEditor->installEventFilter(this);
    connect(scintEditor, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(scintEditor,SIGNAL(selectionChanged()), this, SLOT(updateStats()));
    scintEditor->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    scintEditor->setReadOnly(byteSource->isReadonly());
    scintEditor->setWrapMode(QsciScintilla::WrapCharacter);

    lexerCombobox = new(std::nothrow) QComboBox();
    if (lexerCombobox == nullptr) {
        qFatal("Cannot allocate memory for QComboBox X{");
    }

    QStringList lexersStrings = QStringList()
                                << QString("No syntax")
                                << QString("HTML")
                                << QString("JavaScript")
                                << QString("YAML")
                                << QString("XML")
                                << QString("Properties");
    lexerCombobox->addItems(lexersStrings);
    lexerCombobox->setFrame(false);
    ui->statusLayout->insertWidget(ui->statusLayout->indexOf(ui->codecsComboBox), lexerCombobox);
    connect(lexerCombobox, SIGNAL(currentIndexChanged(int)), SLOT(onLexerChanged(int)));
    connect(scintEditor, SIGNAL(cursorPositionChanged(int,int)),&updateTimer, SLOT(start()));

#else
    plainTextEdit = new(std::nothrow) QPlainTextEdit();
    if (plainTextEdit == nullptr) {
        qFatal("Cannot allocate memory for QPlainTextEdit X{");
    }

    ui->mainLayout->insertWidget(0,plainTextEdit);
    plainTextEdit->installEventFilter(this);
    connect(plainTextEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(plainTextEdit,SIGNAL(selectionChanged()), &updateTimer, SLOT(start()));
    plainTextEdit->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    plainTextEdit->setReadOnly(byteSource->isReadonly());
    connect(plainTextEdit, SIGNAL(cursorPositionChanged()),&updateTimer, SLOT(start()));
#endif

    connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportMenu()));
    setAcceptDrops(true);
    connect(byteSource, SIGNAL(readOnlyChanged(bool)), this, SLOT(onReadOnlyChanged(bool)));

    QList<QByteArray> codecs =  QTextCodec::availableCodecs();
    std::sort(codecs.begin(),codecs.end());

    for (int i = 0; i < codecs.size(); i++) {
        ui->codecsComboBox->addItem(QString(codecs.at(i)),QVariant(codecs.at(i)));
    }
    ui->codecsComboBox->setCurrentIndex(ui->codecsComboBox->findData(DEFAULT_CODEC));
    ui->codecsComboBox->setMaximumWidth(200);
    ui->codecsComboBox->installEventFilter(guiHelper);
    connect(ui->codecsComboBox,SIGNAL(currentIndexChanged(QString)), this, SLOT(onCodecChange(QString)));
    connect(guiHelper, SIGNAL(hexTableSizesUpdated()), this, SLOT(onFontUpdated()));

    onCodecChange(DEFAULT_CODEC);
    buildContextMenu();
}

TextView::~TextView()
{
#ifdef SCINTILLA
    delete scintEditor;
#else
    delete plainTextEdit;
#endif
    delete sendToMenu;
    delete saveToFileMenu;
    delete loadMenu;
    delete copyMenu;
    delete loadFileAction;
    delete selectAllAction;
    delete keepOnlySelectedAction;
    delete globalContextMenu;
    delete ui;
    logger = nullptr;
    guiHelper = nullptr;
}

void TextView::setModel(ByteSourceAbstract *dataModel)
{
    byteSource = dataModel;
    connect(byteSource,SIGNAL(updated(quintptr)), this, SLOT(updateText(quintptr)), Qt::UniqueConnection);
}

void TextView::search(QByteArray block, QBitArray)
{
    QString item = QString::fromUtf8(block.data(), block.size());
    previousSearch = block;
    previousMask = QBitArray(); // disabled for text search

#ifdef SCINTILLA
    bool found = scintEditor->findFirst(item,false,true,false,true);
#else
    bool found = plainTextEdit->find(item);
    if (!found) {
        QTextCursor cursor(plainTextEdit->textCursor());
        cursor.movePosition(QTextCursor::Start);
        plainTextEdit->setTextCursor(cursor);
        found = plainTextEdit->find(item);
    }
#endif

    emit searchStatus(!found);
}

void TextView::copyToClipboard()
{
    QString data;
#ifdef SCINTILLA
    data = scintEditor->text();
#else
    data = plainTextEdit->toPlainText();
#endif
    if (!data.isEmpty()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(data);
    }
}

void TextView::onRightClick(QPoint pos)
{
    // if any selection
#ifdef SCINTILLA
    bool hasTextSelection = scintEditor->hasSelectedText();
    bool isEmpty = scintEditor->text().isEmpty();
#else
    bool hasTextSelection = plainTextEdit->textCursor().hasSelection();
    bool isEmpty = plainTextEdit->toPlainText().isEmpty();
#endif
    sendToMenu->setEnabled(hasTextSelection);
    copyMenu->setEnabled(hasTextSelection);
    keepOnlySelectedAction->setEnabled(hasTextSelection);
    if (isEmpty) {
        saveToFileMenu->setEnabled(false);
        selectAllAction->setEnabled(false);
    } else {
        saveToFileMenu->setEnabled(true);
        selectAllAction->setEnabled(true);
    }
    loadFileAction->setEnabled(byteSource->hasCapability((ByteSourceAbstract::CAP_LOADFILE))
                               && byteSource->hasCapability(ByteSourceAbstract::CAP_WRITE));
    globalContextMenu->exec(this->mapToGlobal(pos));
}

void TextView::onLoad(QAction * action)
{
    guiHelper->loadAction(action->text(), byteSource);
}

void TextView::onCopy(QAction *action)
{
    if (action == copyAsTextAction) {
#ifdef SCINTILLA
        scintEditor->copy();
#else
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(plainTextEdit->textCursor().selection().toPlainText());
#endif
    } else {
        QString data;
#ifdef SCINTILLA
        data = scintEditor->selectedText();
#else
        data = plainTextEdit->textCursor().selection().toPlainText();
#endif
        guiHelper->copyAction(action->text(), encode(data));
    }
}

void TextView::onLoadFile()
{
    emit askForFileLoad();
}

void TextView::onSendToTriggered(QAction * action)
{
    QByteArray dataToSend;
#ifdef SCINTILLA
    dataToSend = encode(scintEditor->selectedText());
#else
    dataToSend = encode(plainTextEdit->textCursor().selection().toPlainText());
#endif
    sendToMenu->processingAction(action, dataToSend);
}

void TextView::onSelectAll()
{
#ifdef SCINTILLA
    scintEditor->selectAll();
#else
    plainTextEdit->selectAll();
#endif

}

void TextView::onKeepOnlySelection()
{
    QString newText;
#ifdef SCINTILLA
    newText = scintEditor->selectedText();
    scintEditor->clear();
    scintEditor->setText(newText);
#else
    newText = plainTextEdit->textCursor().selection().toPlainText();
    plainTextEdit->clear();
    plainTextEdit->appendPlainText(newText);
#endif

}

void TextView::onCodecChange(QString codecName)
{
    QTextCodec *codec = QTextCodec::codecForName(codecName.toUtf8());
    if (codec == nullptr) {
       logger->logError(tr("Cannot found the text codec: %1. Ignoring request.").arg(codecName),LOGID);
    } else {
        currentCodec = codec;
        updateText(0);
    }
}

void TextView::onReadOnlyChanged(bool readonly)
{
#ifdef SCINTILLA
    scintEditor->setReadOnly(readonly);
#else
    plainTextEdit->setReadOnly(readonly);
#endif

}

void TextView::onSaveToFile(QAction * action)
{

    if (action == ui->saveSelectedToFileAction) {
        QString tdata;
#ifdef SCINTILLA
        tdata = scintEditor->selectedText();
#else
        tdata = plainTextEdit->textCursor().selection().toPlainText();
#endif
        if (!tdata.isEmpty()) {
            guiHelper->saveToFileAction(encode(tdata),this);
        }
        else {
            QString mess = tr("No text selected when save requested");
            logger->logError(mess);
            QMessageBox::critical(this,tr("Error"), mess,QMessageBox::Ok);
        }
    }
    else
        guiHelper->saveToFileAction(byteSource->getRawData(),this);
}

void TextView::onFontUpdated()
{
#ifdef SCINTILLA
    scintEditor->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
#else
    plainTextEdit->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
#endif
}

#ifdef SCINTILLA
void TextView::onLexerChanged(int index)
{
    QsciLexer *prevLexer = scintEditor->lexer();
    QsciLexer *newLexer = nullptr;
    switch (index) { // do nothing by default
        case 0:
            break;
        case 1:
            newLexer = new(std::nothrow) QsciLexerHTML();
            break;
        case 2:
            newLexer = new(std::nothrow) QsciLexerJavaScript();
            break;
        case 3:
            newLexer = new(std::nothrow) QsciLexerYAML();
            break;
        case 4:
            newLexer = new(std::nothrow) QsciLexerXML();
            break;
        case 5:
            newLexer = new(std::nothrow) QsciLexerProperties();
            break;
        default:
            logger->logError(tr("Unknown Lexer index: %1 T_T").arg(index));

    }
    if (newLexer != nullptr) {
        newLexer->setDefaultFont(GlobalsValues::GLOBAL_REGULAR_FONT);
        newLexer->setDefaultColor(QApplication::palette().text().color());
    }
    scintEditor->setLexer(newLexer);
    delete prevLexer;

    if (newLexer == nullptr) {
        scintEditor->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    }
}

QString TextView::getCurrentSyntax()
{
    return lexerCombobox->currentText();
}

void TextView::setCurrentSyntax(QString syntaxName)
{
    int index = lexerCombobox->findText(syntaxName);
    if (index > 0)
        lexerCombobox->setCurrentIndex(index);
}

#endif

QHash<QString, QString> TextView::getConfiguration()
{
    QHash<QString, QString> conf = SingleViewAbstract::getConfiguration();
    conf.insert(GuiConst::STATE_ENCODING, ui->codecsComboBox->currentText());
#ifdef SCINTILLA
    conf.insert(GuiConst::STATE_SYNTAX, lexerCombobox->currentText());
#endif
    return conf;
}

void TextView::setConfiguration(QHash<QString, QString> conf)
{
    SingleViewAbstract::setConfiguration(conf);

    if (conf.contains(GuiConst::STATE_ENCODING)) {
        int index = ui->codecsComboBox->findText(conf.value(GuiConst::STATE_ENCODING));
        if (index > 0) {
            ui->codecsComboBox->setCurrentIndex(index);
        }
    }

#ifdef SCINTILLA
    if (conf.contains(GuiConst::STATE_SYNTAX)) {
        int index = lexerCombobox->findText(conf.value(GuiConst::STATE_SYNTAX));
        if (index > 0) {
            lexerCombobox->setCurrentIndex(index);
        }
    }
#endif
}

void TextView::updateImportExportMenu()
{
    guiHelper->updateLoadContextMenu(loadMenu);
    guiHelper->updateCopyContextMenu(copyMenu);

    QAction *firstAction = copyMenu->actions().at(0);
    copyMenu->insertAction(firstAction,copyAsTextAction);
    copyMenu->insertSeparator(firstAction);
}

void TextView::buildContextMenu()
{
#ifdef SCINTILLA
    scintEditor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(scintEditor,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClick(QPoint)));
#else
    plainTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plainTextEdit,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClick(QPoint)));
#endif


    sendToMenu = new(std::nothrow) SendToMenu(guiHelper, tr("Send selection to"));
    if (sendToMenu == nullptr) {
        qFatal("Cannot allocate memory for sendToMenu X{");
        return;
    }

    connect(sendToMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSendToTriggered(QAction*)), Qt::UniqueConnection);

    loadMenu = new(std::nothrow) QMenu(tr("Load from clipboard"));
    if (loadMenu == nullptr) {
        qFatal("Cannot allocate memory for importMenu X{");
        return;
    }
    guiHelper->updateLoadContextMenu(loadMenu);
    connect(loadMenu, SIGNAL(triggered(QAction*)), this, SLOT(onLoad(QAction*)), Qt::UniqueConnection);

    copyMenu = new(std::nothrow) QMenu(tr("Copy as"));
    if (copyMenu == nullptr) {
        qFatal("Cannot allocate memory for copyMenu X{");
        return;
    }

    guiHelper->updateCopyContextMenu(copyMenu);
    QAction *firstAction = copyMenu->actions().at(0);
    copyAsTextAction = new(std::nothrow) QAction(COPY_AS_TEXT, this);
    if (copyAsTextAction == nullptr) {
        qFatal("Cannot allocate memory for copyAsTextAction X{");
    }

    copyMenu->insertAction(firstAction,copyAsTextAction);
    copyMenu->insertSeparator(firstAction);
    connect(copyMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCopy(QAction*)), Qt::UniqueConnection);

    saveToFileMenu = new(std::nothrow) QMenu(tr("Save to file"));
    if (saveToFileMenu == nullptr) {
        qFatal("Cannot allocate memory for saveToFile X{");
        return;
    }
    connect(saveToFileMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSaveToFile(QAction*)), Qt::UniqueConnection);
    saveToFileMenu->addAction(ui->saveAllToFileAction);
    saveToFileMenu->addAction(ui->saveSelectedToFileAction);

    loadFileAction = new(std::nothrow) QAction("Load from file", this);
    if (loadFileAction == nullptr) {
        qFatal("Cannot allocate memory for loadFileAction X{");
    }
    connect(loadFileAction, SIGNAL(triggered()), this, SLOT(onLoadFile()));

    selectAllAction = new(std::nothrow) QAction("Select all", this);
    if (selectAllAction == nullptr) {
        qFatal("Cannot allocate memory for selectAllAction X{");
    }
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(onSelectAll()));

    keepOnlySelectedAction = new(std::nothrow) QAction("Keep only selected", this);
    if (keepOnlySelectedAction == nullptr) {
        qFatal("Cannot allocate memory for keepOnlySelectedAction X{");
    }

    connect(keepOnlySelectedAction, SIGNAL(triggered()), this, SLOT(onKeepOnlySelection()));

    globalContextMenu = new(std::nothrow) QMenu();

    if (globalContextMenu == nullptr) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
        return;
    }

    globalContextMenu->addAction(selectAllAction);
    globalContextMenu->addAction(keepOnlySelectedAction);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(loadFileAction);
    globalContextMenu->addMenu(loadMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addMenu(copyMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(saveToFileMenu);

}

void TextView::onTextChanged()
{
    QString sdata;
#ifdef SCINTILLA
    sdata = scintEditor->text();
#else
    sdata = plainTextEdit->toPlainText();
#endif
    byteSource->setData(encode(sdata),(quintptr) this);
    updateStats();
    if (autoCopyToClipboard)
        copyToClipboard();
}

void TextView::updateText(quintptr source)
{
    if (source == (quintptr) this)
        return;

    QByteArray rawdata = byteSource->getRawData();
#ifdef SCINTILLA
    scintEditor->blockSignals(true);
    if (rawdata.size() > 0) {
        if (currentCodec != nullptr) { //safeguard
            QTextDecoder *decoder = currentCodec->makeDecoder(QTextCodec::ConvertInvalidToNull); // when decoding take possible BOM headers into account
            QString textf = decoder->toUnicode(rawdata.constData(),rawdata.size());
            if (decoder->hasFailure()) {
                if (errorNotReported) {
                    logger->logWarning(tr("invalid text decoding [%1]").arg(QString::fromUtf8(currentCodec->name())),LOGID);
                    ui->codecsComboBox->setStyleSheet(GuiStyles::ComboBoxError);
                    errorNotReported = false;
                }
            } else {
                ui->codecsComboBox->setStyleSheet("");
                errorNotReported = true;
            }
            delete decoder;
            scintEditor->setText(textf);
            updateStats();
            scintEditor->setCursorPosition(0,0);
            scintEditor->ensureCursorVisible();
        } else {
            qCritical() << tr("%1:updatedText() currentCodec is nullptr T_T").arg(metaObject()->className());
        }

        if (autoCopyToClipboard)
            copyToClipboard();
    } else {
        scintEditor->setText(QString());
        updateStats();
    }

    scintEditor->blockSignals(false);
#else
    plainTextEdit->blockSignals(true);
    plainTextEdit->clear();
    if (rawdata.size() > MAX_TEXT_VIEW) {
        plainTextEdit->appendPlainText("Data Too large for this view");
        plainTextEdit->blockSignals(false);
        plainTextEdit->setEnabled(false);
        ui->statsLabel->setText(QString(""));
        ui->codecsComboBox->setStyleSheet("");
        emit invalidText();
    } else {
        if (rawdata.size() > 0) {
            if (currentCodec != nullptr) { //safeguard
                QTextDecoder *decoder = currentCodec->makeDecoder(QTextCodec::ConvertInvalidToNull); // when decoding take possible BOM headers into account
                QString textf = decoder->toUnicode(rawdata.constData(),rawdata.size());
                if (decoder->hasFailure()) {
                    if (errorNotReported) {
                        logger->logWarning(tr("invalid text decoding [%1]").arg(QString::fromUtf8(currentCodec->name())),LOGID);
                        ui->codecsComboBox->setStyleSheet(GuiStyles::ComboBoxError);
                        errorNotReported = false;
                    }
                } else {
                    ui->codecsComboBox->setStyleSheet("");
                    errorNotReported = true;
                }
                delete decoder;
                plainTextEdit->appendPlainText(textf);
                updateStats();
                plainTextEdit->moveCursor(QTextCursor::Start);
                plainTextEdit->ensureCursorVisible();
                plainTextEdit->setEnabled(true);
            } else {
                qCritical() << tr("%1:updatedText() currentCodec is nullptr T_T").arg(metaObject()->className());
            }
        } else {
            plainTextEdit->setEnabled(true);
        }
        if (autoCopyToClipboard)
            copyToClipboard();
    }
    plainTextEdit->blockSignals(false);
#endif
}

void TextView::updateStats()
{
    QString plainText;
    QString selection;
    int selectedSize = 0;
    int currentLine = -1;
    int currentcolumn = -1;

#ifdef SCINTILLA
    plainText = scintEditor->text();
    selection = scintEditor->selectedText();
    selectedSize = selection.size();
    scintEditor->getCursorPosition(&currentLine,&currentcolumn);
#else
    if (plainTextEdit->isEnabled()) {
        plainText = plainTextEdit->toPlainText();
        QTextCursor cursor = plainTextEdit->textCursor();
        if (cursor.hasSelection()){
            selection = cursor.selection().toPlainText();
            selectedSize = cursor.selectionEnd() - cursor.selectionStart();
        }

        currentLine = plainTextEdit->textCursor().blockNumber() + 1;
        currentcolumn = plainTextEdit->textCursor().columnNumber() + 1;
    }
#endif

    if (selectedSize > 0){
        // that should not be here but that's just easier as updateStats is
        // called everytime the selection change anyway
        guiHelper->sendNewSelection(encode(selection));
    }
    // updating text info
    QString ret = "Size: ";
    ret.append(QString::number(plainText.size())).append(tr(" characters"));
    if (selectedSize > 0)
        ret.append(tr(" (%1 selected) |").arg(selectedSize));
    ret.append(tr(" Lines: ")).append((plainText.size() > 0 ? QString::number(plainText.count("\n") + 1) : QString::number(0)));

    if (currentLine > 0)
        ret.append(tr("| Line: %1 Col:%2").arg(currentLine).arg(currentcolumn));

    ui->statsLabel->setText(ret);
}


bool TextView::eventFilter(QObject *obj, QEvent *event)
{
#ifdef SCINTILLA
    QAbstractScrollArea * sa = scintEditor;
#else
    QAbstractScrollArea * sa = plainTextEdit;
#endif

    if (obj == sa) {
        sa->setAttribute(Qt::WA_NoMousePropagation, false);
        if (event->type() == QEvent::KeyPress) {
             QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
             switch (keyEvent->key())
             {
                 case Qt::Key_Z:
                     if (keyEvent->modifiers().testFlag(Qt::ShiftModifier) && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                         byteSource->historyForward();
                         keyEvent->accept();
                         return true;
                     }
                     else if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                         byteSource->historyBackward();
                         keyEvent->accept();
                         return true;
                     }
                    break;
                 default:
                     return false;
             }

        } else if (event->type() == QEvent::Wheel) {
            if (sa->verticalScrollBar()->isVisible()) {
                sa->setAttribute(Qt::WA_NoMousePropagation);
            }

            return false;
        }
    }

    return QWidget::eventFilter(obj, event);

}

QByteArray TextView::encode(QString text)
{
    QByteArray ret;
    if (currentCodec != nullptr) { // safeguard
        QTextEncoder *encoder = currentCodec->makeEncoder(QTextCodec::ConvertInvalidToNull | QTextCodec::IgnoreHeader);

        ret = encoder->fromUnicode(text);
        if (encoder->hasFailure()) {
            logger->logError(tr("Some error(s) occured during the encoding process [%1]").arg(QString::fromUtf8(currentCodec->name())),LOGID);
            ui->codecsComboBox->setStyleSheet(GuiStyles::ComboBoxError);
        } else {
            ui->codecsComboBox->setStyleSheet(qApp->styleSheet());
        }
        delete encoder;
    }  else {
        logger->logError(tr(":encode() currentCodec is nullptr T_T"),LOGID);
    }
    return ret;
}

bool TextView::isAutoCopyToClipboard() const
{
    return autoCopyToClipboard;
}

void TextView::setAutoCopyToClipboard(bool value)
{
    if (autoCopyToClipboard != value) {
        autoCopyToClipboard = value;
        copyToClipboard();
    }
}




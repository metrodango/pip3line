/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QHash>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QThread>
#include <commonstrings.h>
#include <QTimer>
#include <QComboBox>
#include "singleviewabstract.h"

class QTextCodec;
class QMenu;
class QAction;
class SendToMenu;
#ifdef SCINTILLA
class QsciScintilla;
#else
#include <QPlainTextEdit>
#endif

namespace Ui {
    class TextView;
}

class TextView : public SingleViewAbstract
{
        Q_OBJECT
        
    public:
        explicit TextView(ByteSourceAbstract *byteSource, GuiHelper *guiHelper, QWidget *parent = nullptr, bool takeByteSourceOwnership = false);
        ~TextView();
        void setModel(ByteSourceAbstract *byteSource);
        void search(QByteArray item, QBitArray mask = QBitArray());
        void copyToClipboard();
        bool isAutoCopyToClipboard() const;
#ifdef SCINTILLA
        QString getCurrentSyntax();
        void setCurrentSyntax(QString syntaxName);
#endif
        QHash<QString, QString> getConfiguration();
        void setConfiguration(QHash<QString, QString> conf);
    public slots:
        void setAutoCopyToClipboard(bool value);

    signals:
        void invalidText();
        void searchStatus(bool);
    private slots:
        void updateImportExportMenu();
        void onTextChanged();
        void updateText(quintptr source);
        void updateStats();
        void onRightClick(QPoint pos);
        void onLoad(QAction *action);
        void onCopy(QAction *action);
        void onLoadFile();
        void onSendToTriggered(QAction *action);
        void onSelectAll();
        void onKeepOnlySelection();
        void onCodecChange(const QString &codecName);
        void onReadOnlyChanged(bool viewIsReadonly);
        void onSaveToFile(QAction* action);
        void onFontUpdated();
#ifdef SCINTILLA
        void onLexerChanged(int index);
#endif
    private:
        Q_DISABLE_COPY(TextView)
        void buildContextMenu();
        void checkForEncodingError();
        bool eventFilter(QObject *obj, QEvent *event);
        QByteArray encode(QString text);
        static const int MAX_TEXT_VIEW;
        static const QString DEFAULT_CODEC;
        static const QString LOGID;
        static const QString COPY_AS_TEXT;
        QMenu * globalContextMenu;
        SendToMenu * sendToMenu;
        QMenu * copyMenu;
        QMenu * loadMenu;
        QAction * loadFileAction;
        QAction * selectAllAction;
        QAction * keepOnlySelectedAction;
        QAction * copyAsTextAction;
        QMenu * saveToFileMenu;
        Ui::TextView *ui;
#ifdef SCINTILLA
        QsciScintilla * scintEditor;
        QComboBox * lexerCombobox;
#else
        QPlainTextEdit * plainTextEdit;
#endif
        QTextCodec *currentCodec;
        bool errorNotReported;
        bool autoCopyToClipboard;
        QTimer updateTimer;
};

#endif // TEXTVIEW_H

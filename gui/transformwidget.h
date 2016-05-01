/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMWIDGET_H
#define TRANSFORMWIDGET_H

#include <QWidget>
#include <QByteArray>
#include <QBitArray>
#include <QTableWidgetItem>
#include <QMenu>
#include <QColor>
#include <transformmgmt.h>
#include <transformabstract.h>
#include <QNetworkAccessManager>
#include "infodialog.h"
#include <QMutex>
#include <QTime>
#include <QUrl>
#include <QLineEdit>
#include "sources/bytesourceabstract.h"
#include "downloadmanager.h"
#include "guihelper.h"
#include "loggerwidget.h"
#include "views/byteitemmodel.h"
#include "shared/guiconst.h"

class HexView;
class TextView;
class OffsetGotoWidget;
class SearchWidget;
class ClearAllMarkingsButton;
class ByteTableView;
class TransformRequest;
class MessagePanelWidget;

namespace Ui {
class TransformWidget;
}

class TransformWidget : public QWidget
{
    Q_OBJECT
    
    public:
        explicit TransformWidget(GuiHelper *guiHelper ,QWidget *parent = 0);
        ~TransformWidget();
        QByteArray output();
        TransformAbstract *getTransform();
        void forceUpdating();
        bool setTransform(TransformAbstract *transf);
        ByteSourceAbstract * getSource();
        ByteTableView *getHexTableView();
        void copyTextToClipboard();
        BaseStateAbstract *getStateMngtObj();
        QString getDescription();
        bool isFolded() const;
        void setFolded(bool value);

    signals:
        void updated();
        void transfoRequest();
        void transformChanged();
        void confErrors(QString,QString);
        void error(QString, QString);
        void warning(const QString, const QString);
        void status(const QString, const QString);
        void deletionRequest();
        void tryNewName(QString name);
        void sendRequest(TransformRequest *);
        void insertRequest();
        void foldRequest();
        void resetDone();
    public slots:
        void input(QByteArray inputdata);
        void updatingFrom();
        void logWarning(const QString message, const QString source = QString());
        void logError(const QString message, const QString source = QString());
        void logStatus(const QString message, const QString source = QString());
        void reset();
        void fromLocalFile(QString fileName);
        void deleteMe();
        void setAutoCopyTextToClipboard(bool val);
        void onFoldRequest();
        void onCurrentTabChanged(int index);
    private slots:
        void refreshOutput();
        void onFileLoadRequest();
        void processingFinished(QByteArray output, Messages messages);
        void buildSelectionArea();
        void updateView(quintptr source);
        void onInvalidText();
        void onTransformSelected(QString name);
        void updatingFromTransform();
        void onHistoryBackward();
        void onHistoryForward();
        void on_encodeRadioButton_toggled(bool checked);
        void on_decodeRadioButton_toggled(bool checked);
        void on_infoPushButton_clicked();
        void on_clearDataPushButton_clicked();
        void onSearch(QByteArray item, QBitArray mask, bool couldBeText);
        void onGotoOffset(quint64 offset, bool absolute, bool negative, bool select);
    private:
        Q_DISABLE_COPY(TransformWidget)
        static const int MAX_DIRECTION_TEXT;
        static const QString NEW_BYTE_ACTION;
        void integrateTransform();
        void configureViewArea();
        void clearCurrentTransform();
        void configureDirectionBox();
        bool eventFilter(QObject *obj, QEvent *event);
        void dragEnterEvent ( QDragEnterEvent * event );
        void dropEvent(QDropEvent *event);
        bool firstView;
        QNetworkAccessManager *manager;
        Ui::TransformWidget *ui;
        TransformAbstract * currentTransform;
        TransformMgmt *transformFactory;
        InfoDialog * infoDialog;
        GuiHelper * guiHelper;
        LoggerWidget *logger;
        QByteArray outputData;
        ByteSourceAbstract *byteSource;
        HexView *hexView;
        TextView *textView;
        QWidget *settingsTab;
        OffsetGotoWidget *gotoWidget;
        SearchWidget *searchWidget;
        ClearAllMarkingsButton * clearAllMarkingsButton;
        MessagePanelWidget* messagePanel;
        bool folded;

        friend class TransformWidgetStateObj;
};

class TransformWidgetStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit TransformWidgetStateObj(TransformWidget *tw);
        ~TransformWidgetStateObj();
        void run();

    private:
        TransformWidget *tw;
};

class TransformWidgetFoldingObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit TransformWidgetFoldingObj(TransformWidget *tw);
        ~TransformWidgetFoldingObj();
        void run();
    private:
        TransformWidget *tw;
};

#endif // TRANSFORMWIDGET_H

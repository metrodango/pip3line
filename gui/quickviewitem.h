/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef QUICKVIEWITEM_H
#define QUICKVIEWITEM_H

#include <QWidget>
#include <transformabstract.h>
#include <commonstrings.h>
#include <QTextEdit>
#include <QLineEdit>
#include "guihelper.h"
#include "quickviewitemconfig.h"

using namespace Pip3lineConst;

namespace Ui {
class QuickViewItem;
}

class TransformRequest;

class QuickViewItem : public QWidget
{
        Q_OBJECT
        
    public:
        explicit QuickViewItem(GuiHelper * guiHelper, QWidget *parent = nullptr, const QString &guiConfig = QString());
        ~QuickViewItem();
        bool configure();
        QString getXmlConf();
        void processData(const QByteArray &data);
        bool isConfigured();
    private slots:
        void processingFinished(QByteArray output, Messages messages);
        void mouseDoubleClickEvent (QMouseEvent *event);
        void onCopy();
    signals:
        void askForDeletion(QuickViewItem *);
        void sendRequest(TransformRequest *);
    private:
        Q_DISABLE_COPY(QuickViewItem)
        bool setXmlConf(const QString &conf);
        static const QString LOGID;
        Ui::QuickViewItem *ui;
        QuickViewItemConfig * guiConfig;
        GuiHelper * guiHelper;
        TransformAbstract *currentTransform;
        QString toolTipMess;
        bool noError;
        QByteArray currentData;
        OutputFormat format;
        OutputType outputType;
        QLineEdit *outputLine;
        QTextEdit *outputBlock;
};

#endif // QUICKVIEWITEM_H

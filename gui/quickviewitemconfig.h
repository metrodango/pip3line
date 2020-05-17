/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef QUICKVIEWITEMCONFIG_H
#define QUICKVIEWITEMCONFIG_H

#include <QDialog>
#include <transformabstract.h>
#include <transformmgmt.h>
#include <QWidget>
#include <commonstrings.h>
#include "guihelper.h"
#include "infodialog.h"

using namespace Pip3lineConst;

namespace Ui {
    class QuickViewItemConfig;
    class TransformDisplayConf;
}

class QuickViewItemConfig : public QDialog
{
        Q_OBJECT
        
    public:
        explicit QuickViewItemConfig(GuiHelper *guiHelper, QWidget *parent = nullptr);
        ~QuickViewItemConfig();
        TransformAbstract * getTransform();
        void setTransform(TransformAbstract * transform);
        void setName(const QString &name);
        QString getName() const;
        OutputFormat getFormat() const;
        void setFormat(OutputFormat format);
        void setOutputType(OutputType type);
        void setReadonly(bool value);
        bool getReadonly() const;
        OutputType getOutputType();
        void setWayBoxVisible(bool val);
        void setFormatVisible(bool val);
        void setOutputTypeVisible(bool val);
        void closeEvent(QCloseEvent *event);
        void setReadonlyVisible(bool value);
    private slots:
        void onTransformSelect(QString name);
        void onInboundWayChange(bool checked);
        void onTextFormatToggled(bool checked);
        void onInfo();
        void integrateTransform();
        void onTransformDelete();
    private:
        Q_DISABLE_COPY(QuickViewItemConfig)
        static const QString LOGID;
        Ui::QuickViewItemConfig *ui;
        Ui::TransformDisplayConf *uiTransform;
        GuiHelper *guiHelper;
        TransformAbstract *currentTransform;
        TransformMgmt * transformFactory;
        QWidget *confGui;
        InfoDialog * infoDialog;
        OutputFormat format;
        bool wayBoxVisible;
        bool formatBoxVisible;
        bool outputTypeVisible;
        bool readonlyVisible;
};

#endif // QUICKVIEWITEMCONFIG_H

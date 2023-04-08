/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef TRANSFORMSGUI_H
#define TRANSFORMSGUI_H

#include <QSpacerItem>
#include <QWidget>
#include <transformmgmt.h>
#include "transformchain.h"
#include "tababstract.h"

namespace Ui {
class TransformsGui;
}

class MassProcessingDialog;
class TransformWidget;
class TransformWidget;
class ByteSourceAbstract;
class DetachTabButton;
class ByteTableView;
class UniversalReceiverButton;

class TransformsGui : public TabAbstract
{
        Q_OBJECT

    public:
        explicit TransformsGui(GuiHelper *guiHelper , QWidget *parent = nullptr);
        ~TransformsGui();
        QString getCurrentChainConf();
        void setCurrentChainConf(const QString &conf, bool ignoreErrors = false);
        TransformChain getCurrentTransformChain();
        void setData(const QByteArray &data);
        void loadFromFile(QString fileName);
        int getBlockCount() const;
        ByteSourceAbstract *getSource(int blockIndex);
        ByteTableView *getHexTableView(int blockIndex);
        BaseStateAbstract *getStateMngtObj();
        bool isTrackingChanges();
        void setTrackChanges(bool enable);
    signals:
        void chainChanged(QString newConf);

    private slots:
        void processNewTransformation();
        void processDeletionRequest();
        void onMassProcessing();
        void onSaveState();
        void onLoadState();
        void onRegisterChain();
        void onSavedSelected(const QString &name);
        void buildSavedCombo();
        void onTransformChanged();
        void onNameChangeRequest(QString name);
        void resetAll();
        void onAutoCopychanged(bool val);
        void onFoldRequest();
        void onUnfoldRequest();
        void onInsertRequest();
        void onTrackChangesToggled(bool enable);
    private:
        Q_DISABLE_COPY(TransformsGui)
        void setCurrentTransformChain(TransformChain list);
        void addWidget(TransformWidget * transformWidget);
        void baseTransformWidgetconfiguration(TransformWidget * transformWidget);
        Ui::TransformsGui *ui;
        MassProcessingDialog * massProcessingDialog;
        TransformMgmt *transformFactory;
        TransformWidget *firstTransformWidget;
        QList<TransformWidget *> transformWidgetList;
        DetachTabButton *detachButton;
        QSpacerItem *spacer;
        UniversalReceiverButton *urb;
        bool spacerIsUsed;

        friend class TransformGuiStateObj;
};

class TransformGuiStateObj : public TabStateObj
{
        Q_OBJECT
    public:
        explicit TransformGuiStateObj(TransformsGui *tg);
        ~TransformGuiStateObj();
        void run();
    private:
        Q_DISABLE_COPY(TransformGuiStateObj)
};

#endif // TRANSFORMSGUI_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "transformsgui.h"
#include "ui_transformsgui.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "sources/bytesourceabstract.h"
#include "textinputdialog.h"
#include "loggerwidget.h"
#include "guihelper.h"
#include "massprocessingdialog.h"
#include "transformwidget.h"
#include "shared/detachtabbutton.h"
#include "shared/universalreceiverbutton.h"
#include "state/closingstate.h"
#include "views/foldedview.h"
#include "shared/sendtobutton.h"

TransformsGui::TransformsGui(GuiHelper *nguiHelper, QWidget *parent) :
    TabAbstract(nguiHelper,parent)
{
    spacerIsUsed = false;
    ui = new(std::nothrow) Ui::TransformsGui();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::TransformsGui X{");
    }

    transformFactory = guiHelper->getTransformFactory();
    ui->setupUi(this);

    spacer = new(std::nothrow) QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    if (spacer == nullptr) {
        qFatal("Cannot allocate memory for QSpacerItem X{");
    }

    firstTransformWidget = nullptr;
    massProcessingDialog = nullptr;

    // need to be before the transformwidget initialization
    ui->autoCopyLastPushButton->setChecked(guiHelper->isAutoCopyTextTransformGui());

    firstTransformWidget = new(std::nothrow) TransformWidget(guiHelper, this);

    if (firstTransformWidget == nullptr) {
        qFatal("Cannot allocate memory for firstTransformWidget X{");
    }

    addWidget(firstTransformWidget);

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == nullptr) {
        qFatal("Cannot allocate memory for detachButton X{");
    }

    ui->toolbarLayout->insertWidget(ui->toolbarLayout->indexOf(ui->massProcessingPushButton) + 1,detachButton);

    SendToButton * sendToButton = new(std::nothrow) SendToButton(guiHelper, this);
    if (sendToButton == nullptr) {
        qFatal("Cannot allocate memory for SendToButton X{");
    }

    ui->toolbarLayout->insertWidget(ui->toolbarLayout->indexOf(ui->registerPushButton) - 1,sendToButton);

    urb = new(std::nothrow) UniversalReceiverButton(this, guiHelper);
    if (urb == nullptr) {
        qFatal("Cannot allocate memory for UniversalReceiverButton X{");
    }

    ui->toolbarLayout->insertWidget(ui->toolbarLayout->indexOf(ui->registerPushButton),urb);

    ui->savedComboBox->installEventFilter(guiHelper);
    buildSavedCombo();

    //connect(ui->savedComboBox, qOverload<const QString &>(&QComboBox::currentIndexChanged), this, &TransformsGui::onSavedSelected);
    connect(ui->savedComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onSavedSelected(QString)));
    connect(ui->loadPushButton, &QPushButton::clicked, this, &TransformsGui::onLoadState);
    connect(ui->savePushButton, &QPushButton::clicked, this, &TransformsGui::onSaveState);
    connect(ui->resetPushButton, &QPushButton::clicked, this, &TransformsGui::resetAll);
    connect(ui->registerPushButton, &QPushButton::clicked, this, &TransformsGui::onRegisterChain);
    connect(ui->massProcessingPushButton, &QPushButton::clicked, this, &TransformsGui::onMassProcessing);
    connect(transformFactory, &TransformMgmt::savedUpdated, this, &TransformsGui::buildSavedCombo);
    connect(ui->autoCopyLastPushButton, &QPushButton::toggled, this, &TransformsGui::onAutoCopychanged);
    connect(ui->trackChangesCheckBox, &QCheckBox::toggled, this , &TransformsGui::onTrackChangesToggled);
}

TransformsGui::~TransformsGui()
{
 //   qDebug() << "Destroying " << this;
    delete massProcessingDialog;
    delete urb;
    delete spacer;
    while (transformWidgetList.size() > 0) {
        delete transformWidgetList.takeLast();
    }
    //delete spacer;
    delete detachButton;
    logger = nullptr;
    guiHelper = nullptr;
    delete ui;
}

QString TransformsGui::getCurrentChainConf()
{
//    this->setEnabled(false);

    TransformChain list = getCurrentTransformChain();
    QString ret;
    QXmlStreamWriter streamin(&ret);

    transformFactory->saveConfToXML(list, &streamin);

//    this->setEnabled(true);

    return ret;
}

void TransformsGui::setCurrentChainConf(const QString &conf, bool ignoreErrors)
{
    if (conf.isEmpty())
        return;

    MessageDialog errorDialog(guiHelper);
    connect(transformFactory, &TransformMgmt::error, &errorDialog, &MessageDialog::logError);
    QXmlStreamReader reader(conf);

    TransformChain talist = transformFactory->loadConfFromXML(&reader);
    if (talist.isEmpty()) {
        if (!ignoreErrors) {
            QMessageBox::critical(this,tr("Error"),tr("The loaded chain is empty. Check the logs."),QMessageBox::Ok);
        }
    }
    else {
        if (errorDialog.hasMessages() && !ignoreErrors) {
            errorDialog.setWindowTitle(tr("Error(s) while loading the configuration"));
            if (errorDialog.exec() == QDialog::Rejected) {
                while (!talist.isEmpty())
                    delete talist.takeLast();
                return;
            }
        }

        setCurrentTransformChain(talist);
        emit chainChanged(conf);
    }

}

TransformChain TransformsGui::getCurrentTransformChain()
{
   // this->setEnabled(false);

    TransformChain list;
    for (int i = 0; i < transformWidgetList.size() - 1; i++) {
        if (transformWidgetList.at(i)->getTransform() != nullptr )
            list.append(transformWidgetList.at(i)->getTransform());
    }
    list.setName(name);

 //   this->setEnabled(true);
    return list;
}


void TransformsGui::setCurrentTransformChain(TransformChain talist)
{
    name = talist.getName();
    emit nameChanged();
    // At this point nothing should go wrong, we can clean.

    while (transformWidgetList.size() > 0) {
        delete transformWidgetList.takeLast();
    }

    QList<TransformWidget *> widgetList;
    TransformWidget *twa = nullptr;
    for (int i = 0; i < talist.size(); i++) {
        twa = new(std::nothrow) TransformWidget(guiHelper, this);
        if (twa == nullptr) {
            qFatal("Cannot allocate memory for TransformWidget X{");
        } else {
            twa->setTransform(talist.at(i));
            widgetList.append(twa);
        }
    }
    twa = new(std::nothrow) TransformWidget(guiHelper, this);
    if (twa == nullptr) {
        qFatal("Cannot allocate memory for TransformWidget X{");
    }

    widgetList.append(twa);

    // setting the first transform widget
    firstTransformWidget = widgetList.at(0);

    // adding the rest
    for (int i = 0; i < widgetList.size(); i++) {
        addWidget(widgetList.at(i));
    }
}

void TransformsGui::setData(const QByteArray &data)
{
    if (ui->appendCheckBox->isChecked()) {
        QByteArray odata = firstTransformWidget->getSource()->getRawData();
        odata.append(data);
        firstTransformWidget->input(odata);
    } else {
        firstTransformWidget->input(data);
    }
}

void TransformsGui::loadFromFile(QString fileName)
{
    if (transformWidgetList.size() > 0) {
        TransformWidget * tw = transformWidgetList.at(0);
        tw->fromLocalFile(fileName);
    }
}

int TransformsGui::getBlockCount() const
{
    return transformWidgetList.size();
}

ByteSourceAbstract *TransformsGui::getSource(int blockIndex)
{
    if (blockIndex < 0 || blockIndex >= transformWidgetList.size()) {
        logger->logError(tr("[TransformsGui::getSource] index out-of-bounds: %1").arg(blockIndex));
        return nullptr;
    }

    return transformWidgetList.at(blockIndex)->getSource();
}

ByteTableView *TransformsGui::getHexTableView(int blockIndex)
{
    if (blockIndex < 0 || blockIndex >= transformWidgetList.size()) {
        logger->logError(tr("[TransformsGui::getHexTableView] index out-of-bounds %1").arg(blockIndex));
        return nullptr;
    }

    return transformWidgetList.at(blockIndex)->getHexTableView();
}

BaseStateAbstract *TransformsGui::getStateMngtObj()
{
    TransformGuiStateObj *stateObj = new(std::nothrow) TransformGuiStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for TransformGuiStateObj X{");
    }
    return stateObj;
}

bool TransformsGui::isTrackingChanges()
{
    return ui->trackChangesCheckBox->isChecked();
}

void TransformsGui::setTrackChanges(bool enable)
{
    ui->trackChangesCheckBox->setChecked(enable);
}

void TransformsGui::processNewTransformation()
{
    QObject *s = sender();

    if (s != nullptr) {
        TransformWidget *transformWidget = static_cast<TransformWidget *>(s);
        int pos;
        pos = transformWidgetList.indexOf(transformWidget);
        if (pos < 0) {
            qCritical() << tr("[TransformsGui::processNewTransformation] Cannot find the transform widget T_T");
            return;
        }

        if (transformWidgetList.last() == transformWidget) { // this is the last one, creating a new widget
            TransformWidget * ntw = new(std::nothrow) TransformWidget(guiHelper, this);
            if (ntw == nullptr) {
                qFatal("Cannot allocate memory for TransformWidget ntw X{");
            }
            ntw->getSource()->setTrackChanges(ui->trackChangesCheckBox->isChecked());
            addWidget(ntw);
        }
    } else {
        qCritical() << tr("[TransformsGui::processNewTransformation] sender is NULL T_T");
    }

}

void TransformsGui::processDeletionRequest()
{
    QObject *s = sender();

    if (s != nullptr) {
        TransformWidget *transformWidget = static_cast<TransformWidget *>(s);
        if (transformWidgetList.size() < 2) {
            firstTransformWidget->reset();
            return;
        }

        int i = transformWidgetList.indexOf(transformWidget);
        if (i == -1) {
            qCritical() << tr("[TransformsGui::processDeletionRequest] Widget object not found T_T");
            return;
        }

        if (i == 0) {
            firstTransformWidget = transformWidgetList.at(i + 1);
            transformWidgetList.takeAt(i);
            delete transformWidget;
        } else if (i < transformWidgetList.size() - 1) {
            TransformWidget *prev = transformWidgetList.at(i - 1);
            TransformWidget *suiv = transformWidgetList.at(i + 1);
            transformWidgetList.takeAt(i);
            delete transformWidget;
            connect(prev, &TransformWidget::updated, suiv, &TransformWidget::updatingFrom);
            prev->forceUpdating();
        }

        emit entriesChanged();
        emit chainChanged(getCurrentChainConf());
    } else {
        qCritical() << tr("[TransformsGui::processDeletionRequest] sender is NULL T_T");
    }
}

void TransformsGui::onMassProcessing()
{
    if (transformWidgetList.size() == 1) {
        QMessageBox::critical(this,tr("Nothing to be processed"), tr("Please, select a transformation before."),QMessageBox::Ok);
        return;
    }
    if (massProcessingDialog == nullptr) {
        massProcessingDialog = new(std::nothrow) MassProcessingDialog(guiHelper, this);
        if (massProcessingDialog == nullptr) {
            qFatal("Cannot allocate memory for MassProcessingDialog X{");
        }
        massProcessingDialog->setTranformChain(getCurrentChainConf());
        massProcessingDialog->setWindowTitle(tr("Mass processing for %1").arg(name));
    }

    massProcessingDialog->show();
}

void TransformsGui::addWidget(TransformWidget *transformWidget)
{
    if (!transformWidgetList.isEmpty()) { // only if there is already another TransformWidget
        TransformWidget *previousTw = transformWidgetList.last();
        previousTw->setAutoCopyTextToClipboard(false);
        connect(previousTw, &TransformWidget::updated,transformWidget, &TransformWidget::updatingFrom);
    }

    transformWidget->setAutoCopyTextToClipboard(ui->autoCopyLastPushButton->isChecked());
    transformWidget->getSource()->setTrackChanges(ui->trackChangesCheckBox->isChecked());
    transformWidgetList.append(transformWidget); // updating the list

    // Adding the widget to the gui
    ui->scrollAreaWidgetContents->layout()->addWidget(transformWidget);
    baseTransformWidgetconfiguration(transformWidget);

    emit entriesChanged();
}

void TransformsGui::baseTransformWidgetconfiguration(TransformWidget *transformWidget)
{
    connect(transformWidget, &TransformWidget::error,logger, &LoggerWidget::logError);
    connect(transformWidget, &TransformWidget::warning,logger, &LoggerWidget::logWarning);
    connect(transformWidget, &TransformWidget::status,logger, &LoggerWidget::logStatus);
    connect(transformWidget, &TransformWidget::transfoRequest,this,&TransformsGui::processNewTransformation);
    connect(transformWidget, &TransformWidget::deletionRequest, this, &TransformsGui::processDeletionRequest);
    connect(transformWidget, &TransformWidget::transformChanged, this, &TransformsGui::onTransformChanged,Qt::QueuedConnection);
    connect(transformWidget, &TransformWidget::tryNewName, this, &TransformsGui::onNameChangeRequest);
    connect(transformWidget, &TransformWidget::foldRequest, this, &TransformsGui::onFoldRequest);
    connect(transformWidget, &TransformWidget::insertRequest, this, &TransformsGui::onInsertRequest);
}

void TransformsGui::onSaveState()
{
    if (transformWidgetList.size() == 1) {
        QMessageBox::critical(this,tr("Error"),tr("No transformation selected, nothing to save!"),QMessageBox::Ok);
        return;
    }

    MessageDialog errorDialog(guiHelper);
    errorDialog.setJustShowMessages(true);
    connect(transformFactory, &TransformMgmt::error, &errorDialog, &MessageDialog::logError);

    TransformChain transformList = getCurrentTransformChain();

    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose file to save"), GuiConst::GLOBAL_LAST_PATH,tr("XML documents (*.xml);; All (*)"));

    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();

    transformFactory->saveConfToFile(fileName,transformList);

    if (errorDialog.hasMessages()) {
        errorDialog.setWindowTitle(tr("Error(s) while saving the state file:"));
        errorDialog.exec();
    } else {
        logger->logStatus(tr("Saved transformation chain to %1").arg(fileName));
    }
}

void TransformsGui::onLoadState()
{
    MessageDialog errorDialog(guiHelper);
    connect(transformFactory, &TransformMgmt::error, &errorDialog, &MessageDialog::logError);

    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose state file to load from"),GuiConst::GLOBAL_LAST_PATH,tr("XML documents (*.xml);; All (*)"));

    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();

    TransformChain talist = transformFactory->loadConfFromFile(fileName);
    if (talist.isEmpty())
        QMessageBox::critical(this,tr("Error"),tr("The loaded chain is empty. Check the logs."),QMessageBox::Ok);
    else {
        if (errorDialog.hasMessages()) {
            errorDialog.setWindowTitle(tr("Error(s) while loading the state file"));
            if (errorDialog.exec() == QDialog::Rejected) {
                while (!talist.isEmpty())
                    delete talist.takeLast();
                return;
            }
        } else {
            logger->logStatus(tr("File %1 loaded").arg(fileName));
        }

        setCurrentTransformChain(talist);
        // not really efficient
        QString conf;
        QXmlStreamWriter writer(&conf);
        transformFactory->saveConfToXML(talist, &writer);
        emit chainChanged(conf);
    }
}

void TransformsGui::onRegisterChain()
{
    if (transformWidgetList.size() == 1) {
        QMessageBox::critical(this,tr("Error"),tr("No transformation selected, nothing to register!"),QMessageBox::Ok);
    } else {

        TextInputDialog * dia = guiHelper->getNameDialog(this, name);
        if (dia != nullptr) {
            int ret = dia->exec();
            if (ret == QDialog::Accepted) {
                QString newName = dia->getInputText();
                if (newName.isEmpty())
                    newName = name;
                setName(newName);

                transformFactory->registerChainConf(getCurrentTransformChain(),true);

            }
            delete dia;
        }
    }
}

void TransformsGui::onSavedSelected(const QString &chainName)
{
    if (chainName.isEmpty()) {
        QMessageBox::critical(this,tr("Error"),tr("The selected name is empty T_T"),QMessageBox::Ok);
        qWarning() << tr("[TransformsGui] The selected name is empty T_T");
        return;
    }
    TransformChain tc = transformFactory->loadChainFromSaved(chainName);
    if (tc.isEmpty()) {
        QMessageBox::critical(this,tr("Error"),tr("The returned chain is empty. Check the logs."),QMessageBox::Ok);
    } else {
        resetAll();
        setCurrentTransformChain(tc);
        emit chainChanged(transformFactory->getSavedConfs().value(chainName));
    }
}

void TransformsGui::buildSavedCombo()
{
    ui->savedComboBox->blockSignals(true);
    ui->savedComboBox->clear();
    int row = 0;
    // first inactive element
    ui->savedComboBox->addItem(QString("User's chains"));
    QStandardItem * item = qobject_cast<QStandardItemModel *>(ui->savedComboBox->model())->item( row );
    item->setEnabled( false );
    item->setTextAlignment(Qt::AlignCenter);
    item->setBackground(Qt::darkGray);
    item->setForeground(Qt::white);
    // then the rest
    QHash<QString, QString> hash = transformFactory->getSavedConfs();
    QStringList list = hash.keys();
    if (list.isEmpty()) {
        ui->savedComboBox->setEnabled(false);
    } else {
        ui->savedComboBox->addItems(list);
        ui->savedComboBox->setEnabled(true);
    }

    ui->savedComboBox->blockSignals(false);
}

void TransformsGui::resetAll()
{
    while (transformWidgetList.size() > 1) { // we want to keep the first one
        delete transformWidgetList.takeLast();
    }

    // just reset first one
    firstTransformWidget->reset();

    ui->savedComboBox->blockSignals(true);
    ui->savedComboBox->setCurrentIndex(0);
    ui->savedComboBox->blockSignals(false);
}

void TransformsGui::onAutoCopychanged(bool val)
{
    if (!transformWidgetList.isEmpty()) { // only if there is a TransformWidget
        TransformWidget *last = transformWidgetList.last();
        last->setAutoCopyTextToClipboard(val);
    } else {
        qCritical() << tr("No TransformWidget in the list T_T");
    }
}

void TransformsGui::onFoldRequest()
{
    QObject *s = sender();
    if (s != nullptr) {
        TransformWidget * requester = static_cast<TransformWidget *>(s);

        int index = ui->mainLayout->indexOf(requester);
        if (index == -1) {
            logger->logError(tr("Invalid index when folding T_T"), metaObject()->className());
        } else {
            ui->mainLayout->removeWidget(requester);
            requester->hide();
            requester->setFolded(true);
            FoldedView *fv = new(std::nothrow) FoldedView(requester,this);
            if (fv == nullptr) {
                qFatal("Cannot allocate memory for FoldedWidget X{");
            }

            fv->enableDelete(transformWidgetList.last() != requester);

            connect(fv, &FoldedView::unfoldRequested, this, &TransformsGui::onUnfoldRequest);

            ui->mainLayout->insertWidget(index,fv,0, Qt::AlignTop);
            bool allFolded = true;
            for (int i = 0; i < transformWidgetList.size(); i++) {
                allFolded = allFolded && transformWidgetList.at(i)->isFolded();
            }

            if (allFolded && !spacerIsUsed) {
                ui->mainLayout->addSpacerItem(spacer);
                spacerIsUsed = true;
            }
        }
    } else {
        qCritical() << tr("[TransformsGui::onFoldRequest] sender is NULL T_T");
    }
}

void TransformsGui::onUnfoldRequest()
{
    QObject *s = sender();

    if (s != nullptr) {
        FoldedView *fv = static_cast<FoldedView *>(s);
        int index = ui->mainLayout->indexOf(fv);
        if (index == -1) {
            logger->logError(tr("Invalid index when unfolding T_T"), metaObject()->className());
        } else {
            TransformWidget * tw = fv->getTransformWidget();
            ui->mainLayout->removeWidget(fv);
            delete fv;
            ui->mainLayout->removeItem(spacer);
            spacerIsUsed = false;
            ui->mainLayout->insertWidget(index,tw);
            tw->setFolded(false);
            tw->show();
        }
    } else {
        qCritical() << tr("[TransformsGui::onUnfoldRequest] sender is NULL T_T");
    }
}

void TransformsGui::onInsertRequest()
{
    QObject *s = sender();

    if (s != nullptr) {
        TransformWidget * requester = static_cast<TransformWidget *>(s);
        int index = transformWidgetList.indexOf(requester);
        if (index == -1) {
            qCritical() << tr("[TransformsGui::onInsertRequest] Invalid index when inserting T_T");
        } else {
            TransformWidget * newtw = new(std::nothrow) TransformWidget(guiHelper, this);

            if (newtw == nullptr) {
                qFatal("Cannot allocate memory for newtw X{");
            }

            newtw->enableDeletetion(true);

            transformWidgetList.insert(index,newtw); // updating the list

            // Adding the widget to the gui
            QVBoxLayout * layout = dynamic_cast<QVBoxLayout *>(ui->scrollAreaWidgetContents->layout());
            if (layout != nullptr)
                layout->insertWidget(index, newtw);
            else
                qCritical() << tr("[TransformsGui::onInsertRequest] null pointer cast for layout T_T");

            baseTransformWidgetconfiguration(newtw);

            connect(newtw, &TransformWidget::updated,requester, &TransformWidget::updatingFrom); // connect the new one to the requester

            if (index == 0) {
                firstTransformWidget = newtw;
            } else if (index < transformWidgetList.size() - 1) {
                TransformWidget *prev = transformWidgetList.at(index - 1);

                disconnect(prev, &TransformWidget::updated, requester, &TransformWidget::updatingFrom); // disconnect the link between the previous and the requester

                connect(prev, &TransformWidget::updated, newtw, &TransformWidget::updatingFrom);

                prev->forceUpdating();
            }

            emit entriesChanged();
            emit chainChanged(getCurrentChainConf());
        }
    } else {
        qCritical() << tr("[TransformsGui::onInsertRequest] sender is NULL T_T");
    }
}

void TransformsGui::onTrackChangesToggled(bool enable)
{
    for (int i = 0; i < transformWidgetList.size(); i++) {
        transformWidgetList.at(i)->getSource()->setTrackChanges(enable);
    }
}

void TransformsGui::onTransformChanged()
{
    emit chainChanged(getCurrentChainConf());
}

void TransformsGui::onNameChangeRequest(QString chainName)
{
    QObject * obj = sender();

    if (obj == firstTransformWidget  && !chainName.isEmpty()) {
        setName(chainName);
    }
}



TransformGuiStateObj::TransformGuiStateObj(TransformsGui *tg) :
    TabStateObj(tg)
{
    name = tg->metaObject()->className();
}

TransformGuiStateObj::~TransformGuiStateObj()
{

}

void TransformGuiStateObj::run()
{
    TabStateObj::run();
    QString conf;
    int size = 0;

    TransformsGui *tgtab = dynamic_cast<TransformsGui *>(tab);

    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        conf = tgtab->getCurrentChainConf();
        writer->writeAttribute(GuiConst::STATE_TRACK_CHANGES, write(tgtab->isTrackingChanges()));
        writer->writeAttribute(GuiConst::STATE_TRANSFORM_CONF, write(conf,true));
        size = tgtab->getBlockCount();
        int finalSize = size;

        QList<BaseStateAbstract *> sourcesState;
        TransformWidget * tw = nullptr;
        // first saving the transformWidget states so that they are saved last.
        for (int i = size - 1  ; i > -1; i--) { // need to reverse the order due to the stack behaviour
            tw = tgtab->transformWidgetList.at(i);
            // if there is no Transform configured, removing the tab (it does not do anything anyway)
            // unless this is the last one
            if (tw->getTransform() != nullptr || i == size - 1) {
                BaseStateAbstract *state = tw->getStateMngtObj();
                sourcesState.append(tgtab->getSource(i)->getStateMngtObj());
                emit addNewState(state);
            } else {
                // and reducing the actual size of the list
                finalSize--;
            }
        }

        // now pushing the ByteSources states
        writer->writeAttribute(GuiConst::STATE_SIZE, write(finalSize));
        for (int i = 0; i < sourcesState.size(); i++) {
            emit addNewState(sourcesState.at(i));
        }
        sourcesState.clear(); // not really useful, but one never now ...

    } else {
        QXmlStreamAttributes attrList = reader->attributes();
        conf = readString(attrList.value(GuiConst::STATE_TRANSFORM_CONF));

        if (attributes.hasAttribute(GuiConst::STATE_TRACK_CHANGES)) {
            QString val = attributes.value(GuiConst::STATE_TRACK_CHANGES).toString();
            if (val == GuiConst::STATE_YES) {
                tgtab->setTrackChanges(true);
            } else if (val == GuiConst::STATE_NO) {
                tgtab->setTrackChanges(false);
            }
        }

        if (!conf.isEmpty()) {
            tgtab->setCurrentChainConf(conf, true);
        }
        int bsize = tgtab->getBlockCount();
        bool ok = false;
        size = readInt(attrList.value(GuiConst::STATE_SIZE),&ok);
        if (!ok) {
            emit log(tr("Error while parsing the number of blocks from the saved state, stopping restore."), tr("TransformGuiLoader"), Pip3lineConst::LERROR);
            return;
        }

        if (bsize != size) {
            emit log(tr("The number of transform blocks (%1) is different from the array size (%2). Taking the smaller number.").arg(bsize).arg(size), tr("TransformGuiLoader"), Pip3lineConst::LWARNING);
            size = qMin(bsize, size);
        }

        // first restoring the transformWidget states (so that they are executed last)
        for (int i = size - 1  ; i > -1; i--) { // need to reverse the order due to the stack behaviour
            BaseStateAbstract *state = tgtab->transformWidgetList.at(i)->getStateMngtObj();
            emit addNewState(state);
        }

        // then the data (executed first)
        for (int i = size - 1  ; i > -1; i--) { // need to reverse the order due to the stack behaviour
            BaseStateAbstract *tempState = tgtab->getSource(i)->getStateMngtObj();
            emit addNewState(tempState);
        }
    }
}

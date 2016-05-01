/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <transformfactoryplugininterface.h>
#include "pluginconfwidget.h"
#include <QHashIterator>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QListWidgetItem>
#include <commonstrings.h>
#include <deleteablelistitem.h>
#include "quickviewitemconfig.h"
#include <transformmgmt.h>
#include <QListWidgetItem>
#include <QFileDialog>
#include "guihelper.h"
#include <QSettings>
#include <QStandardItemModel>
#include <QSslConfiguration>
#include <QDebug>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <tabs/tababstract.h>
#include <QFontDialog>
#include "shared/defaultcontrolgui.h"
#include "loggerwidget.h"
#include "shared/guiconst.h"
#include "sources/blocksources/blockssource.h"

using namespace GuiConst;

const QString SettingsDialog::LOGID = "SettingsDialog";

SettingsDialog::SettingsDialog(GuiHelper *nhelper, QWidget *parent) :
    AppDialog(nhelper, parent)
{
    ui = new(std::nothrow) Ui::SettingsDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::SettingsDialog X{");
    }
    setModal(false);

    tManager = guiHelper->getTransformFactory();
    settings = tManager->getSettingsObj();
    ui->setupUi(this);

    ui->timerSaveSpinBox->setMinimum(GuiConst::MIN_AUTO_SAVE_TIMER_INTERVAL);
    ui->timerSaveSpinBox->setMaximum(GuiConst::MAX_AUTO_SAVE_TIMER_INTERVAL);

    initializeConf();
    updatePluginList();
    updateDeletedTabsList();

    // getting the control gui for this blocksource
    BlocksSource * defaultBs = guiHelper->getIncomingBlockListener();
    QWidget * bConf = defaultBs->getGui(this);
    if (bConf != nullptr) {
        ui->defaultIncomingLayout->addWidget(bConf);
    }

    connect(ui->checkUpdatePushButton, SIGNAL(clicked()), this, SLOT(onUpdateRequest()));
    connect(tManager, SIGNAL(savedUpdated()), this, SLOT(updateRegisteredList()));
    connect(guiHelper, SIGNAL(markingsUpdated()), this, SLOT(updateSavedMarkingColors()));
    connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportFuncs()));
    connect(ui->resetMarkingsPushButton, SIGNAL(clicked()), this, SLOT(onResetMarkings()));
    connect(ui->resetIEFuncsPushButton, SIGNAL(clicked()), this, SLOT(onResetImportExportFuncs()));
    connect(ui->savedListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onSavedClicked(QListWidgetItem*)));
    connect(ui->pluginsListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onPluginClicked(QModelIndex)));
    connect(ui->importExportListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onDoubleClickImportExportFuncs(QListWidgetItem*)));
    connect(ui->addImportExportPushButton, SIGNAL(clicked()), this, SLOT(onAddImportExportFuncs()));
    connect(ui->slByteDataCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLoadSaveOptionsToggled(bool)));
    connect(ui->dataMarkingsCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLoadSaveOptionsToggled(bool)));
    connect(ui->dataHistCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLoadSaveOptionsToggled(bool)));
    connect(ui->quickViewCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLoadSaveOptionsToggled(bool)));
    connect(ui->comparisonCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLoadSaveOptionsToggled(bool)));
    connect(ui->guiPosCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLoadSaveOptionsToggled(bool)));
    connect(ui->globalConfCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLoadSaveOptionsToggled(bool)));
    connect(guiHelper, SIGNAL(deletedTabsUpdated()), this, SLOT(updateDeletedTabsList()));
    connect(ui->clearAllTabsPushButton, SIGNAL(clicked()), guiHelper, SLOT(clearDeletedTabs()));
    connect(ui->deletedTabsListWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDeletedTabsDoubleClicked(QModelIndex)));
    connect(ui->fileSavePushButton, SIGNAL(clicked()), this , SLOT(onAutoSaveFileButtonclicked()));
    connect(ui->customFontPushButton, SIGNAL(clicked(bool)), SLOT(onCustomFontClicked()));
    connect(ui->saveNowPushButton, SIGNAL(clicked(bool)), this, SLOT(onSaveNowClicked()));
    ui->titleListWidget->setCurrentRow(0);
}

SettingsDialog::~SettingsDialog()
{
    delete settings;
    delete ui;
}

void SettingsDialog::setVersionUpdateMessage(QString mess)
{
    ui->versionCheckedLabel->setText(mess);
}

void SettingsDialog::initializeConf()
{
    // preventing signals to loop back to guiHelper
    disconnectUpdateSignals();
    ui->portSpinBox->setValue(guiHelper->getDefaultPort());
    ui->decodeCheckBox->setChecked(guiHelper->getDefaultServerDecode());
    ui->encodeCheckBox->setChecked(guiHelper->getDefaultServerEncode());
    ui->pipeNameLineEdit->setText(guiHelper->getDefaultServerPipeName());
    ui->autoUpdateCheckBox->setChecked(settings->value(SETTINGS_AUTO_UPDATE, true).toBool());
    ui->minimizeCheckBox->setChecked(settings->value(SETTINGS_MINIMIZE_TO_TRAY, true).toBool());
    ui->hexWidget->setChar(guiHelper->getDefaultServerSeparator());
    ui->ignoreSSLErrCheckBox->setChecked(guiHelper->getIgnoreSSLErrors());
    ui->enableProxyCheckBox->setChecked(guiHelper->getEnableNetworkProxy());
    ui->proxyHostLineEdit->setEnabled(ui->enableProxyCheckBox->isChecked());
    ui->proxyPortSpinBox->setEnabled(ui->enableProxyCheckBox->isChecked());
    ui->proxyHostLineEdit->setText(guiHelper->getProxyInterface());
    ui->proxyPortSpinBox->setValue(guiHelper->getProxyPort());
    ui->autoTextCopyCheckBox->setChecked(guiHelper->isAutoCopyTextTransformGui());
    ui->autoRestoreCheckBox->setChecked(guiHelper->getAutoRestoreOnStartup());
    QFont font = guiHelper->getRegularFont();
    ui->customFontName->setFont(font);
    ui->customFontName->setText(font.family());

    int val = ui->offsetBaseComboBox->findText(QString::number(guiHelper->getDefaultOffsetBase()));
    if (val != -1) {
        ui->offsetBaseComboBox->setCurrentIndex(val);
    }

    quint64 stateflags = guiHelper->getDefaultSaveStateFlags();
    ui->slByteDataCheckBox->setChecked(stateflags & GuiConst::STATE_LOADSAVE_DATA);
    if (ui->slByteDataCheckBox->isChecked()) {
        ui->dataMarkingsCheckBox->setEnabled(true);
        ui->dataHistCheckBox->setEnabled(true);
    } else {
        ui->dataMarkingsCheckBox->setEnabled(false);
        ui->dataHistCheckBox->setEnabled(false);
    }
    ui->dataMarkingsCheckBox->setChecked(stateflags & GuiConst::STATE_LOADSAVE_MARKINGS);
    ui->dataHistCheckBox->setChecked(stateflags & GuiConst::STATE_LOADSAVE_HISTORY);
    ui->quickViewCheckBox->setChecked(stateflags & GuiConst::STATE_LOADSAVE_QUICKVIEW_CONF);
    ui->comparisonCheckBox->setChecked(stateflags & GuiConst::STATE_LOADSAVE_COMPARISON);
    ui->guiPosCheckBox->setChecked(stateflags & GuiConst::STATE_LOADSAVE_DIALOG_POS);

    updateRegisteredList();
    updateSavedMarkingColors();
    updateImportExportFuncs();
    updateFilter();

    ui->defaultTabComboBox->clear();
    ui->defaultTabComboBox->addItem(GuiConst::TRANSFORM_TAB_STRING,QVariant(GuiConst::TRANSFORM_PRETAB));
    ui->defaultTabComboBox->addItem(GuiConst::BASEHEX_TAB_STRING,QVariant(GuiConst::HEXAEDITOR_PRETAB));
    int index = ui->defaultTabComboBox->findData(guiHelper->getDefaultNewTab());
    if (index == -1) {
        qCritical() << tr("[SettingsDialog::initializeConf] cannot find the index for the default tab combobox T_T");
        index = 0;
    }
    ui->defaultTabComboBox->setCurrentIndex(index);

    ui->autoSaveGroupBox->setChecked(guiHelper->getAutoSaveState());
    ui->fileSaveLineEdit->setText(guiHelper->getAutoSaveFileName());

    ui->saveOnExitCheckBox->setChecked(guiHelper->getAutoSaveOnExit());
    ui->timerSaveCheckBox->setChecked(guiHelper->getAutoSaveTimerEnable());
    ui->timerSaveSpinBox->setValue(guiHelper->getAutoSaveTimerInterval());
    //once finished we reconnect everything
    connectUpdateSignals();
}

void SettingsDialog::autoUpdateChanged(bool checked)
{
    settings->setValue(SETTINGS_AUTO_UPDATE, checked);
}


void SettingsDialog::onMinimizeChanged(bool checked)
{
    settings->setValue(SETTINGS_MINIMIZE_TO_TRAY, checked);
}

void SettingsDialog::onUpdateRequest()
{
    emit updateCheckRequested();
}

void SettingsDialog::updatePluginList()
{
    ui->pluginsListWidget->clear();
    stackedList.clear();
    int count = ui->pluginsStackedWidget->count();
    for (int i = 0 ; i < count; i++) {
      QWidget * wid = ui->pluginsStackedWidget->widget(i);
      ui->pluginsStackedWidget->removeWidget(wid);
      delete wid;
    }

    QHash<QString, TransformFactoryPluginInterface *> pluginList = tManager->getPlugins();

    QHashIterator<QString, TransformFactoryPluginInterface *> i(pluginList);
    while (i.hasNext()) {
        i.next();
        PluginConfWidget *widget = new(std::nothrow) PluginConfWidget(i.value());
        if (widget != nullptr) {
            stackedList.insert(i.key(), ui->pluginsStackedWidget->addWidget(widget));
            ui->pluginsListWidget->addItem(i.key());
        } else {
            qFatal("Cannot allocate memory for PluginConfWidget X{");
        }
    }
    ui->pluginsListWidget->sortItems();
}

void SettingsDialog::updateRegisteredList()
{
    ui->savedListWidget->clear();
    QStringList list = tManager->getSavedConfs().keys();

    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i));
        if (itemWid != nullptr) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onDeleteSaved(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != nullptr) {
                ui->savedListWidget->addItem(item);
                ui->savedListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem registered X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem registered X{");
        }
    }
}

void SettingsDialog::updateSavedMarkingColors()
{
    ui->markingColorsListWidget->clear();
    QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
    QHashIterator<QString, QColor> i(colors);
    while (i.hasNext()) {
        i.next();
        QPixmap pix(20,20);
        pix.fill(i.value());
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(i.key(), pix);
        if (itemWid != nullptr) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onMarkingDelete(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != nullptr) {
                ui->markingColorsListWidget->addItem(item);
                ui->markingColorsListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem Colors X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem Colors X{");
        }
    }
}

void SettingsDialog::updateImportExportFuncs()
{
    ui->importExportListWidget->clear();
    QStringList list = guiHelper->getImportExportFunctions();
    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i));
        if (itemWid != nullptr) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onImportExportFuncDeletes(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != nullptr) {
                ui->importExportListWidget->addItem(item);
                ui->importExportListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem Import/Export X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem Import/Export X{");
        }
    }
}

void SettingsDialog::updateMisc()
{
    int offsetbase = guiHelper->getDefaultOffsetBase();
    switch (offsetbase) {
        case 8:
            ui->offsetBaseComboBox->setCurrentIndex(0);
            break;
        case 10:
            ui->offsetBaseComboBox->setCurrentIndex(1);
            break;
        case 16:
            ui->offsetBaseComboBox->setCurrentIndex(2);
            break;
        default: // this should obviously not happen ...
            qCritical() << "[SettingsDialog::updateMisc] offsetbase looks fishy"<< offsetbase << "T_T";
            guiHelper->setDefaultOffsetBase(16);
            ui->offsetBaseComboBox->setCurrentIndex(2);
    }
}

void SettingsDialog::updateFilter()
{
    QStringList typesList = tManager->getTypesList();
    QStandardItemModel *model = new(std::nothrow) QStandardItemModel(typesList.size(), 1);
    if (model == nullptr ) {
        qFatal("Cannot allocate memory for QStandardItemModel X{");
        return;
    }
    QSet<QString> typesBlacklist = guiHelper->getTypesBlacklist();


    for (int i = 0; i < typesList.size(); ++i)
    {
        QStandardItem* item = new(std::nothrow) QStandardItem(typesList.at(i));
        if (item != nullptr) {
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setData( (typesBlacklist.contains(typesList.at(i)) ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);
            model->setItem(i, 0, item);
        } else {
          qFatal("Cannot allocate memory for QStandardItem 2 X{");
        }
    }
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), guiHelper, SLOT(onFilterChanged(QModelIndex,QModelIndex)));
    ui->filterListView->setModel(model);
}

void SettingsDialog::updateDeletedTabsList()
{
    ui->deletedTabsListWidget->clear();
    QList<TabAbstract *> list = guiHelper->getDeletedTabs();
    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i)->getName());
        if (itemWid != nullptr) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onDeletedTabsDeleted(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != nullptr) {
                ui->deletedTabsListWidget->addItem(item);
                ui->deletedTabsListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem DeleteTabs settings X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem DeleteTabs settings X{");
        }
    }
}

void SettingsDialog::onDeletedTabsDeleted(const QString)
{
    DeleteableListItem *senderItemWid = dynamic_cast<DeleteableListItem *>(sender());
    if (senderItemWid == nullptr) {
        qFatal("Cannot cast QObject to  DeleteableListItem X{");
    }

    // find the corresponding item by iterating the list (sub-optimal I know)

    for (int i = 0; i < ui->deletedTabsListWidget->count(); i++) {
        QListWidgetItem *item = ui->deletedTabsListWidget->item(i);
        DeleteableListItem *itemWid = dynamic_cast<DeleteableListItem *>(ui->deletedTabsListWidget->itemWidget(item));
        if (itemWid == senderItemWid) {
            TabAbstract * tab = guiHelper->takeDeletedTab(i);
            delete tab;
            return; // stopping now
        }
    }
    // if we are here, something went wrong
    qCritical() << "could not find the item attached to the widget";
}

void SettingsDialog::onDeletedTabsDoubleClicked(QModelIndex index)
{

    int row = index.row();
    QListWidgetItem * item = ui->deletedTabsListWidget->item(row);
    DeleteableListItem *itemWid = (DeleteableListItem *)ui->deletedTabsListWidget->itemWidget(item);
    itemWid->deleteLater();

    guiHelper->reviveTab(row);
}

void SettingsDialog::onImportExportFuncDeletes(const QString &name)
{
    guiHelper->removeImportExportFunctions(name);
}

void SettingsDialog::onResetImportExportFuncs()
{
    guiHelper->resetImportExportFuncs();
}

void SettingsDialog::onDoubleClickImportExportFuncs(QListWidgetItem *item)
{
    DeleteableListItem *itemWid = dynamic_cast<DeleteableListItem *>(ui->importExportListWidget->itemWidget(item));

    QString name = itemWid->getName();

    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (itemConfig != nullptr) {
        itemConfig->setWayBoxVisible(false);
        itemConfig->setFormatVisible(false);
        TransformAbstract *ta = guiHelper->getImportExportFunction(name);

        if (ta != nullptr) {

            ta = tManager->cloneTransform(ta);
            if (ta != nullptr) {
                itemConfig->setTransform(ta);
                itemConfig->setName(name);
                int ret = itemConfig->exec();
                if (ret == QDialog::Accepted) {
                    delete ta;
                    ta = itemConfig->getTransform();
                    QString newName = itemConfig->getName();

                    guiHelper->removeImportExportFunctions(name);
                    guiHelper->addImportExportFunctions(newName, ta);
                } else {
                    delete ta;
                }

            }
        }
        delete itemConfig;
    } else {
        qFatal("Cannot allocate memory for QuickViewItemConfig Import/Export double X{");
    }
}

void SettingsDialog::onAddImportExportFuncs()
{
    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (itemConfig != nullptr) {
        itemConfig->setWayBoxVisible(false);
        itemConfig->setFormatVisible(false);
        itemConfig->setOutputTypeVisible(false);
        int ret = itemConfig->exec();
        if (ret == QDialog::Accepted) {
            TransformAbstract *ta = itemConfig->getTransform();
            if (ta != nullptr) {
                guiHelper->addImportExportFunctions(itemConfig->getName(),ta);
            }
        }

        delete itemConfig;
    } else {
        qFatal("Cannot allocate memory for QuickViewItemConfig Import/Export add X{");
    }
}

void SettingsDialog::onPluginClicked(QModelIndex index)
{
    QString name = index.data().toString();

    if (stackedList.contains(name)) {
        ui->pluginsStackedWidget->setCurrentIndex(stackedList.value(name));
    } else {
        qWarning() << tr("[SettingsDialog] Cannot find the item %1 in stackedList T_T").arg(name);
    }
}


void SettingsDialog::onSavedClicked(QListWidgetItem * item)
{
    DeleteableListItem *itemWid = (DeleteableListItem *)ui->savedListWidget->itemWidget(item);

    QString name = itemWid->getName();

    QHash<QString, QString> list = tManager->getSavedConfs();
    if (list.contains(name))
        ui->savedDescriptLabel->setText(tManager->getSavedConfs().value(name));
    else
        guiHelper->getLogger()->logError(tr("Saved conf \"%1\" not found").arg(name),LOGID);
}

void SettingsDialog::onDeleteSaved(const QString &name)
{
    tManager->unregisterChainConf(name);
    ui->savedDescriptLabel->clear();
}

void SettingsDialog::onMarkingDelete(const QString &name)
{
    guiHelper->removeMarkingColor(name);
}

void SettingsDialog::onResetMarkings()
{
    guiHelper->resetMarkings();
}

void SettingsDialog::onServerPortChanged(int port)
{
    guiHelper->setDefaultServerPort(port);
}

void SettingsDialog::onServerDecodeChanged(bool val)
{
    guiHelper->setDefaultServerDecode(val);
}

void SettingsDialog::onServerEncodeChanged(bool val)
{
    guiHelper->setDefaultServerEncode(val);
}

void SettingsDialog::onServerSeparatorChanged(char c)
{
    guiHelper->setDefaultServerSeparator(c);
}

void SettingsDialog::onServerPipeNameChanged(QString name)
{
    guiHelper->setDefaultServerPipeName(name);
}

void SettingsDialog::onOffsetBaseChanged(QString val)
{
    bool ok = false;
    int intval = val.toInt(&ok);

    if (!ok) {
        guiHelper->getLogger()->logError("Invalid integer value for offset base in settings T_T");
    } else {
        guiHelper->setDefaultOffsetBase(intval);
    }
}

void SettingsDialog::onIgnoreSSLErrChanged(bool ignore)
{
    guiHelper->setIgnoreSSLErrors(ignore);
}

void SettingsDialog::onProxyEnabledChanged(bool proxyEnable)
{
    guiHelper->setEnableNetworkProxy(proxyEnable);
    ui->proxyHostLineEdit->setEnabled(proxyEnable);
    ui->proxyPortSpinBox->setEnabled(proxyEnable);
}

void SettingsDialog::onProxyIPChanged(QString ipString)
{
    guiHelper->setProxyInterface(ipString);
}

void SettingsDialog::onProxyPortChanged(int port)
{
    guiHelper->setProxyPort((quint16)port);
}

void SettingsDialog::onDefaultTabChanged(int index)
{
    if (index >= 0 && index < GuiConst::AVAILABLE_TAB_STRINGS.size())
        guiHelper->setDefaultNewTab((GuiConst::AVAILABLE_PRETABS)ui->defaultTabComboBox->itemData(index).toInt()); // don't need to check if this is an actual int
    else if (index != -1) {
        qWarning() << "[SettingsDialog::onDefaultTabChanged] Invalid index value" << index;
    }
}

void SettingsDialog::onAutoTextCopyChanged(bool val)
{
    guiHelper->setAutoCopyTextTransformGui(val);
}

void SettingsDialog::onLoadSaveOptionsToggled(bool checked)
{
    QObject * o = sender();
    quint64 flags = guiHelper->getDefaultLoadStateFlags();

    if ( o == ui->slByteDataCheckBox ) {
        if (checked)
            flags |= GuiConst::STATE_LOADSAVE_DATA;
        else
            flags &= ~GuiConst::STATE_LOADSAVE_DATA;
    } else if ( o == ui->dataMarkingsCheckBox ) {
        if (checked)
            flags |= GuiConst::STATE_LOADSAVE_MARKINGS;
        else
            flags &= ~GuiConst::STATE_LOADSAVE_MARKINGS;
    } else if ( o == ui->dataHistCheckBox ) {
        if (checked)
            flags |= GuiConst::STATE_LOADSAVE_HISTORY;
        else
            flags &= ~GuiConst::STATE_LOADSAVE_HISTORY;
    } else if ( o == ui->quickViewCheckBox ) {
        if (checked)
            flags |= GuiConst::STATE_LOADSAVE_QUICKVIEW_CONF;
        else
            flags &= ~GuiConst::STATE_LOADSAVE_QUICKVIEW_CONF;
    } else if ( o == ui->comparisonCheckBox ) {
        if (checked)
            flags |= GuiConst::STATE_LOADSAVE_COMPARISON;
        else
            flags &= ~GuiConst::STATE_LOADSAVE_COMPARISON;
    } else if ( o == ui->guiPosCheckBox ) {
        if (checked)
            flags |= GuiConst::STATE_LOADSAVE_DIALOG_POS;
        else
            flags &= ~GuiConst::STATE_LOADSAVE_DIALOG_POS;
    } else if ( o == ui->globalConfCheckBox ) {
        if (checked)
            flags |= GuiConst::STATE_LOADSAVE_GLOBAL_CONF;
        else
            flags &= ~GuiConst::STATE_LOADSAVE_GLOBAL_CONF;
    }

    guiHelper->setDefaultStateFlags(flags);
}

void SettingsDialog::onAutoSaveToggled(bool checked)
{
    guiHelper->setAutoSaveState(checked);
}

void SettingsDialog::onAutoSaveFileNameChanged(QString name)
{
    guiHelper->setAutoSaveFileName(name);
}

void SettingsDialog::onAutoSaveFileButtonclicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose a file to save to"),
                                                    QDir::homePath().append(QDir::separator()).append(GuiConst::DEFAULT_STATE_FILE));
    if (!fileName.isEmpty()) {
        ui->fileSaveLineEdit->setText(fileName);
    }
}

void SettingsDialog::onAutoSaveOnExitToggled(bool checked)
{
    guiHelper->setAutoSaveOnExit(checked);
}

void SettingsDialog::onAutoSaveTimerEnableToggled(bool checked)
{
    guiHelper->setAutoSaveTimerEnable(checked);
}

void SettingsDialog::onAutoSaveTimerIntervalChanged(int value)
{
    guiHelper->setAutoSaveTimerInterval(value);
}

void SettingsDialog::onDataSaveToggled(bool checked)
{
    ui->dataMarkingsCheckBox->setEnabled(checked);
    ui->dataHistCheckBox->setEnabled(checked);
}

void SettingsDialog::onAutoRestoreToggled(bool checked)
{
    guiHelper->setAutoRestoreOnStartup(checked);
}

void SettingsDialog::onCustomFontClicked()
{
    bool ok;
    QFont selectedFont = QFontDialog::getFont(
                 &ok, guiHelper->getRegularFont(), this);
    if (ok) {
        guiHelper->setRegularFont(selectedFont);
        ui->customFontName->setText(selectedFont.family());
    }
}

void SettingsDialog::onSaveNowClicked()
{
    hide();
    emit forceAutoSave();
}

void SettingsDialog::connectUpdateSignals()
{
    connect(ui->autoUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoUpdateChanged(bool)));
    connect(ui->minimizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMinimizeChanged(bool)));
    connect(ui->defaultTabComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDefaultTabChanged(int)));
    connect(ui->decodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerDecodeChanged(bool)));
    connect(ui->encodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerEncodeChanged(bool)));
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onServerPortChanged(int)));
    connect(ui->pipeNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onServerPipeNameChanged(QString)));
    connect(ui->hexWidget, SIGNAL(charChanged(char)), this, SLOT(onServerSeparatorChanged(char)));
    connect(ui->offsetBaseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onOffsetBaseChanged(QString)));
    connect(ui->enableProxyCheckBox, SIGNAL(toggled(bool)), this, SLOT(onProxyEnabledChanged(bool)));
    connect(ui->ignoreSSLErrCheckBox, SIGNAL(toggled(bool)), this, SLOT(onIgnoreSSLErrChanged(bool)));
    connect(ui->proxyHostLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onProxyIPChanged(QString)));
    connect(ui->proxyPortSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onProxyPortChanged(int)));
    connect(ui->autoTextCopyCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoTextCopyChanged(bool)));
    connect(ui->autoSaveGroupBox, SIGNAL(toggled(bool)), this, SLOT(onAutoSaveToggled(bool)));
    connect(ui->fileSaveLineEdit, SIGNAL(textChanged(QString)), this , SLOT(onAutoSaveFileNameChanged(QString)));
    connect(ui->saveOnExitCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoSaveOnExitToggled(bool)));
    connect(ui->timerSaveCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoSaveTimerEnableToggled(bool)));
    connect(ui->timerSaveSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onAutoSaveTimerIntervalChanged(int)));
    connect(ui->slByteDataCheckBox, SIGNAL(toggled(bool)), this , SLOT(onDataSaveToggled(bool)));
    connect(ui->autoRestoreCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoRestoreToggled(bool)));
}

void SettingsDialog::disconnectUpdateSignals()
{
    disconnect(ui->autoUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoUpdateChanged(bool)));
    disconnect(ui->minimizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMinimizeChanged(bool)));
    disconnect(ui->defaultTabComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDefaultTabChanged(int)));
    disconnect(ui->decodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerDecodeChanged(bool)));
    disconnect(ui->encodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerEncodeChanged(bool)));
    disconnect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onServerPortChanged(int)));
    disconnect(ui->pipeNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onServerPipeNameChanged(QString)));
    disconnect(ui->hexWidget, SIGNAL(charChanged(char)), this, SLOT(onServerSeparatorChanged(char)));
    disconnect(ui->offsetBaseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onOffsetBaseChanged(QString)));
    disconnect(ui->enableProxyCheckBox, SIGNAL(toggled(bool)), this, SLOT(onProxyEnabledChanged(bool)));
    disconnect(ui->ignoreSSLErrCheckBox, SIGNAL(toggled(bool)), this, SLOT(onIgnoreSSLErrChanged(bool)));
    disconnect(ui->proxyHostLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onProxyIPChanged(QString)));
    disconnect(ui->proxyPortSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onProxyPortChanged(int)));
    disconnect(ui->autoTextCopyCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoTextCopyChanged(bool)));
    disconnect(ui->autoSaveGroupBox, SIGNAL(toggled(bool)), this, SLOT(onAutoSaveToggled(bool)));
    disconnect(ui->fileSaveLineEdit, SIGNAL(textChanged(QString)), this , SLOT(onAutoSaveFileNameChanged(QString)));
    disconnect(ui->saveOnExitCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoSaveOnExitToggled(bool)));
    disconnect(ui->timerSaveCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoSaveTimerEnableToggled(bool)));
    disconnect(ui->timerSaveSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onAutoSaveTimerIntervalChanged(int)));
    disconnect(ui->autoRestoreCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAutoRestoreToggled(bool)));
}


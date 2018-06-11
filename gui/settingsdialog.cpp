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

    connect(ui->checkUpdatePushButton, &QPushButton::clicked, this, &SettingsDialog::onUpdateRequest);
    connect(tManager, &TransformMgmt::savedUpdated, this, &SettingsDialog::updateRegisteredList);
    connect(guiHelper, &GuiHelper::markingsUpdated, this, &SettingsDialog::updateSavedMarkingColors);
    connect(guiHelper, &GuiHelper::importExportUpdated, this, &SettingsDialog::updateImportExportFuncs);
    connect(ui->resetMarkingsPushButton, &QPushButton::clicked, this, &SettingsDialog::onResetMarkings);
    connect(ui->resetIEFuncsPushButton, &QPushButton::clicked, this, &SettingsDialog::onResetImportExportFuncs);
    connect(ui->savedListWidget, &QListWidget::itemClicked, this, &SettingsDialog::onSavedClicked);
    connect(ui->pluginsListWidget, &QListWidget::clicked, this, &SettingsDialog::onPluginClicked);
    connect(ui->importExportListWidget, &QListWidget::itemDoubleClicked, this, &SettingsDialog::onDoubleClickImportExportFuncs);
    connect(ui->addImportExportPushButton, &QPushButton::clicked, this, &SettingsDialog::onAddImportExportFuncs);
    connect(ui->slByteDataCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onLoadSaveOptionsToggled);
    connect(ui->dataMarkingsCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onLoadSaveOptionsToggled);
    connect(ui->dataHistCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onLoadSaveOptionsToggled);
    connect(ui->quickViewCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onLoadSaveOptionsToggled);
    connect(ui->comparisonCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onLoadSaveOptionsToggled);
    connect(ui->guiPosCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onLoadSaveOptionsToggled);
    connect(ui->globalConfCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onLoadSaveOptionsToggled);
    connect(guiHelper, &GuiHelper::deletedTabsUpdated, this, &SettingsDialog::updateDeletedTabsList,Qt::QueuedConnection);
    connect(ui->clearAllTabsPushButton, &QPushButton::clicked, guiHelper, &GuiHelper::clearDeletedTabs);
    connect(ui->deletedTabsListWidget, &QListWidget::doubleClicked, this, &SettingsDialog::onDeletedTabsDoubleClicked);
    connect(ui->fileSavePushButton, &QPushButton::clicked, this , &SettingsDialog::onAutoSaveFileButtonclicked);
    connect(ui->customFontPushButton, &QPushButton::clicked, this, &SettingsDialog::onCustomFontClicked);
    connect(ui->saveNowPushButton, &QPushButton::clicked, this, &SettingsDialog::onSaveNowClicked);
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
    ui->customFontName->setText(QString("%1, %2px").arg(font.family()).arg(font.pointSize()));

    ui->hexadecimalTableRowsHeightSpinBox->setDisabled(true);
    ui->hexadecimalTableTextWidthSpinBox->setDisabled(true);
    ui->hexadecimalTableWidthSpinBox->setDisabled(true);

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
            connect(itemWid, &DeleteableListItem::itemDeleted, this, &SettingsDialog::onDeleteSaved);
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
            connect(itemWid, &DeleteableListItem::itemDeleted, this, &SettingsDialog::onMarkingDelete);
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
            connect(itemWid, &DeleteableListItem::itemDeleted, this, &SettingsDialog::onImportExportFuncDeletes);
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
        // as per Qt documentation the model is supposed to take owership of the item ... ASAN seems to disagree with that
        QStandardItem* item = new(std::nothrow) QStandardItem(typesList.at(i));
        if (item != nullptr) {
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setData( (typesBlacklist.contains(typesList.at(i)) ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);
            model->setItem(i, 0, item);
        } else {
          qFatal("Cannot allocate memory for QStandardItem 2 X{");
        }
    }
    connect(model, &QStandardItemModel::dataChanged, guiHelper, &GuiHelper::onFilterChanged);
    ui->filterListView->setModel(model);
}

void SettingsDialog::updateDeletedTabsList()
{
    ui->deletedTabsListWidget->clear();
    QList<TabAbstract *> list = guiHelper->getDeletedTabs();
    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i)->getName());
        if (itemWid != nullptr) {
            connect(itemWid, &DeleteableListItem::itemDeleted, this, &SettingsDialog::onDeletedTabsDeleted);
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
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
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
        ui->customFontName->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
        ui->customFontName->setText(QString("%1, %2px").arg(selectedFont.family()).arg(selectedFont.pointSize()));
        ui->hexadecimalTableWidthSpinBox->blockSignals(true);
        ui->hexadecimalTableWidthSpinBox->setValue(GlobalsValues::HEXCOLUMNWIDTH);
        ui->hexadecimalTableWidthSpinBox->blockSignals(false);
        ui->hexadecimalTableTextWidthSpinBox->blockSignals(true);
        ui->hexadecimalTableTextWidthSpinBox->setValue(GlobalsValues::TEXTCOLUMNWIDTH);
        ui->hexadecimalTableTextWidthSpinBox->blockSignals(false);
        ui->hexadecimalTableRowsHeightSpinBox->blockSignals(true);
        ui->hexadecimalTableRowsHeightSpinBox->setValue(GlobalsValues::ROWSHEIGHT);
        ui->hexadecimalTableRowsHeightSpinBox->blockSignals(false);
    }
}

void SettingsDialog::onSaveNowClicked()
{
    hide();
    emit forceAutoSave();
}

void SettingsDialog::onHexSizesValuesChanged()
{
    GlobalsValues::HEXCOLUMNWIDTH = ui->hexadecimalTableWidthSpinBox->value();
    GlobalsValues::TEXTCOLUMNWIDTH = ui->hexadecimalTableTextWidthSpinBox->value();
    GlobalsValues::ROWSHEIGHT = ui->hexadecimalTableRowsHeightSpinBox->value();
}

void SettingsDialog::connectUpdateSignals()
{
    connect(ui->autoUpdateCheckBox, &QCheckBox::toggled, this, &SettingsDialog::autoUpdateChanged);
    connect(ui->minimizeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onMinimizeChanged);
    //connect(ui->defaultTabComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingsDialog::onDefaultTabChanged);
    connect(ui->defaultTabComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDefaultTabChanged(int)));
    connect(ui->decodeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onServerDecodeChanged);
    connect(ui->encodeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onServerEncodeChanged);
    //connect(ui->portSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onServerPortChanged);
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onServerPortChanged(int)));
    connect(ui->pipeNameLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::onServerPipeNameChanged);
    connect(ui->hexWidget, &HexWidget::charChanged, this, &SettingsDialog::onServerSeparatorChanged);
    //connect(ui->offsetBaseComboBox, qOverload<const QString &>(&QComboBox::currentIndexChanged), this, &SettingsDialog::onOffsetBaseChanged);
    connect(ui->offsetBaseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onOffsetBaseChanged(QString)));
    connect(ui->enableProxyCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onProxyEnabledChanged);
    connect(ui->ignoreSSLErrCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onIgnoreSSLErrChanged);
    connect(ui->proxyHostLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::onProxyIPChanged);
    //connect(ui->proxyPortSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onProxyPortChanged);
    connect(ui->proxyPortSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onProxyPortChanged(int)));
    connect(ui->autoTextCopyCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoTextCopyChanged);
    connect(ui->autoSaveGroupBox, &QGroupBox::toggled, this, &SettingsDialog::onAutoSaveToggled);
    connect(ui->fileSaveLineEdit, &QLineEdit::textChanged, this , &SettingsDialog::onAutoSaveFileNameChanged);
    connect(ui->saveOnExitCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoSaveOnExitToggled);
    connect(ui->timerSaveCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoSaveTimerEnableToggled);
    //connect(ui->timerSaveSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onAutoSaveTimerIntervalChanged);
    connect(ui->timerSaveSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onAutoSaveTimerIntervalChanged(int)));
    connect(ui->slByteDataCheckBox, &QCheckBox::toggled, this , &SettingsDialog::onDataSaveToggled);
    connect(ui->autoRestoreCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoRestoreToggled);
    //connect(ui->hexadecimalTableRowsHeightSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onHexSizesValuesChanged);
    connect(ui->hexadecimalTableRowsHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHexSizesValuesChanged()));
    //connect(ui->hexadecimalTableTextWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onHexSizesValuesChanged);
    connect(ui->hexadecimalTableTextWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHexSizesValuesChanged()));
    //connect(ui->hexadecimalTableWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onHexSizesValuesChanged);
    connect(ui->hexadecimalTableWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHexSizesValuesChanged()));
}

void SettingsDialog::disconnectUpdateSignals()
{
    disconnect(ui->autoUpdateCheckBox, &QCheckBox::toggled, this, &SettingsDialog::autoUpdateChanged);
    disconnect(ui->minimizeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onMinimizeChanged);
    //disconnect(ui->defaultTabComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingsDialog::onDefaultTabChanged);
    disconnect(ui->defaultTabComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDefaultTabChanged(int)));
    disconnect(ui->decodeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onServerDecodeChanged);
    disconnect(ui->encodeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onServerEncodeChanged);
    //disconnect(ui->portSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onServerPortChanged);
    disconnect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onServerPortChanged(int)));
    disconnect(ui->pipeNameLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::onServerPipeNameChanged);
    disconnect(ui->hexWidget, &HexWidget::charChanged, this, &SettingsDialog::onServerSeparatorChanged);
    //disconnect(ui->offsetBaseComboBox, qOverload<const QString &>(&QComboBox::currentIndexChanged), this, &SettingsDialog::onOffsetBaseChanged);
    disconnect(ui->offsetBaseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onOffsetBaseChanged(QString)));
    disconnect(ui->enableProxyCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onProxyEnabledChanged);
    disconnect(ui->ignoreSSLErrCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onIgnoreSSLErrChanged);
    disconnect(ui->proxyHostLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::onProxyIPChanged);
    //disconnect(ui->proxyPortSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onProxyPortChanged);
    disconnect(ui->proxyPortSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onProxyPortChanged(int)));
    disconnect(ui->autoTextCopyCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoTextCopyChanged);
    disconnect(ui->autoSaveGroupBox, &QGroupBox::toggled, this, &SettingsDialog::onAutoSaveToggled);
    disconnect(ui->fileSaveLineEdit, &QLineEdit::textChanged, this , &SettingsDialog::onAutoSaveFileNameChanged);
    disconnect(ui->saveOnExitCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoSaveOnExitToggled);
    disconnect(ui->timerSaveCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoSaveTimerEnableToggled);
    //disconnect(ui->timerSaveSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onAutoSaveTimerIntervalChanged);
    disconnect(ui->timerSaveSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onAutoSaveTimerIntervalChanged(int)));
    disconnect(ui->slByteDataCheckBox, &QCheckBox::toggled, this , &SettingsDialog::onDataSaveToggled);
    disconnect(ui->autoRestoreCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoRestoreToggled);
//    disconnect(ui->hexadecimalTableRowsHeightSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onHexSizesValuesChanged);
    disconnect(ui->hexadecimalTableRowsHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHexSizesValuesChanged()));
//    disconnect(ui->hexadecimalTableTextWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onHexSizesValuesChanged);
    disconnect(ui->hexadecimalTableTextWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHexSizesValuesChanged()));
//    disconnect(ui->hexadecimalTableWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsDialog::onHexSizesValuesChanged);
    disconnect(ui->hexadecimalTableWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onHexSizesValuesChanged()));
}


/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QStandardItem>
#include <QStandardItemModel>
#include <QNetworkAccessManager>
#include <QApplication>
#include <QMutexLocker>
#include <QDebug>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <transformabstract.h>
#include <QXmlStreamWriter>
#include <QMimeData>
#include "guihelper.h"
#include "tabs/tababstract.h"
#include "sources/bytesourceabstract.h"
#include "newbytedialog.h"
#include "loggerwidget.h"
#include "textinputdialog.h"
#include "downloadmanager.h"
#include <transformmgmt.h>
#include <QDragEnterEvent>
#include <threadedprocessor.h>
#include <QNetworkProxy>
#include <QTimer>
#include "shared/guiconst.h"
#include "sources/blocksources/tlsserverlistener.h"

using namespace GuiConst;

const QString GuiHelper::LOGID = "GuiHelper";

GuiHelper::GuiHelper(TransformMgmt *ntransformFactory, QNetworkAccessManager *nmanager, LoggerWidget *nlogger)
{
    transformFactory = ntransformFactory;
    networkManager = nmanager;
    logger = nlogger;
    centralTransProc = nullptr;
    newTabMenu = nullptr;
    universalReceiver = nullptr;
    autoSaveTimer = nullptr;
    debugMode = false;
    defaultNewTab = GuiConst::TRANSFORM_PRETAB;
    offsetDefaultBase = GuiConst::DEFAULT_OFFSET_BASE;
    ignoreSSLErrors = GuiConst::DEFAULT_IGNORE_SSL;
    enableNetworkProxy = GuiConst::DEFAULT_PROXY_ENABLE;
    autoCopyTextTransformGui = GuiConst::DEFAULT_AUTO_COPY_TEXT;
    proxyInterface = GuiConst::DEFAULT_GLOBAL_PROXY_IP;
    proxyPort = GuiConst::DEFAULT_GLOBAL_PROXY_PORT;
    defaultServerPort = GuiConst::DEFAULT_PORT;
    defaultServerSeparator = GuiConst::DEFAULT_BLOCK_SEPARATOR;
    defaultServerPipeName = GuiConst::DEFAULT_PIPE_MASS;
    defaultServerDecode = GuiConst::DEFAULT_SERVER_DECODE;
    defaultServerEncode = GuiConst::DEFAULT_SERVER_ENCODE;
    defaultStateFlags = GuiConst::STATE_LOADSAVE_SAVE_ALL;
    autoSaveState = GuiConst::DEFAULT_AUTO_SAVE_ENABLED;
    autoSaveOnExit = GuiConst::DEFAULT_AUTO_SAVE_ON_EXIT;
    autoSaveTimerEnable = GuiConst::DEFAULT_AUTO_SAVE_TIMER_ENABLE;
    autoSaveTimerInterval = GuiConst::DEFAULT_AUTO_SAVE_TIMER_INTERVAL;
    regularFont = GuiStyles::DEFAULT_REGULAR_FONT;
    settings = nullptr;

    incomingBlockListener = new(std::nothrow) TLSServerListener(DEFAULT_INCOMING_SERVER_ADDRESS, DEFAULT_INCOMING_SERVER_PORT);
    if (incomingBlockListener == nullptr) {
        qFatal("Cannot allocate memory for TcpServerListener X{");
    }

    // by default everything is base64 encoded, to avoid parsing issues
    incomingBlockListener->setFlags(BlocksSource::TLS_OPTIONS | BlocksSource::B64BLOCKS_ENABLED);

    refreshAll();


    centralTransProc = new(std::nothrow) ThreadedProcessor();
    if (centralTransProc == nullptr) {
        qFatal("Cannot allocate memory for action CentralProcessor X{");
    }
}

GuiHelper::~GuiHelper()
{
    if (incomingBlockListener != nullptr) {
        delete incomingBlockListener;
        incomingBlockListener = nullptr;
    }

    delete centralTransProc;
    centralTransProc = nullptr;
    saveImportExportFunctions();
    deleteImportExportFuncs();
    deletedTabs.clear();
    // clearDeletedTabs(); // not needed because the maintab will delete them anyway;
    delete settings;
    logger = nullptr;
    if (autoSaveTimer != nullptr) {
        delete autoSaveTimer;
        autoSaveTimer = nullptr;
    }
}

LoggerWidget *GuiHelper::getLogger()
{
    return logger;
}

TransformMgmt *GuiHelper::getTransformFactory()
{
    return transformFactory;
}

QNetworkAccessManager *GuiHelper::getNetworkManager()
{
    return networkManager;
}

void GuiHelper::sendNewSelection(const QByteArray &selection)
{
    emit newSelection(selection);
}

void GuiHelper::sendToNewTab(const QByteArray &initialValue)
{
    emit newTabRequested(initialValue);
}

void GuiHelper::setUniveralReceiver(TabAbstract *tab)
{
    universalReceiver = tab;
}

void GuiHelper::addTab(TabAbstract *tab)
{
    connect(tab, SIGNAL(destroyed()), this, SLOT(onTabDestroyed()), Qt::UniqueConnection);
    tabNameUpdated(tab);
}

void GuiHelper::tabNameUpdated(TabAbstract *tab)
{
    tabs.insert(tab);
    updateSortedTabs();
    emit tabsUpdated();
}

void GuiHelper::removeTab(TabAbstract *tab)
{
    tabs.remove(tab);
    disconnect(tab, SIGNAL(destroyed()), this, SLOT(onTabDestroyed()));
    updateSortedTabs();
    emit tabsUpdated();
}

QList<TabAbstract *> GuiHelper::getTabs()
{
    return sortedTabs.values();
}

void GuiHelper::onTabDestroyed()
{
    TabAbstract * tg = static_cast<TabAbstract *>(sender());
    if (tg == nullptr) {
        logger->logError(tr("nullptr Deleted Tab"), LOGID);
    } else if (!tabs.remove(tg)) {
        logger->logError(tr("Deleted Tab not found"), LOGID);
    } else {
        updateSortedTabs();
        emit tabsUpdated();
    }
}

void GuiHelper::raisePip3lineWindow()
{
    emit raiseWindowRequest();
}

void GuiHelper::routeExternalDataBlock(QByteArray data)
{
    if (universalReceiver != nullptr) {
        universalReceiver->setData(data);
    } else {
        emit newTabRequested(data);
    }
}

void GuiHelper::updateSortedTabs()
{
    sortedTabs.clear();
    QSetIterator<TabAbstract *> i(tabs);
    while (i.hasNext()) {
     TabAbstract * tg = i.next();
     sortedTabs.insertMulti(tg->getName(), tg);
    }
}

TextInputDialog *GuiHelper::getNameDialog(QWidget *parent,const QString &defaultvalue, const QString &title)
{
    TextInputDialog *nameDialog; // yes this has to be cleaned at the upper layer.

    nameDialog = new(std::nothrow) TextInputDialog(parent);
    if (nameDialog != nullptr) {

        nameDialog->setModal(true);
        if (title.isEmpty())
            nameDialog->setWindowTitle(tr("Name"));
        else
            nameDialog->setWindowTitle(title);
        nameDialog->setText(defaultvalue);
    } else {
        qFatal("Could not instanciate textInputDialog X{");
    }

    return nameDialog;
}

void GuiHelper::buildTransformComboBox(QComboBox *box, const QString &defaultSelected, bool applyFilter)
{
    // creating transform combo box
    // Adding the first element (unselectable)
    QString tempLabel = tr("Choose Transformation");
    box->addItem(tempLabel);
    int row = 0;
    int defaultIndex = -1;

    // Set the flag of the item within the combobox model
    QStandardItem * item = qobject_cast<QStandardItemModel *>(box->model())->item(row);
    item->setEnabled( false );
    QStringList typesList = transformFactory->getTypesList();
    for (int i = 0; i < typesList.size(); i++) {
        if (applyFilter && typesBlacklist.contains(typesList.at(i)))
            continue;
        QStringList list = transformFactory->getTransformsList(typesList.at(i));
        if (list.isEmpty())
            continue;
        qSort(list);
        tempLabel = typesList.at(i);
        row = box->model()->rowCount();
        box->addItem(tempLabel);

        // Set the flag of the item within the combobox model
        QStandardItem * item = qobject_cast<QStandardItemModel *>(box->model())->item(row);
        item->setEnabled( false );
        item->setTextAlignment(Qt::AlignCenter);
        item->setBackground(Qt::darkGray);
        item->setForeground(Qt::white);
        //adding the transform for this category
        for (int j = 0; j < list.size(); j++) {
            if (list.at(j) == defaultSelected) {
                defaultIndex = box->model()->rowCount();
            }
            box->addItem(list.at(j));
        }
    }

    if (defaultIndex != -1)
        box->setCurrentIndex(defaultIndex);
    else if (!defaultSelected.isEmpty()){
        box->insertItem(1,defaultSelected);
        box->setCurrentIndex(1);
    }
}

QStringList GuiHelper::getDefaultQuickViews()
{
    QStringList list;
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int8lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int16lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int32lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int64lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int64leunsign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/utf16le.xml"));
    return list;
}

QStringList GuiHelper::getQuickViewConf()
{
    QStringList list = settings->value(SETTINGS_QUICKVIEWS, QStringList()).toStringList();
    if (list.isEmpty()) {
        list = getDefaultQuickViews();
    }
    return list;
}

void GuiHelper::saveQuickViewConf(QStringList conf)
{
    settings->setValue(SETTINGS_QUICKVIEWS, conf);
}

QHash<QString, QColor> GuiHelper::getDefaultMarkingsColor()
{
    QHash<QString, QColor> defaultColors;
    defaultColors.insert(tr("Data"),GuiStyles::DEFAULT_MARKING_COLOR_DATA);
    defaultColors.insert(tr("Size"),GuiStyles::DEFAULT_MARKING_COLOR_SIZE);
    defaultColors.insert(tr("Type"),GuiStyles::DEFAULT_MARKING_COLOR_TYPE);
    return defaultColors;
}

QHash<QString, QColor> GuiHelper::getMarkingsColor()
{
    return markingColors;
}

void GuiHelper::resetMarkings()
{
    markingColors.clear();
    saveMarkingsColor();
    markingColors = getDefaultMarkingsColor();
    emit markingsUpdated();
}

void GuiHelper::addNewMarkingColor(const QString &name, const QColor &color)
{
    markingColors.insert(name, color);
    saveMarkingsColor();
    emit markingsUpdated();
}

void GuiHelper::removeMarkingColor(const QString &name)
{
    if (markingColors.contains(name)) {
        markingColors.remove(name);
        saveMarkingsColor();
        emit markingsUpdated();
    }
}

void GuiHelper::saveMarkingsColor()
{
    QHash<QString, QVariant> colors;
    QHashIterator<QString, QColor> i(markingColors);
    while (i.hasNext()) {
        i.next();
        colors.insert(i.key(),i.value().rgb());
    }
    settings->setValue(SETTINGS_MARKINGS_COLORS, colors);
}

void GuiHelper::loadImportExportFunctions()
{
    QStringList list = settings->value(SETTINGS_EXPORT_IMPORT_FUNC, QStringList()).toStringList();
    if (list.isEmpty()) {
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/base64decode.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/hexadecimal.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/escapedhexadecimal.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/escapedhexadecimalmixed.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/cstylearray.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/urlencoded.xml"));
    }
    TransformAbstract * ta = nullptr;
    for (int i = 0; i < list.size(); i++) {
        TransformChain talist = transformFactory->loadConfFromXML(list.at(i));
        if (talist.isEmpty()) {
            logger->logError(tr("Could not load the configuration for Export/Import (skipping)\n%1").arg(list.at(i)), LOGID);
            continue;
        }
        else {
            ta = talist.takeFirst();
            connect(ta, SIGNAL(error(QString,QString)), logger, SLOT(logError(QString,QString)));
            connect(ta, SIGNAL(warning(QString,QString)), logger, SLOT(logWarning(QString,QString)));
            if (talist.size() > 0) {
                logger->logError(tr("Configuration loaded multiple transform, clearing the remaining ones."), LOGID);
                while (!talist.isEmpty()) {
                    delete talist.takeLast();
                }
            }
            importExportFunctions.insert(talist.getName(), ta);
        }
    }
}

void GuiHelper::saveImportExportFunctions()
{
    QHashIterator<QString, TransformAbstract *> i(importExportFunctions);
    QStringList savedConf;
    while (i.hasNext()) {
         i.next();
         QString conf;
         TransformChain list;
         list.append(i.value());
         list.setName(i.key());
         QXmlStreamWriter writer(&conf);
         transformFactory->saveConfToXML(list,&writer);
         savedConf.append(conf);
     }
    settings->setValue(SETTINGS_EXPORT_IMPORT_FUNC, savedConf);
}

QStringList GuiHelper::getImportExportFunctions()
{
    QStringList list = importExportFunctions.keys();
    qSort(list);
    return list;
}

TransformAbstract *GuiHelper::getImportExportFunction(const QString &name)
{

    if (name.isEmpty()) {
        logger->logError(tr("Empty export/import function name"));
        return nullptr;
    }

    if (importExportFunctions.contains(name)) {
        return importExportFunctions.value(name);
    }

    logger->logError(tr("Unkown import/export function name: \"%1\"").arg(name));
    return nullptr;
}

void GuiHelper::resetImportExportFuncs()
{
    deleteImportExportFuncs();
    saveImportExportFunctions();
    loadImportExportFunctions();
    emit importExportUpdated();
}

void GuiHelper::addImportExportFunctions(const QString &name, TransformAbstract * ta)
{
    if (name.isEmpty()) {
        logger->logError(tr("Empty export/import function name. Ignoring add request."));
        return;
    }

    if (ta == nullptr) {
        logger->logError(tr("Empty export/import function \"%1\" is nullptr. Ignoring add request.").arg(name));
        return;
    }

    if (importExportFunctions.contains(name)) { // overwriting by default
        TransformAbstract * oldta = importExportFunctions.take(name);
        delete oldta;
    }

    importExportFunctions.insert(name, ta);

    saveImportExportFunctions();

    emit importExportUpdated();
}

void GuiHelper::removeImportExportFunctions(const QString &name)
{
    if (name.isEmpty()) {
        logger->logError(tr("Empty export/import function name. Ignoring remove request."));
        return;
    }

    if (!importExportFunctions.contains(name)) {
        logger->logError(tr("Unknown export/import function \"%1\". Ignoring remove request.").arg(name));
        return;
    }

    TransformAbstract * ta = importExportFunctions.take(name);
    delete ta;

    saveImportExportFunctions();

    emit importExportUpdated();
}

void GuiHelper::deleteImportExportFuncs()
{
    if (!importExportFunctions.isEmpty()) {
        QHashIterator<QString , TransformAbstract *> i(importExportFunctions);
         while (i.hasNext()) {
             i.next();
             delete i.value();
         }

        importExportFunctions.clear();
    }
}

void GuiHelper::refreshNetworkProxySettings()
{
    QNetworkProxy networkProxy = networkManager->proxy();
    if (enableNetworkProxy) {
        networkProxy.setType(QNetworkProxy::HttpProxy);
        networkProxy.setHostName(proxyInterface);
        networkProxy.setPort(proxyPort);
    } else {
        networkProxy = QNetworkProxy::DefaultProxy;
    }
    networkManager->setProxy(networkProxy);
}

void GuiHelper::refreshIgnoreSSLSetting()
{
    QSslConfiguration currentConf = QSslConfiguration::defaultConfiguration();
    if (ignoreSSLErrors) {
        currentConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    }
    else {
        currentConf.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
    }
    QSslConfiguration::setDefaultConfiguration(currentConf);
}

void GuiHelper::refreshAutoSaveTimer()
{
    if (autoSaveTimerEnable) {
        if (autoSaveTimer == nullptr) {
           autoSaveTimer = new(std::nothrow) QTimer();
           if (autoSaveTimer == nullptr) {
               qFatal("Cannot allocate memory for auto save QTimer X{");
           }
           connect(autoSaveTimer, SIGNAL(timeout()), this, SIGNAL(requestSaveState()), Qt::UniqueConnection);
        }

        // value is initially in minutes, have to convert in ms
        autoSaveTimer->start(autoSaveTimerInterval * 60 * 1000);

    } else if (autoSaveTimer != nullptr) {
        autoSaveTimer->stop();
        delete autoSaveTimer;
        autoSaveTimer = nullptr;
    }
}
QMenu *GuiHelper::getNewTabMenu() const
{
    return newTabMenu;
}

void GuiHelper::setNewTabMenu(QMenu *value)
{
    newTabMenu = value;
    connect(newTabMenu, SIGNAL(destroyed(QObject*)), SLOT(onNewTabMenuDestroyed()), Qt::UniqueConnection);
}

QFont GuiHelper::getRegularFont() const
{
    return regularFont;
}

void GuiHelper::setRegularFont(const QFont &value)
{
    if (value != regularFont) {
        regularFont = value;
        GuiStyles::GLOBAL_REGULAR_FONT = regularFont;
        settings->setValue(GuiConst::SETTINGS_REGULAR_FONT, regularFont.toString());
    }
}

void GuiHelper::registerBlockSource(BlocksSource *bs)
{
    if (registeredBlockSources.contains(bs)) {
        logger->logWarning(tr("BlockSource already registered, ignoring.T_T"),"");
    } else {
        registeredBlockSources.append(bs);
        emit registeredBlockSourcesUpdated();
    }
}

void GuiHelper::unregisterBlockSource(BlocksSource *bs)
{
    if (registeredBlockSources.contains(bs)) {
        int num = registeredBlockSources.removeAll(bs);
        if (num != 1) {
            logger->logError(tr("More than one BlockSource have been removed: %1 T_T").arg(num),"");
        }
        emit registeredBlockSourcesUpdated();
    } else {
        logger->logError(tr("BlockSource is not registered T_T"),"");
    }
}

QList<BlocksSource *> GuiHelper::getRegisteredBlockSources()
{
    return registeredBlockSources;
}

void GuiHelper::clearRegisteredBlockSources()
{
    if (registeredBlockSources.size() > 0) {
        registeredBlockSources.clear();
        emit registeredBlockSourcesUpdated();
    }
}

void GuiHelper::registerOrchestrator(SourcesOrchestatorAbstract *orch)
{
    if (registeredOrchestators.contains(orch)) {
        logger->logWarning(tr("Orchestrator already registered, ignoring. T_T"),"");
    } else {
        registeredOrchestators << orch;
        emit registeredOrchestratorsUpdated();
    }
}

void GuiHelper::unregisterOrchestrator(SourcesOrchestatorAbstract *orch)
{
    if (registeredOrchestators.contains(orch)) {
        int num = registeredOrchestators.removeAll(orch);
        if (num != 1) {
            logger->logError(tr("More than one Orchestrator have been removed: %1 T_T").arg(num),"");
        }
        emit registeredOrchestratorsUpdated();
    } else {
        logger->logError(tr("Orchestrator is not registered T_T"),"");
    }
}

QList<SourcesOrchestatorAbstract *> GuiHelper::getRegisteredOrchestrators()
{
    return registeredOrchestators;
}

void GuiHelper::clearRegisteredOrchestrators()
{
    if (registeredOrchestators.size() > 0) {
        registeredOrchestators.clear();
        emit registeredOrchestratorsUpdated();
    }
}

BlocksSource *GuiHelper::getIncomingBlockListener() const
{
    return incomingBlockListener;
}

bool GuiHelper::getAutoRestoreOnStartup() const
{
    return autoRestoreOnStartup;
}

void GuiHelper::setAutoRestoreOnStartup(bool value)
{
    autoRestoreOnStartup = value;
    settings->setValue(GuiConst::SETTINGS_AUTO_RESTORE_ON_STARTUP, autoRestoreOnStartup);

}

int GuiHelper::getAutoSaveTimerInterval() const
{
    return autoSaveTimerInterval;
}

void GuiHelper::setAutoSaveTimerInterval(int value)
{
    if (autoSaveTimerInterval != value
            && value >= GuiConst::MIN_AUTO_SAVE_TIMER_INTERVAL
            && value <= GuiConst::MAX_AUTO_SAVE_TIMER_INTERVAL) {
        autoSaveTimerInterval = value;
        settings->setValue(GuiConst::SETTINGS_AUTO_SAVE_TIMER_INTERVAL, autoSaveTimerInterval);
        refreshAutoSaveTimer();
    } else {
        logger->logWarning(tr("Invalid timer value for auto save: %1. Ignoring it.").arg(value));
    }
}

bool GuiHelper::getAutoSaveTimerEnable() const
{
    return autoSaveTimerEnable;
}

void GuiHelper::setAutoSaveTimerEnable(bool value)
{
    autoSaveTimerEnable = value;
    settings->setValue(GuiConst::SETTINGS_AUTO_SAVE_TIMER_ENABLE, autoSaveTimerEnable);
    refreshAutoSaveTimer();
}

bool GuiHelper::getAutoSaveOnExit() const
{
    return autoSaveOnExit;
}

void GuiHelper::setAutoSaveOnExit(bool value)
{
    autoSaveOnExit = value;
    settings->setValue(GuiConst::SETTINGS_AUTO_SAVE_ON_EXIT, autoSaveOnExit);
}

QString GuiHelper::getAutoSaveFileName() const
{
    return autoSaveFileName;
}

void GuiHelper::setAutoSaveFileName(const QString &value)
{
    autoSaveFileName = value;
    settings->setValue(GuiConst::SETTINGS_AUTO_SAVE_SINGLE_FILENAME, autoSaveFileName);
}

bool GuiHelper::getAutoSaveState() const
{
    return autoSaveState;
}

void GuiHelper::setAutoSaveState(bool value)
{
    if (autoSaveState != value) {
        autoSaveState = value;
        settings->setValue(GuiConst::SETTINGS_AUTO_SAVE_ENABLE, autoSaveState);
    }
}

quint64 GuiHelper::getDefaultSaveStateFlags() const
{
    return defaultStateFlags | GuiConst::STATE_SAVE_REQUEST;
}

quint64 GuiHelper::getDefaultLoadStateFlags() const
{
    return defaultStateFlags & (~GuiConst::STATE_SAVE_REQUEST);
}

void GuiHelper::setDefaultStateFlags(const quint64 &value)
{
    defaultStateFlags = value;
    settings->setValue(GuiConst::SETTINGS_DEFAULT_SAVELOAD_FLAGS, defaultStateFlags);
}

bool GuiHelper::isAutoCopyTextTransformGui() const
{
    return autoCopyTextTransformGui;
}

void GuiHelper::setAutoCopyTextTransformGui(bool value)
{
    if (autoCopyTextTransformGui != value) {
        autoCopyTextTransformGui = value;
        settings->setValue(SETTINGS_AUTO_COPY_TRANSFORM, value);
    }
}

QList<TabAbstract *> GuiHelper::getDeletedTabs() const
{
    return deletedTabs;
}

TabAbstract *GuiHelper::takeDeletedTab(int index)
{
    if (!(index < deletedTabs.size())) {
        qWarning() << "[GuiHelper::takeDeletedTab] could not find the tab at index T_T" << index;
        return nullptr;
    }
    TabAbstract * tab = deletedTabs.takeAt(index);
    emit deletedTabsUpdated();
    return tab;
}

void GuiHelper::addDeletedTab(TabAbstract *dtab)
{
    if (deletedTabs.size() > MAX_DELETED_TABS_KEPT) {
        logger->logStatus(tr("Maximum number of deleted tabs reached, destroying the first one for good."));
        delete deletedTabs.takeFirst();
    }

    deletedTabs.append(dtab);
    emit deletedTabsUpdated();
}

void GuiHelper::reviveTab(int index)
{
    TabAbstract * tab = takeDeletedTab(index);

    if (tab != nullptr) {
        emit tabRevived(tab);
    }
}

void GuiHelper::clearDeletedTabs()
{
    while (!deletedTabs.isEmpty())
         delete deletedTabs.takeFirst();
    emit deletedTabsUpdated();
}

void GuiHelper::onNewTabMenuDestroyed()
{
    newTabMenu = nullptr;
}

void GuiHelper::logError(const QString &message, const QString &source)
{
    logger->logError(message,source);
}

void GuiHelper::logWarning(const QString &message, const QString &source)
{
    logger->logWarning(message,source);
}

void GuiHelper::logStatus(const QString &message, const QString &source)
{
    logger->logStatus(message,source);
}

void GuiHelper::logMessage(const QString &message, const QString &source, Pip3lineConst::LOGLEVEL level)
{
    logger->logMessage(message,source,level);
}

quint16 GuiHelper::getProxyPort() const
{
    return proxyPort;
}

void GuiHelper::setProxyPort(const quint16 &value)
{
    if (proxyPort != value) {
        proxyPort = value;
        refreshNetworkProxySettings();
        settings->setValue(SETTINGS_GLOBAL_PROXY_PORT, value);
    }
}

QString GuiHelper::getProxyInterface() const
{
    return proxyInterface;
}

void GuiHelper::setProxyInterface(const QString &value)
{
    if (proxyInterface != value) {
        proxyInterface = value;
        refreshNetworkProxySettings();
        settings->setValue(SETTINGS_GLOBAL_PROXY_IP, value);
    }
}

bool GuiHelper::getEnableNetworkProxy() const
{
    return enableNetworkProxy;
}

void GuiHelper::setEnableNetworkProxy(bool value)
{
    if (enableNetworkProxy != value) {
        enableNetworkProxy = value;

        refreshNetworkProxySettings();

        settings->setValue(SETTINGS_ENABLE_NETWORK_PROXY, value);
    }
}

bool GuiHelper::getIgnoreSSLErrors() const
{
    return ignoreSSLErrors;
}

void GuiHelper::setIgnoreSSLErrors(bool value)
{
    if (ignoreSSLErrors != value) {
        ignoreSSLErrors = value;
        refreshIgnoreSSLSetting();
        settings->setValue(SETTINGS_IGNORE_SSL_ERRORS, value);
    }
}

bool GuiHelper::getDebugMode() const
{
    return debugMode;
}

void GuiHelper::setDebugMode(bool value)
{
    debugMode = value;
}

void GuiHelper::refreshAll()
{
    bool ok = false;
    delete settings;
    settings = transformFactory->getSettingsObj();

    int vl = settings->value(GuiConst::SETTINGS_DEFAULT_TAB, GuiConst::TRANSFORM_PRETAB).toInt(&ok);

    if (ok && vl >= 0 && vl < GuiConst::AVAILABLE_TAB_STRINGS.size()) {
        defaultNewTab = (GuiConst::AVAILABLE_PRETABS)vl;
    }

    defaultServerPort = settings->value(GuiConst::SETTINGS_SERVER_PORT,GuiConst::DEFAULT_PORT).toInt(&ok);
    if (!ok || defaultServerPort < 1)
        defaultServerPort = GuiConst::DEFAULT_PORT;

    regularFont = QFont();
    regularFont.fromString(settings->value(GuiConst::SETTINGS_REGULAR_FONT,GuiStyles::DEFAULT_REGULAR_FONT).toString());
    GuiStyles::GLOBAL_REGULAR_FONT.fromString(regularFont.toString());

    QByteArray temp = settings->value(GuiConst::SETTINGS_SERVER_SEPARATOR,QByteArray()).toByteArray();
    temp = QByteArray::fromHex(temp);
    if (temp.isEmpty())
        defaultServerSeparator = GuiConst::DEFAULT_BLOCK_SEPARATOR;
    else
        defaultServerSeparator = temp.at(0);

    defaultServerDecode = settings->value(GuiConst::SETTINGS_SERVER_DECODE,GuiConst::DEFAULT_SERVER_DECODE).toBool();
    defaultServerEncode = settings->value(GuiConst::SETTINGS_SERVER_ENCODE, GuiConst::DEFAULT_SERVER_ENCODE).toBool();
    defaultServerPipeName = settings->value(GuiConst::SETTINGS_SERVER_PIPE_NAME, GuiConst::DEFAULT_PIPE_MASS).toString();

    offsetDefaultBase = settings->value(GuiConst::SETTINGS_OFFSET_BASE, GuiConst::DEFAULT_OFFSET_BASE).toInt(&ok);
    if (!ok || (offsetDefaultBase != 8 && offsetDefaultBase != 10 && offsetDefaultBase != 16)) {
        offsetDefaultBase = DEFAULT_OFFSET_BASE;
    }

    ignoreSSLErrors = settings->value(GuiConst::SETTINGS_IGNORE_SSL_ERRORS, GuiConst::DEFAULT_IGNORE_SSL).toBool();
    refreshIgnoreSSLSetting();

    enableNetworkProxy = settings->value(GuiConst::SETTINGS_ENABLE_NETWORK_PROXY, GuiConst::DEFAULT_PROXY_ENABLE).toBool();
    proxyInterface = settings->value(GuiConst::SETTINGS_GLOBAL_PROXY_IP, GuiConst::DEFAULT_GLOBAL_PROXY_IP).toString();
    proxyPort = settings->value(GuiConst::SETTINGS_GLOBAL_PROXY_PORT, GuiConst::DEFAULT_GLOBAL_PROXY_PORT).toUInt(&ok);
    if (!ok)
        proxyPort = GuiConst::DEFAULT_GLOBAL_PROXY_PORT;

    refreshNetworkProxySettings();

    autoCopyTextTransformGui = settings->value(GuiConst::SETTINGS_AUTO_COPY_TRANSFORM, GuiConst::DEFAULT_AUTO_COPY_TEXT).toBool();

    defaultStateFlags = settings->value(GuiConst::SETTINGS_DEFAULT_SAVELOAD_FLAGS, GuiConst::STATE_LOADSAVE_SAVE_ALL).toULongLong(&ok);
    if (!ok)
        defaultStateFlags = GuiConst::STATE_LOADSAVE_SAVE_ALL;

    autoSaveState = settings->value(GuiConst::SETTINGS_AUTO_SAVE_ENABLE, GuiConst::DEFAULT_AUTO_SAVE_ENABLED).toBool();
    autoSaveFileName = settings->value(GuiConst::SETTINGS_AUTO_SAVE_SINGLE_FILENAME,  transformFactory->getHomeDirectory().append(QDir::separator()).append(GuiConst::SETTINGS_AUTO_SAVE_FILENAME)).toString();
    autoSaveOnExit = settings->value(GuiConst::SETTINGS_AUTO_SAVE_ON_EXIT, GuiConst::DEFAULT_AUTO_SAVE_ON_EXIT).toBool();
    autoSaveTimerEnable = settings->value(GuiConst::SETTINGS_AUTO_SAVE_TIMER_ENABLE, GuiConst::DEFAULT_AUTO_SAVE_TIMER_ENABLE).toBool();
    autoSaveTimerInterval = settings->value(GuiConst::SETTINGS_AUTO_SAVE_TIMER_INTERVAL, GuiConst::DEFAULT_AUTO_SAVE_TIMER_INTERVAL).toInt(&ok);

    autoRestoreOnStartup = settings->value(GuiConst::SETTINGS_AUTO_RESTORE_ON_STARTUP, GuiConst::DEFAULT_AUTO_RESTORE_ENABLED).toBool();

    if (!ok || autoSaveTimerInterval < GuiConst::MIN_AUTO_SAVE_TIMER_INTERVAL || autoSaveTimerInterval > GuiConst::MAX_AUTO_SAVE_TIMER_INTERVAL) {
        autoSaveTimerInterval = GuiConst::DEFAULT_AUTO_SAVE_TIMER_INTERVAL;
    }
    refreshAutoSaveTimer();

    QStringList blacklist = settings->value(SETTINGS_FILTER_BLACKLIST, QStringList()).toStringList();
    typesBlacklist = blacklist.toSet();

    QHash<QString, QVariant> hash = settings->value(SETTINGS_MARKINGS_COLORS).toHash();
    if (hash.isEmpty())
        markingColors = getDefaultMarkingsColor();
    else {
        QHashIterator<QString, QVariant> i(hash);
        while (i.hasNext()) {
            i.next();
            QColor color(i.value().toUInt(&ok));
            if (!ok) {
                logger->logError(tr("Invalid color for marking from persistant conf"), LOGID);
                continue;
            }
            markingColors.insert(i.key(),color);
        }
    }

    deleteImportExportFuncs();
    loadImportExportFunctions();
    emit globalUpdates();
}

GuiConst::AVAILABLE_PRETABS GuiHelper::getDefaultNewTab() const
{
    return defaultNewTab;
}

void GuiHelper::setDefaultNewTab(AVAILABLE_PRETABS value)
{
    if (value != defaultNewTab) {
        defaultNewTab = value;
        settings->setValue(SETTINGS_DEFAULT_TAB, QVariant((int)value));
    }
}


ThreadedProcessor *GuiHelper::getCentralTransProc() const
{
    return centralTransProc;
}

void GuiHelper::processDragEnter(QDragEnterEvent *event, ByteSourceAbstract *byteSource)
{
    if ((event->mimeData()->hasHtml() ||
         event->mimeData()->hasText() ||
         event->mimeData()->hasUrls()) &&
            byteSource->hasCapability(ByteSourceAbstract::CAP_RESET))
        event->accept();
    else {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void GuiHelper::processDropEvent(QDropEvent *event, ByteSourceAbstract *byteSource, DownloadManager *downloadManager)
{
    if (byteSource == nullptr) {
        qCritical() << tr("Bytesource is nullptr, ignoring the drop event");
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
        return;
    }

    QStringList formats  = event->mimeData()->formats();

//    for (int i = 0; i < formats.size(); i++) {
//        qDebug() << formats.at(i);
//    }

    if (event->mimeData()->hasImage()) {
            qDebug() << event->mimeData()->imageData();
    } else if (event->mimeData()->hasUrls() || formats.contains("text/uri-list")) {

        QList<QUrl> list = event->mimeData()->urls();


        if (list.size() < 1) {
            logger->logError(tr("Url list is empty, nothing to drop"));
            event->acceptProposedAction();
            return;
        }

        if (list.size() > 1) {
            logger->logError(tr("Multiple urls entered, only opening the first one"));
        }

        QUrl resource = list.at(0);

        QString filename = QString(QByteArray::fromPercentEncoding(resource.toEncoded()));

        logger->logStatus(tr("Received %1 for Drop action").arg(filename));

        if (resource.scheme() == "file") {
            if (!byteSource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
                logger->logWarning(tr("%1 does not accept load file request").arg(byteSource->metaObject()->className()));
                return;
            }
#if defined(Q_OS_WIN32)
            if (filename.at(0) == '/') // bug on QT < 5 (Windows only)
                filename = filename.mid(1);
            if (filename.startsWith("file:///"))
                filename = filename.mid(8);
#else
            if (filename.startsWith("file:///"))
                filename = filename.mid(7);
#endif

            byteSource->fromLocalFile(filename);
        } else {
            requestDownload(resource, byteSource, downloadManager);
        }
        event->acceptProposedAction();
    } else if (event->mimeData()->hasText() || event->mimeData()->hasHtml()) {
        QUrl resource(event->mimeData()->text().toUtf8());

        if (resource.scheme() ==  "https" || resource.scheme() ==  "http" || resource.scheme() ==  "ftp" || resource.scheme() ==  "file") {
            int res = QMessageBox::question(nullptr, "Download ?","This looks like a valid URL.", QMessageBox::Yes, QMessageBox::No);
            if (res ==  QMessageBox::Yes) {
                logger->logStatus(tr("Received %1 for Drop action").arg(resource.toString()));

                requestDownload(resource, byteSource, downloadManager);

            } else {
                byteSource->setData(event->mimeData()->text().toUtf8());
            }
        } else {
            byteSource->setData(event->mimeData()->text().toUtf8());
        }
        event->acceptProposedAction();
    } else {
        logger->logWarning(tr("Don't know how to handle this Drop action"));
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void GuiHelper::requestDownload(QUrl url, ByteSourceAbstract *byteSource, DownloadManager *ndownloadManager)
{
    if (byteSource == nullptr && ndownloadManager == nullptr) { // double check , just in case of alcohol
        qCritical() << tr("Bytesource and downloadManager cannot be both nullptr at the same time, ignoring the download");
        return;
    }
    if (networkManager != nullptr) {
        DownloadManager * downloadManager = nullptr;
        if (ndownloadManager == nullptr) { // caller has not defined any DownloadManager, creating a default one, which will be managed here
            downloadManager = new(std::nothrow) DownloadManager(url, this);
            if (downloadManager == nullptr) {
                qFatal("Cannot allocate memory for setDownload downloadManager X{");
                return;
            }
            if (byteSource != nullptr) {
                connect(downloadManager,SIGNAL(finished(QByteArray)),byteSource, SLOT(setData(QByteArray)));
            }
        } else { // caller has supplied their own DownloadManager Object, use it
            downloadManager = ndownloadManager;
        }
        downloadManager->launch();

    } else {
        logger->logError(tr("No NetworkManager configured, no download launched"));
    }
}

QSet<QString> GuiHelper::getTypesBlacklist() const
{
    return typesBlacklist;
}

void GuiHelper::setDefaultServerPort(int port)
{
    defaultServerPort = port;
    settings->setValue(SETTINGS_SERVER_PORT, defaultServerPort);
}

int GuiHelper::getDefaultPort() const
{
    return defaultServerPort;
}

void GuiHelper::setDefaultServerPipeName(const QString &local)
{
    defaultServerPipeName = local;
    settings->setValue(SETTINGS_SERVER_PIPE_NAME, defaultServerPipeName);
}

QString GuiHelper::getDefaultServerPipeName() const
{
    return defaultServerPipeName;
}

void GuiHelper::setDefaultServerDecode(bool val)
{
    defaultServerDecode = val;
    settings->setValue(SETTINGS_SERVER_DECODE, defaultServerDecode);
}

bool GuiHelper::getDefaultServerDecode() const
{
    return defaultServerDecode;
}

void GuiHelper::setDefaultServerEncode(bool val)
{
    if (defaultServerEncode != val) {
        defaultServerEncode = val;
        settings->setValue(SETTINGS_SERVER_ENCODE, defaultServerEncode);
    }
}

bool GuiHelper::getDefaultServerEncode() const
{
    return defaultServerEncode;
}

void GuiHelper::setDefaultServerSeparator(char sep)
{
    if (defaultServerSeparator != sep) {
        defaultServerSeparator = sep;

    }
}

char GuiHelper::getDefaultServerSeparator() const
{
    return defaultServerSeparator;
}

int GuiHelper::getDefaultOffsetBase() const
{
    return offsetDefaultBase;
}

void GuiHelper::setDefaultOffsetBase(int val)
{
    if (offsetDefaultBase != val &&
            (offsetDefaultBase == 8 ||
             offsetDefaultBase == 10 ||
             offsetDefaultBase == 16)) {
        offsetDefaultBase = val;
        settings->setValue(SETTINGS_OFFSET_BASE,  offsetDefaultBase);
    }
}

void GuiHelper::goIntoHidding()
{
    emit appGoesIntoHidding();
}

void GuiHelper::isRising()
{
    emit appIsRising();
}

bool GuiHelper::eventFilter(QObject *o, QEvent *e)
{
    // Filtering out wheel event for comboboxes
    if ( e->type() == QEvent::Wheel && qobject_cast<QComboBox*>(o) )
    {
        e->ignore();
        return true;
    }
    return false;
}

void GuiHelper::updateCopyContextMenu(QMenu *copyMenu)
{
    QAction * action = nullptr;
    copyMenu->clear();

    action = new(std::nothrow) QAction(UTF8_STRING_ACTION, copyMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus UTF8 X{");
        return;
    }
    copyMenu->addAction(action);

    QStringList list = getImportExportFunctions();

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), copyMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateImportExportMenus copyMenu X{");
            return;
        }
        copyMenu->addAction(action);
    }
}

void GuiHelper::updateLoadContextMenu(QMenu *loadMenu)
{
    QAction * action = nullptr;
    loadMenu->clear();
    action = new(std::nothrow) QAction(UTF8_STRING_ACTION, loadMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus loadMenu UTF8 X{");
        return;
    }
    loadMenu->addAction(action);

    QStringList list = getImportExportFunctions();

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), loadMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateImportExportMenus loadMenu user's X{");
            return;
        }
        loadMenu->addAction(action);
    }
}

void GuiHelper::loadAction(QString action, ByteSourceAbstract *byteSource)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    if (action == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(this);
        if (dialog == nullptr) {
            qFatal("Cannot allocate memory for action NewByteDialog X{");
            return;
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            byteSource->setData(QByteArray(dialog->byteCount(),dialog->getChar()));
        }
        delete dialog;

    } else if (action == UTF8_STRING_ACTION) {
        byteSource->setData(input.toUtf8());
    } else {
        TransformAbstract *ta  = getImportExportFunction(action);
        if (ta != nullptr) {
            ta->setWay(TransformAbstract::OUTBOUND);
            byteSource->setData(ta->transform(input.toUtf8()));
        }
    }
}

void GuiHelper::copyAction(QString action, QByteArray value)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (action == UTF8_STRING_ACTION) {
        clipboard->setText(QString::fromUtf8(value));
    } else {
        TransformAbstract *ta  = getImportExportFunction(action);
        if (ta != nullptr) {
            ta->setWay(TransformAbstract::INBOUND);
            clipboard->setText(ta->transform(value));
        }
    }
}

void GuiHelper::saveToFileAction(QByteArray value, QWidget *parent)
{
    QString fileName = QFileDialog::getSaveFileName(parent,tr("Choose a file to save to"));
    if (!fileName.isEmpty()) {

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QString mess = tr("Failed to open the file for writing:\n %1").arg(file.errorString());
            logger->logError(mess);
            QMessageBox::critical(parent,tr("Error"), mess,QMessageBox::Ok);
            return;
        }
        file.write(value);
        file.close();
    }
}

const QString GuiHelper::getXMLfromRes(const QString &res)
{
    QFile file(res);
    file.open(QIODevice::ReadOnly);
    return QString::fromUtf8(file.readAll());
}


void GuiHelper::onFilterChanged(const QModelIndex &topLeft, const QModelIndex &)
{
    QString item = topLeft.data().toString();
    Qt::CheckState state = (Qt::CheckState) topLeft.data(Qt::CheckStateRole).toInt();
    if (state == Qt::Unchecked) {
        typesBlacklist.insert(item);
    } else {
        typesBlacklist.remove(item);
    }
    settings->setValue(SETTINGS_FILTER_BLACKLIST, (QStringList)typesBlacklist.toList());
    emit filterChanged();
}

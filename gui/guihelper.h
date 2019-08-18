/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef GUIHELPER_H
#define GUIHELPER_H

#include <QByteArray>
#include <QObject>
#include <QHash>
#include <QDialog>
#include <QComboBox>
#include <QString>
#include <QMultiHash>
#include <QColor>
#include <QSet>
#include <QMultiMap>
#include <QMenu>
#include <QFont>
#include <QSslConfiguration>
#include "shared/guiconst.h"
#include <commonstrings.h>

class TransformsGui;
class QSettings;
class TransformAbstract;
class TabAbstract;
class ByteSourceAbstract;
class ThreadedProcessor;
class QNetworkAccessManager;
class LoggerWidget;
class TransformMgmt;
class TransformRequest;
class TextInputDialog;
class QDragEnterEvent;
class DownloadManager;
class QTimer;
class BlocksSource;
class SourcesOrchestatorAbstract;
class PacketAnalyserTab;
class Packet;

class GuiHelper : public QObject
{
        Q_OBJECT
    public:
        explicit GuiHelper( TransformMgmt *transformFactory, QNetworkAccessManager *networkManager, LoggerWidget *logger);
        ~GuiHelper();
        LoggerWidget *getLogger();
        TransformMgmt *getTransformFactory();
        QNetworkAccessManager *getNetworkManager();

        void sendNewSelection(const QByteArray &selection);
        void sendToNewTab(const QByteArray &initialValue = QByteArray());
        void sendToNewHexEditor(const QByteArray &initialValue = QByteArray());

        void setUniveralReceiver(TabAbstract * tab);

        void addTab(TabAbstract *tab);
        void tabNameUpdated(TabAbstract *tab);
        void removeTab(TabAbstract * tab);
        QList<TabAbstract *> getTabs();

        TextInputDialog *getNameDialog(QWidget *parent, const QString &defaultvalue, const QString &title = QString());
        void buildTransformComboBox(QComboBox *box, const QString &defaultSelected = QString(), bool applyFilter = false);

        QStringList getDefaultQuickViews();
        QStringList getQuickViewConf();
        void saveQuickViewConf(QStringList conf);

        QHash<QString, QColor> getDefaultMarkingsColor();
        QHash<QString, QColor> getMarkingsColor();
        void resetMarkings();
        void addNewMarkingColor(const QString &name, const QColor &color);
        void removeMarkingColor(const QString &name);

        QStringList getImportExportFunctions();
        TransformAbstract * getImportExportFunction(const QString &name);
        void resetImportExportFuncs();
        void addImportExportFunctions(const QString &name, TransformAbstract *ta);
        void removeImportExportFunctions(const QString &name);

        void updateCopyContextMenu(QMenu *copyMenu);
        void updateLoadContextMenu(QMenu *loadMenu);
        void loadAction(QString action, ByteSourceAbstract * byteSource);
        void copyAction(QString action, QByteArray value);
        void saveToFileAction(QByteArray value, QWidget *parent = nullptr);

        void setDefaultServerPort(int port);
        int getDefaultPort() const;
        void setDefaultServerPipeName(const QString &local);
        QString getDefaultServerPipeName() const;
        void setDefaultServerDecode(bool val);
        bool getDefaultServerDecode() const;
        void setDefaultServerEncode(bool val);
        bool getDefaultServerEncode() const;
        void setDefaultServerSeparator(char sep);
        char getDefaultServerSeparator() const;

        int getDefaultOffsetBase() const;
        void setDefaultOffsetBase(int val);

        void goIntoHidding();
        void isRising();
        QSet<QString> getTypesBlacklist() const;
        ThreadedProcessor *getCentralTransProc() const;

        void processDragEnter(QDragEnterEvent * event, ByteSourceAbstract *byteSource);
        void processDropEvent(QDropEvent *event, ByteSourceAbstract *byteSource = nullptr, DownloadManager * downloadManager = nullptr);
        void requestDownload(QUrl url, ByteSourceAbstract *byteSource = nullptr, DownloadManager * downloadManager = nullptr);

        GuiConst::AVAILABLE_PRETABS getDefaultNewTab() const;
        void setDefaultNewTab(GuiConst::AVAILABLE_PRETABS value);

        bool getDebugMode() const;
        void setDebugMode(bool value);

        bool getIgnoreSSLErrors() const;
        void setIgnoreSSLErrors(bool value);

        bool getEnableNetworkProxy() const;
        void setEnableNetworkProxy(bool value);

        QString getProxyInterface() const;
        void setProxyInterface(const QString &value);

        quint16 getProxyPort() const;
        void setProxyPort(const quint16 &value);

        QList<TabAbstract *> getDeletedTabs() const;
        TabAbstract * takeDeletedTab(int index);
        void addDeletedTab(TabAbstract * dtab);
        void reviveTab(int index);

        bool isAutoCopyTextTransformGui() const;
        void setAutoCopyTextTransformGui(bool value);

        quint64 getDefaultSaveStateFlags() const;
        quint64 getDefaultLoadStateFlags() const;
        void setDefaultStateFlags(const quint64 &value);

        bool getAutoSaveState() const;
        void setAutoSaveState(bool value);

        QString getAutoSaveFileName() const;
        void setAutoSaveFileName(const QString &value);

        bool getAutoSaveOnExit() const;
        void setAutoSaveOnExit(bool value);

        bool getAutoSaveTimerEnable() const;
        void setAutoSaveTimerEnable(bool value);

        int getAutoSaveTimerInterval() const;
        void setAutoSaveTimerInterval(int value);

        void deleteImportExportFuncs();

        bool getAutoRestoreOnStartup() const;
        void setAutoRestoreOnStartup(bool value);

        BlocksSource *getIncomingBlockListener() const;

        QFont getRegularFont() const;
        void setRegularFont(const QFont &value);

        void saveEqualityPacketColors();
        void loadEqualityPacketColors();

        void saveJsonColor();
        void loadJsonColors();

        void saveSearchColors();
        void loadSearchColors();

        void registerBlockSource(BlocksSource * bs);
        void unregisterBlockSource(BlocksSource * bs);
        QList<BlocksSource *> getRegisteredBlockSources();
        void clearRegisteredBlockSources();

        void registerOrchestrator(SourcesOrchestatorAbstract * orch);
        void unregisterOrchestrator(SourcesOrchestatorAbstract * orch);
        QList<SourcesOrchestatorAbstract *> getRegisteredOrchestrators();
        void clearRegisteredOrchestrators();

        QMenu *getNewTabMenu() const;
        void setNewTabMenu(QMenu *value);

        quint64 getMaxSizeForFuzzingExport() const;
        void setMaxSizeForFuzzingExport(const quint64 &value);

        QJsonDocument::JsonFormat getFuzzingExportFormat() const;
        void setFuzzingExportFormat(const QJsonDocument::JsonFormat &value);

        void registerPacketsAnalyser(PacketAnalyserTab * paTab);
        void unregisterPacketsAnalyser(PacketAnalyserTab * paTab);
        QList<PacketAnalyserTab *> getRegisteredPacketsAnalysers() const;
        void clearRegisteredPacketsAnalysers();

    public slots:
        void refreshAll();
        void raisePip3lineWindow();
        void routeExternalDataBlock(QByteArray data);
        void clearDeletedTabs();
        void onNewTabMenuDestroyed();
        void logError(const QString &message, const QString &source = QString());
        void logWarning(const QString &message,const QString &source = QString());
        void logStatus(const QString &message,const QString &source = QString());
        void logMessage(const QString &message,const QString &source = QString(), Pip3lineConst::LOGLEVEL level = Pip3lineConst::PLSTATUS);
        void calculatingHexTableSizes();
        void onFilterChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
        void onTabDestroyed();
        void onInboundTransformRequested();
        void onOutboundTransformRequested();
    signals:
        void newDataSelection(QByteArray selection);
        void newTabRequested(QByteArray initialValue);
        void newHexEditorRequested(const QByteArray &initialValue);
        void newPacketAnalyserRequested(QList<QSharedPointer<Packet> >);
        void filterChanged();
        void markingsUpdated();
        void importExportUpdated();
        void tabsUpdated();
        void appGoesIntoHidding();
        void appIsRising();
        void raiseWindowRequest();
        void globalUpdates();
        void deletedTabsUpdated();
        void tabRevived(TabAbstract * dtab);
        void requestSaveState();
        void registeredBlockSourcesUpdated();
        void registeredOrchestratorsUpdated();
        void registeredPacketsAnalysersUpdated();
        void hexTableSizesUpdated();
    private:
        Q_DISABLE_COPY(GuiHelper)
        static const QString LOGID;
        bool eventFilter(QObject *o, QEvent *e);
        inline void updateSortedTabs();
        void loadImportExportFunctions();
        void saveImportExportFunctions();
        const QString getXMLfromRes(const QString &res);
        void saveMarkingsColor();
        void refreshNetworkProxySettings();
        void refreshIgnoreSSLSetting();
        void refreshAutoSaveTimer();
        QColor getColorFromSetting(const QString &key, const QColor &defaultcolor);
        TransformMgmt *transformFactory;
        QNetworkAccessManager *networkManager;
        LoggerWidget *logger;
        QSettings *settings;
        QSet<TabAbstract *> tabs;
        QMultiMap<QString, TabAbstract *> sortedTabs;
        QTimer *autoSaveTimer;
        int defaultServerPort;
        QString defaultServerIp;
        QString defaultServerPipeName;
        bool defaultServerDecode;
        bool defaultServerEncode;
        char defaultServerSeparator;
        QSet<QString> typesBlacklist;
        QHash<QString, QColor> markingColors;
        QHash<QString , TransformAbstract *> importExportFunctions;
        ThreadedProcessor * centralTransProc;
        int offsetDefaultBase;
        TabAbstract * universalReceiver;
        GuiConst::AVAILABLE_PRETABS defaultNewTab;
        bool ignoreSSLErrors;
        bool enableNetworkProxy;
        QString proxyInterface;
        quint16 proxyPort;
        QList<TabAbstract *> deletedTabs;
        bool autoCopyTextTransformGui;
        bool debugMode;
        quint64 defaultStateFlags;
        bool autoSaveState;
        QString autoSaveFileName;
        bool autoSaveOnExit;
        bool autoSaveTimerEnable;
        int autoSaveTimerInterval;
        bool autoRestoreOnStartup;
        QFont regularFont;
        quint64 maxSizeForFuzzingExport;
        QJsonDocument::JsonFormat fuzzingExportFormat;
        BlocksSource * incomingBlockListener;
        QList<BlocksSource *> registeredBlockSources;
        QList<SourcesOrchestatorAbstract *> registeredOrchestators;
        QList<PacketAnalyserTab *> registeredPacketsAnalysers;
        QMenu *newTabMenu;
};

#endif // GUIHELPER_H

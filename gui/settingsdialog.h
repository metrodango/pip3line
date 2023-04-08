/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "appdialog.h"
#include <QHash>
#include <QModelIndex>

namespace Ui {
class SettingsDialog;
}

class QSettings;
class TransformMgmt;
class QListWidgetItem;

class SettingsDialog : public AppDialog
{
        Q_OBJECT
    public:
        explicit SettingsDialog(GuiHelper *guiHelper, QWidget *parent = nullptr);
        ~SettingsDialog();
        void setVersionUpdateMessage(QString mess);
    public slots:
        void initializeConf();
    signals:
        void updateCheckRequested();
        void error(QString, QString);
        void forceAutoSave();
    private slots:
        void autoUpdateChanged(bool checked);
        void onUpdateRequest();
        void updatePluginList();
        void updateRegisteredList();
        void updateSavedMarkingColors();
        void updateImportExportFuncs();
        void updateMisc();
        void updateFilter();
        void updateDeletedTabsList();
        void onDeletedTabsDeleted(const QString name);
        void onDeletedTabsDoubleClicked(QModelIndex index);
        void onPluginClicked(QModelIndex index);
        void onSavedClicked(QListWidgetItem*item);
        void onDeleteSaved(const QString &name);
        void onMarkingDelete(const QString &name);
        void onResetMarkings();
        void onImportExportFuncDeletes(const QString &name);
        void onResetImportExportFuncs();
        void onDoubleClickImportExportFuncs(QListWidgetItem* item);
        void onAddImportExportFuncs();
        void onMinimizeChanged(bool checked);
        void onServerPortChanged(int port);
        void onServerDecodeChanged(bool val);
        void onServerEncodeChanged(bool val);
        void onServerSeparatorChanged(char c);
        void onServerPipeNameChanged(QString name);
        void onOffsetBaseChanged(QString val);
        void onIgnoreSSLErrChanged(bool ignore);
        void onProxyEnabledChanged(bool proxyEnable);
        void onProxyIPChanged(QString ipString);
        void onProxyPortChanged(int port);
        void onDefaultTabChanged(int index);
        void onAutoTextCopyChanged(bool val);
        void onLoadSaveOptionsToggled(bool checked);
        void onAutoSaveToggled(bool checked);
        void onAutoSaveFileNameChanged(QString name);
        void onAutoSaveFileButtonclicked();
        void onAutoSaveOnExitToggled(bool checked);
        void onAutoSaveTimerEnableToggled(bool checked);
        void onAutoSaveTimerIntervalChanged(int value);
        void onDataSaveToggled(bool checked);
        void onAutoRestoreToggled(bool checked);
        void onCustomFontClicked();
        void onSaveNowClicked();
        void onHexSizesValuesChanged();
        void onJsonColorChanges();
        void onSearchColorsChanges();
        void onResetAppearance();
        void onEqualPacketsColorsChanges();
    private:
        Q_DISABLE_COPY(SettingsDialog)
        void connectUpdateSignals();
        void disconnectUpdateSignals();
        static const QString LOGID;
        TransformMgmt *tManager;
        QHash<QString, int> stackedList;
        Ui::SettingsDialog *ui;
        QSettings *settings;
};

#endif // SETTINGSDIALOG_H

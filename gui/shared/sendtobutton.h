#ifndef SENDTOBUTTON_H
#define SENDTOBUTTON_H

#include <QPushButton>
#include <QAbstractTableModel>
#include "sources/blocksources/target.h"

class GuiHelper;
class TabAbstract;
class BlocksSource;
class SourcesOrchestatorAbstract;
class DefaultDialog;
class TargetsWidget;

class SendToModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit SendToModel(GuiHelper *guiHelper, QObject *parent = nullptr);
        ~SendToModel() override;
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const  override;
    public slots:
        void updateTabs();
        void updateBlockSource();
        void updateOrchestrator();
    private:
        Q_DISABLE_COPY(SendToModel)
        enum RowType {
            Invalid = -1,
            Orchestrator = 0,
            Blocksource,
            Tab
        };
        SendToModel::RowType translateRow(int &row) const;
        QList<TabAbstract *> sendToTabList;
        QList<BlocksSource *> sendToBlockSourceList;
        QList<SourcesOrchestatorAbstract *> sendToOrchestratorList;
        GuiHelper *guiHelper;        
};

class SendToButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit SendToButton(GuiHelper *guiHelper, QWidget *parent = nullptr);
        ~SendToButton() override;
    private slots:
        void onClicked();
    private:
        Q_DISABLE_COPY(SendToButton)
        GuiHelper *guiHelper;
        SendToModel *model;
        DefaultDialog *targetsDialog;
        TargetsWidget * targetsControl;
};

#endif // SENDTOBUTTON_H

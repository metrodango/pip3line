#include "packetanalyseroptionsdialog.h"
#include "ui_packetanalyseroptionsdialog.h"
#include "ui_transformdisplayconf.h"
#include "packetmodelabstract.h"
#include "packetsortfilterproxymodel.h"
#include <QTableView>
#include <QDebug>
#include <QModelIndexList>
#include <deleteablelistitem.h>
#include <QStandardItem>
#include <QColorDialog>
#include "quickviewitemconfig.h"
#include "guihelper.h"
#include "tabs/packetanalysertab.h"

const QStringList ColumnModel::colNames =  QStringList () << "Name"
                                        << "Hidden"
                                        << "Equality"
                                        << "DEL";

ColumnModel::ColumnModel(GuiHelper *guiHelper,
                         PacketModelAbstract *mainModel,
                         PacketSortFilterProxyModel *proxyModel,
                         QTableView *tableView,
                         QObject *parent) :
    QAbstractTableModel(parent),
    guiHelper(guiHelper),
    mainModel(mainModel),
    proxyModel(proxyModel),
    tableView(tableView)
{

}

ColumnModel::~ColumnModel()
{

}

int ColumnModel::columnCount(const QModelIndex &) const
{
    return colNames.size();
}

int ColumnModel::rowCount(const QModelIndex &) const
{
    return mainModel->columnCount();
}

QVariant ColumnModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int i = index.row();
    int col = index.column();
    if (role == Qt::DisplayRole) {
        if (col == COLUMN_NAME) {
            return QVariant(mainModel->getColumnName(i));
        }
    } else if (role == Qt::CheckStateRole) {
        if (col == COLUMN_HIDDEN)
            return (tableView->isColumnHidden(i) ? Qt::Checked : Qt::Unchecked);
        else if (col == COLUMN_EQUALITY) {
            return (proxyModel->isColumnEqualityenabled(i) ? Qt::Checked : Qt::Unchecked);
        }
    } else if (role == Qt::DecorationRole &&
               col == COLUMN_DELETE &&
               mainModel->isUserColumn(i)) {
        return QVariant(QIcon(":/Images/icons/dialog-cancel-3.png"));
    }

    return QVariant();
}

QVariant ColumnModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return colNames.at(section);
        }
    }
    return QVariant();
}

bool ColumnModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool ret = false;
    if (index.isValid() && role == Qt::CheckStateRole) {
        bool val = value.toBool();
        int i = index.row();
        int col = index.column();

        if (col == COLUMN_HIDDEN) {
            tableView->setColumnHidden(i,val);
            if (!val && tableView->columnWidth(i) == 0)
                tableView->setColumnWidth(i, mainModel->getDefaultWidthForColumn(col)); // need to do that otherwise the column stays hidden
            ret = true;
        } else if (col == COLUMN_EQUALITY) {
            proxyModel->setEqualitycolumn(i,val);
            ret = true;
        }
    }

    return ret;
}

Qt::ItemFlags ColumnModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    int col = index.column();
    if (col == COLUMN_HIDDEN || col == COLUMN_EQUALITY) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return QAbstractItemModel::flags(index);
}

int ColumnModel::getDefaultColumnwidth(int column)
{
    if (column >= 0 && column < colNames.size()) {
        QString columnName = colNames.at(column);
        columnName.append("  "); // cosmetics

        return GuiConst::calculateStringWidthWithGlobalFont(columnName);
    }

    return 0;
}

void ColumnModel::onColumnsUpdated()
{
    beginResetModel();
    endResetModel();
}

PacketAnalyserOptionsDialog::PacketAnalyserOptionsDialog(GuiHelper *guiHelper,
                                                         PacketModelAbstract *mainModel,
                                                         PacketSortFilterProxyModel *proxyModel,
                                                         QTableView *tableView,
                                                         QWidget *parent) :
    QDialog(parent),
    mainModel(mainModel),
    proxyModel(proxyModel),
    tableView(tableView),
    guiHelper(guiHelper),
    ui(new Ui::PacketAnalyserOptionsDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Packet analyser options"));

    uiTransform = new(std::nothrow) Ui::TransformDisplayConf ();
    if (uiTransform == nullptr) {
        qFatal("Cannot allocate memory for Ui::TransformDisplayConf X{");
    }
    uiTransform->setupUi(ui->confWidget);

    currentGui = nullptr;

    colModel = new(std::nothrow) ColumnModel(guiHelper, mainModel, proxyModel, tableView, ui->columnsTableView);
    if (colModel == nullptr) {
        qFatal("Cannot allocate memory for ColumnModel X{");
    }

    QAbstractItemModel * old = ui->columnsTableView->model();
    ui->columnsTableView->setModel(colModel);
    delete old;

    ui->columnsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->columnsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->columnsTableView->setMinimumHeight(200);
    ui->columnsTableView->setMinimumWidth(300);

    ui->columnsTableView->resizeColumnToContents(ColumnModel::COLUMN_NAME);
    ui->columnsTableView->setColumnWidth(ColumnModel::COLUMN_HIDDEN, colModel->getDefaultColumnwidth(ColumnModel::COLUMN_HIDDEN));
    ui->columnsTableView->setColumnWidth(ColumnModel::COLUMN_EQUALITY, colModel->getDefaultColumnwidth(ColumnModel::COLUMN_EQUALITY));
    ui->columnsTableView->setColumnWidth(ColumnModel::COLUMN_DELETE, colModel->getDefaultColumnwidth(ColumnModel::COLUMN_DELETE));

    ui->maxPayloadSizeSpinBox->setMinimum(PacketModelAbstract::MAX_PAYLOAD_DISPLAY_SIZE_MIN_VAL);
    ui->maxPayloadSizeSpinBox->setMaximum(PacketModelAbstract::MAX_PAYLOAD_DISPLAY_SIZE_MAX_VAL);
    ui->maxPayloadSizeSpinBox->setValue(mainModel->getMaxPayloadDisplaySize());

    QPalette example_palette;
    example_palette.setColor(QPalette::Window, GlobalsValues::EqualsPacketsBackground);
    example_palette.setColor(QPalette::WindowText, GlobalsValues::EqualsPacketsForeground);
    ui->equalityExampleLabel->setAutoFillBackground(true);
    ui->equalityExampleLabel->setPalette(example_palette);

    connect(ui->columnsTableView->selectionModel(), &QItemSelectionModel::selectionChanged,this, [=](QItemSelection selection,QItemSelection) { onItemSelected(selection);});
    connect(uiTransform->inboundRadioButton, &QRadioButton::toggled, this, &PacketAnalyserOptionsDialog::onInboundButtonToggled);
    connect(uiTransform->infoPushButton, &QPushButton::clicked, this, &PacketAnalyserOptionsDialog::onInfoClicked);
    connect(uiTransform->textRadioButton, &QRadioButton::toggled, this, &PacketAnalyserOptionsDialog::onTextFormatToggled);
    connect(ui->textRadioButton, &QRadioButton::toggled, this, &PacketAnalyserOptionsDialog::onTextFormatToggled);
    //connect(ui->maxPayloadSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), mainModel, &PacketModelAbstract::setMaxPayloadDisplaySize);
    connect(ui->maxPayloadSizeSpinBox, SIGNAL(valueChanged(int)), mainModel, SLOT(setMaxPayloadDisplaySize(int)));
    connect(ui->columnsTableView, &QTableView::clicked, this, &PacketAnalyserOptionsDialog::onIndexClicked);
    connect(mainModel, &PacketModelAbstract::columnsUpdated, this , &PacketAnalyserOptionsDialog::onColumnsUpdated);

    connect(ui->equalityBackgroundPushButton, &QPushButton::clicked, this, &PacketAnalyserOptionsDialog::onEqualityBackgroundClicked);
    connect(ui->equalityTextPushButton, &QPushButton::clicked, this, &PacketAnalyserOptionsDialog::onEqualityForegroundClicked);

    uiTransform->wayGroupBox->setVisible(true);
    uiTransform->formatGroupBox->setVisible(true);
    uiTransform->typeGroupBox->setVisible(false);
    uiTransform->nameWidget->setVisible(false);
    ui->maxPayloadSizeWidget->setVisible(false);

    connect(this, &PacketAnalyserOptionsDialog::rejected,this, &PacketAnalyserOptionsDialog::deleteLater);

    connect(ui->addPushButton, &QPushButton::clicked, this, &PacketAnalyserOptionsDialog::onAddNewColumn);

    onItemSelected(0);
}

PacketAnalyserOptionsDialog::~PacketAnalyserOptionsDialog()
{
    delete currentGui;
    currentGui = nullptr;
    delete ui;
    delete uiTransform;
}

void PacketAnalyserOptionsDialog::onItemSelected(QItemSelection index)
{
    QModelIndexList list =  index.indexes();
    if (list.isEmpty()) {
        ui->columnOptionsWidget->setEnabled(false);
        ui->columnOptionsWidget->setVisible(false);
    } else {
        onItemSelected(list.at(0).row());
        ui->columnOptionsWidget->setVisible(true);
        ui->columnOptionsWidget->setEnabled(true);
    }
}

 void PacketAnalyserOptionsDialog::onItemSelected(int index) {

     ui->maxPayloadSizeWidget->setVisible(false);

     //cleaning previous Transform Gui
     if (currentGui != nullptr) {
         delete currentGui;
         currentGui = nullptr;
     }
     if (mainModel->isUserColumn(index)) {
         currentGui = mainModel->getGuiForUserColumn(index);
         if (currentGui != nullptr) {
             uiTransform->mainLayout->addWidget(currentGui);
         }
         TransformAbstract * ta = mainModel->getTransform(index);
         if (ta != nullptr) {
             if (ta->isTwoWays()) {
                 uiTransform->wayGroupBox->setVisible(true);

                 if (ta->way() == TransformAbstract::INBOUND) {
                     uiTransform->inboundRadioButton->blockSignals(true);
                     uiTransform->inboundRadioButton->setChecked(true);
                     uiTransform->inboundRadioButton->blockSignals(false);
                 }
                 else {
                     uiTransform->outboundRadioButton->blockSignals(true);
                     uiTransform->outboundRadioButton->setChecked(true);
                     uiTransform->outboundRadioButton->blockSignals(false);
                 }
             } else {
                 uiTransform->wayGroupBox->setVisible(false);
             }
             ui->confWidget->setVisible(true);
             Pip3lineConst::OutputFormat of = mainModel->getColumnFormat(index);
             if (of == Pip3lineConst::TEXTFORMAT) {
                 uiTransform->textRadioButton->blockSignals(true);
                 uiTransform->textRadioButton->setChecked(true);
                 uiTransform->textRadioButton->blockSignals(false);
             }
             else {
                 uiTransform->hexaRadioButton->blockSignals(true);
                 uiTransform->hexaRadioButton->setChecked(true);
                 uiTransform->hexaRadioButton->blockSignals(false);
             }
             setFormatVisible(false);

         } else {
             ui->confWidget->setVisible(false);
             setFormatVisible(true, mainModel->getColumnFormat(index));
         }
     } else {
         ui->confWidget->setVisible(false);
         if (index == PacketModelAbstract::COLUMN_PAYLOAD) {
             setFormatVisible(true, mainModel->getColumnFormat(index));
             ui->maxPayloadSizeWidget->setVisible(true);
         } else {
             setFormatVisible(false);
         }
     }
     adjustSize();
 }

void PacketAnalyserOptionsDialog::onAddNewColumn()
{
    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (itemConfig == nullptr) {
        qFatal("Cannot allocate memory for QuickViewItemConfig X{");
    }
    itemConfig->setWayBoxVisible(true);
    itemConfig->setFormatVisible(true);
    itemConfig->setOutputTypeVisible(false);
    int ret = itemConfig->exec();
    if (ret == QDialog::Accepted) {
        TransformAbstract * ta = itemConfig->getTransform();
        if (ta != nullptr) {
            mainModel->addUserColumn(itemConfig->getName(), ta, itemConfig->getFormat());

//            int newIndex = addGeneralItem(itemConfig->getName());

//            ui->columnsTableView->item(newIndex)->setSelected(true);
            currentGui = ta->getGui(ui->columnOptionsWidget);
            if (currentGui != nullptr) {
                uiTransform->mainLayout->addWidget(currentGui);
            }
        }
    }

    delete itemConfig;
}

void PacketAnalyserOptionsDialog::onInboundButtonToggled(bool checked)
{
    QModelIndexList cols = ui->columnsTableView->selectionModel()->selectedRows();
    if (cols.size() == 0) {
        return;
    } else {
        TransformAbstract * ta = mainModel->getTransform(cols.at(0).row());
        if (ta != nullptr) {
            ta->setWay(checked ? TransformAbstract::INBOUND : TransformAbstract::OUTBOUND);
        }
    }
}

void PacketAnalyserOptionsDialog::onTextFormatToggled(bool checked)
{
    QModelIndexList cols = ui->columnsTableView->selectionModel()->selectedRows();
    if (cols.size() == 0) {
        return;
    } else { // there should be only one column, so only considering the first in the list
        mainModel->setColumnFormat(cols.at(0).row(), checked ? Pip3lineConst::TEXTFORMAT : Pip3lineConst::HEXAFORMAT);
    }
}

void PacketAnalyserOptionsDialog::onInfoClicked()
{
    QModelIndexList cols = ui->columnsTableView->selectionModel()->selectedRows();
    if (cols.size() == 0) {
        return;
    } else {
        TransformAbstract * ta = mainModel->getTransform(cols.at(0).row());
        if (ta != nullptr) {
            ta->description();
        }
    }
}

void PacketAnalyserOptionsDialog::onEqualityBackgroundClicked()
{
    QColor choosenColor = QColorDialog::getColor(GlobalsValues::EqualsPacketsBackground, this);
    if (choosenColor.isValid()) {
        QPalette example_palette;
        GlobalsValues::EqualsPacketsBackground = choosenColor;
        example_palette.setColor(QPalette::Window, GlobalsValues::EqualsPacketsBackground);
        example_palette.setColor(QPalette::WindowText, GlobalsValues::EqualsPacketsForeground);
        ui->equalityExampleLabel->setPalette(example_palette);
        guiHelper->saveEqualityPacketColors();
    }
}

void PacketAnalyserOptionsDialog::onEqualityForegroundClicked()
{
    QColor choosenColor = QColorDialog::getColor(GlobalsValues::EqualsPacketsForeground, this);
    if (choosenColor.isValid()) {
        QPalette example_palette;
        GlobalsValues::EqualsPacketsForeground = choosenColor;
        example_palette.setColor(QPalette::Window, GlobalsValues::EqualsPacketsBackground);
        example_palette.setColor(QPalette::WindowText, GlobalsValues::EqualsPacketsForeground);
        ui->equalityExampleLabel->setPalette(example_palette);
        guiHelper->saveEqualityPacketColors();
    }
}

void PacketAnalyserOptionsDialog::onIndexClicked(const QModelIndex &index)
{
    if (index.column() == ColumnModel::COLUMN_DELETE) {
        int row = index.row();
        if (mainModel->isUserColumn(row))
            mainModel->removeUserColumn(row);

        currentGui = nullptr;
        ui->confWidget->setVisible(false);
    }
}

void PacketAnalyserOptionsDialog::onColumnsUpdated()
{
    colModel->onColumnsUpdated();
    ui->columnsTableView->resizeColumnToContents(ColumnModel::COLUMN_NAME);
}

void PacketAnalyserOptionsDialog::setFormatVisible(bool visible, OutputFormat format)
{
    ui->vierwFormatGroupBox->setVisible(visible);
    if (visible) {

        if (format == Pip3lineConst::TEXTFORMAT) {
            ui->textRadioButton->blockSignals(true);
            ui->textRadioButton->setChecked(true);
            ui->textRadioButton->blockSignals(false);
        }
        else {
            ui->hexaRadioButton->blockSignals(true);
            ui->hexaRadioButton->setChecked(true);
            ui->hexaRadioButton->blockSignals(false);
        }
    }
}


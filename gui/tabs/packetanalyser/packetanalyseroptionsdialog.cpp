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
#include "quickviewitemconfig.h"
#include "guihelper.h"

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

    ui->colListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->colListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    addItems(mainModel->getColumnNames());
    connect(ui->colListWidget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onItemSelected(QItemSelection)));
    connect(ui->equalitycheckBox, SIGNAL(toggled(bool)), SLOT(onEqualityToggled(bool)));
    connect(ui->hiddenCheckBox, SIGNAL(toggled(bool)), SLOT(onHiddenToggled(bool)));
    connect(uiTransform->inboundRadioButton, SIGNAL(toggled(bool)), SLOT(onInboundButtonToggled(bool)));
    connect(uiTransform->infoPushButton, SIGNAL(clicked()), this, SLOT(onInfoClicked()));
    connect(uiTransform->textRadioButton, SIGNAL(toggled(bool)), this, SLOT(onTextFormatToggled(bool)));
    connect(ui->textRadioButton, SIGNAL(toggled(bool)), this, SLOT(onTextFormatToggled(bool)));

    uiTransform->wayGroupBox->setVisible(true);
    uiTransform->formatGroupBox->setVisible(true);
    uiTransform->typeGroupBox->setVisible(false);
    uiTransform->nameWidget->setVisible(false);

    connect(this, SIGNAL(rejected()), SLOT(deleteLater()));

    connect(ui->addPushButton, SIGNAL(clicked(bool)), SLOT(onAddNewColumn()));

    onItemSelected(0);

    resize(600, 500);
}

PacketAnalyserOptionsDialog::~PacketAnalyserOptionsDialog()
{
    delete currentGui;
    currentGui = nullptr;
    delete ui;
}

void PacketAnalyserOptionsDialog::onHiddenToggled(bool checked)
{
    QModelIndexList cols = ui->colListWidget->selectionModel()->selectedRows();
    if (cols.size() == 0) {
        return;
    } else {
        tableView->setColumnHidden(cols.at(0).row(),checked);
    }
}

void PacketAnalyserOptionsDialog::onEqualityToggled(bool checked)
{
    QModelIndexList cols = ui->colListWidget->selectionModel()->selectedRows();
    if (cols.size() == 0) {
        return;
    } else {
        proxyModel->setEqualitycolumn(cols.at(0).row(),checked);
    }
}

void PacketAnalyserOptionsDialog::onItemSelected(QItemSelection index)
{
    QModelIndexList list =  index.indexes();
    if (list.isEmpty()) {
        ui->colOptionsWidget->setEnabled(false);
    } else {
        onItemSelected(list.at(0).row());
        ui->colOptionsWidget->setEnabled(true);
    }
}

 void PacketAnalyserOptionsDialog::onItemSelected(int index) {

   //  qDebug() << tr("Column %1 selected, hidden: %2").arg(index).arg(tableView->isColumnHidden(index));
     ui->hiddenCheckBox->blockSignals(true);
     ui->hiddenCheckBox->setChecked(tableView->isColumnHidden(index));
     ui->hiddenCheckBox->blockSignals(false);
     ui->equalitycheckBox->blockSignals(true);
     ui->equalitycheckBox->setChecked(proxyModel->isColumnEqualityenabled(index));
     ui->equalitycheckBox->blockSignals(false);
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

            int newIndex = addItem(itemConfig->getName());

            ui->colListWidget->item(newIndex)->setSelected(true);
            currentGui = ta->getGui(ui->colOptionsWidget);
            if (currentGui != nullptr) {
                uiTransform->mainLayout->addWidget(currentGui);
            }
        }
    }

    delete itemConfig;
}

void PacketAnalyserOptionsDialog::onDeleteColumn(const QString &name)
{
    int index = mainModel->getColumnIndex(name);
    if (index != PacketModelAbstract::COLUMN_INVALID) {
        mainModel->removeUserColumn(name);
        QListWidgetItem * item = itemsWidgets.value(dynamic_cast<QWidget *>(sender()),nullptr);
        if (item != nullptr) {
            ui->colListWidget->removeItemWidget(item); // need to remove the widget manually .. for some reasons
            ui->colListWidget->takeItem(ui->colListWidget->row(item));
            delete item;
        } else {
            qCritical() << "[PacketAnalyserOptionsDialog::onDeleteColumn] Cnnot find the associated item T_T";
        }

        ui->colListWidget->selectionModel()->clearSelection();
    }
}

void PacketAnalyserOptionsDialog::onInboundButtonToggled(bool checked)
{
    QModelIndexList cols = ui->colListWidget->selectionModel()->selectedRows();
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
    QModelIndexList cols = ui->colListWidget->selectionModel()->selectedRows();
    if (cols.size() == 0) {
        return;
    } else { // there should be only one column, so only considering the first in the list
        mainModel->setColumnFormat(cols.at(0).row(), checked ? Pip3lineConst::TEXTFORMAT : Pip3lineConst::HEXAFORMAT);
    }
}

void PacketAnalyserOptionsDialog::onInfoClicked()
{
    QModelIndexList cols = ui->colListWidget->selectionModel()->selectedRows();
    if (cols.size() == 0) {
        return;
    } else {
        TransformAbstract * ta = mainModel->getTransform(cols.at(0).row());
        if (ta != nullptr) {
            ta->description();
        }
    }
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

int PacketAnalyserOptionsDialog::addItem(const QString &name)
{
    int newIndex = ui->colListWidget->count();
    DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(name);

    if (itemWid != nullptr) {
        if (mainModel->isUserColumn(name)) {
            itemWid->setEnableDelete(true);
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onDeleteColumn(QString)), Qt::QueuedConnection);
        } else {
            itemWid->setEnableDelete(false);
        }
        QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
        if (item != nullptr) {
            ui->colListWidget->addItem(item);
            ui->colListWidget->setItemWidget(item, itemWid);
            itemsWidgets.insert(itemWid, item);
        } else {
            qFatal("Cannot allocate memory for QListWidgetItem X{");
        }
    } else {
        qFatal("Cannot allocate memory for DeleteableListItem X{");
    }

    return newIndex;
}

int PacketAnalyserOptionsDialog::addItems(const QStringList &names)
{
    int index = 0;
    for (int i = 0; i < names.size(); i++) {
        index = addItem(names.at(i));
    }

    return index; // return the last index
}

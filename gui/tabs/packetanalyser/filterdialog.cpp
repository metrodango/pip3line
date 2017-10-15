#include "filterdialog.h"
#include "ui_filterdialog.h"
#include "../../shared/guiconst.h"
#include <QDebug>
#include <deleteablelistitem.h>
#include <QMessageBox>
#include <QTextCodec>
#include <QTextEncoder>
#include <QEvent>
#include <QKeyEvent>
#include "filteritem.h"
#include "packetsortfilterproxymodel.h"

FilterDialog::FilterDialog(PacketSortFilterProxyModel *sortFilterProxyModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterDialog),
    sortFilterProxyModel(sortFilterProxyModel)
{
    ui->setupUi(this);

    setWindowTitle("Configure View filters");
    ui->enableCheckBox->setChecked(sortFilterProxyModel->isFilteringEnabled());
    ui->filterNameLineEdit->setFocus();
    ui->utf8RadioButton->setChecked(true);
    ui->startOffsetLineEdit->setText("0");

    list = sortFilterProxyModel->getFilterList();
    updateList();

    ui->filterNameLineEdit->installEventFilter(this);
    ui->filterLineEdit->installEventFilter(this);
    ui->filtersGroupBox->installEventFilter(this);

    connect(this, SIGNAL(rejected()), SLOT(deleteLater()));
    connect(this, SIGNAL(finished(int)), SLOT(deleteLater()));
    connect(ui->addPushButton, SIGNAL(clicked(bool)), this, SLOT(onAdd()));
    connect(ui->enableCheckBox, SIGNAL(toggled(bool)), this, SLOT(onFilterToggled(bool)));
    connect(ui->filtersListWidget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onItemSelected(QItemSelection)));
    connect(ui->clearPushButton, SIGNAL(clicked(bool)), this, SLOT(onClear()));
    connect(ui->applyPushButton, SIGNAL(clicked(bool)), this, SLOT(onSave()));
}

FilterDialog::~FilterDialog()
{
    qDebug() << "destroying" << this;
    delete ui;
    sortFilterProxyModel = nullptr;
}

void FilterDialog::onAdd()
{
    QString name = ui->filterNameLineEdit->text();
    for (int i = 0 ; i < list.size(); i++) {
        if (list.at(i).getName() == name) {
            QMessageBox::critical(this,tr("Duplicate filter name"),tr("Another filter already has this name. Choose another one."),QMessageBox::Ok);
            return;
        }
    }

    FilterItem item = getCurrentConfItem();
    if (item.isValid()) {
        list.append(item);
        sortFilterProxyModel->setFilterList(list);
        updateList();
    }
}

void FilterDialog::updateList()
{
    ui->filtersListWidget->clear();

    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i).getName());
        if (itemWid != nullptr) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onDeleteItem(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != nullptr) {
                ui->filtersListWidget->addItem(item);
                ui->filtersListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem filter item X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem filter item X{");
        }
    }
}

void FilterDialog::onDeleteItem(QString name)
{
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).getName() == name) {
            list.removeAt(i);
            ui->filtersListWidget->clearSelection();
            sortFilterProxyModel->setFilterList(list);
            updateList();
            return;// there supposed to be only one ...
        }
    }
    // we should never arrive here
    qCritical() << tr("The item \"%1\" was not found T_T").arg(name);
}

void FilterDialog::onFilterToggled(bool toggled)
{
    sortFilterProxyModel->setFilteringEnabled(toggled);
}

void FilterDialog::onItemSelected(QItemSelection selection)
{
    QModelIndexList indexList =  selection.indexes();
    if (!indexList.isEmpty()) {
        int index = indexList.at(0).row();
        if (index >= 0 && index < list.size()) {
            FilterItem item = list.at(index);
            ui->filterNameLineEdit->setText(item.getName());
            ui->hexaRadioButton->setChecked(item.isReverseSelection());
            int offset = item.getStartingOffset();
            QString off_str = QString::number(offset, 10);
            ui->startOffsetLineEdit->setText(off_str);
            if (item.getSearchType() == FilterItem::HEXA) {
                ui->hexaRadioButton->setChecked(true);
                ui->filterLineEdit->setText(item.getHexValueWithMask());
            } else if (item.getSearchType() == FilterItem::UTF8) {
                ui->utf8RadioButton->setChecked(true);
                ui->filterLineEdit->setText(QString::fromUtf8(item.getValue()));
            } else {
                ui->utf16RadioButton->setChecked(true);
                QTextCodec * codec = QTextCodec::codecForName("UTF-16");
                if (codec == nullptr) {
                    qCritical() << tr("Could not find the UTF-16 codec T_T");
                } else {
                    QTextDecoder *decoder = codec->makeDecoder(QTextCodec::IgnoreHeader | QTextCodec::ConvertInvalidToNull);
                    QString textf = decoder->toUnicode(item.getValue().constData(),item.getValue().size());
                    if (decoder->hasFailure()) {
                        qWarning() << tr("[FilterDialog::onItemSelected] decoding from UTF-16 failed at some point...");
                    }

                    ui->filterNameLineEdit->setText(textf);
                    delete decoder;
                }
            }
        }
        ui->applyPushButton->setEnabled(true);
    } else {
        ui->applyPushButton->setEnabled(false);
    }
}

void FilterDialog::onClear()
{
    ui->filterNameLineEdit->setText(QString(""));
    ui->filterLineEdit->setText(QString(""));
    ui->startOffsetLineEdit->setText(QString("0"));
    ui->hexaRadioButton->setChecked(true);
    ui->reverseCheckBox->setChecked(false);
    ui->filtersListWidget->clearSelection();
}

void FilterDialog::onSave()
{
    QModelIndexList indexList = ui->filtersListWidget->selectionModel()->selectedIndexes();
    if (!indexList.isEmpty()) {
        int index = indexList.at(0).row();
        if (index >= 0 && index < list.size()) {
            FilterItem item = getCurrentConfItem();
            if (item.isValid()) {
                list.replace(index,item);
                sortFilterProxyModel->setFilterList(list);
                updateList();
                ui->filtersListWidget->blockSignals(false);
                ui->filtersListWidget->selectionModel()->select(indexList.at(0), QItemSelectionModel::Select);
                ui->filtersListWidget->blockSignals(true);
            }
        }
    }
}

FilterItem FilterDialog::getCurrentConfItem()
{
    FilterItem item;
    QString name = ui->filterNameLineEdit->text();
    QString searchTerm = ui->filterLineEdit->text();

    if (name.isEmpty()) {
        QMessageBox::critical(this,tr("Empty filter name"),tr("The name field is empty"),QMessageBox::Ok);
        return item;
    }
    if (searchTerm.isEmpty()) {
        QMessageBox::critical(this,tr("Empty filter"),tr("The search term is empty"),QMessageBox::Ok);
        return item;
    }

    item.setName(name);

    bool ok = false;
    int offset = ui->startOffsetLineEdit->text().toInt(&ok,10);
    if (ok && offset >= 0)
        item.setStartingOffset(offset);

    QBitArray mask;
    QByteArray data;
    if (ui->hexaRadioButton->isChecked()) {
        item.setSearchType(FilterItem::HEXA);
        data = GuiConst::extractSearchHexa(searchTerm, mask);
        if (data.isEmpty()) {
            QMessageBox::critical(this,tr("Invalid Hexadecimal value"),tr("The hexadecimal value to search for is invalid. Try again."),QMessageBox::Ok);
            return item;
        }
    } else if (ui->utf8RadioButton->isChecked()) {
        item.setSearchType(FilterItem::UTF8);
        data = searchTerm.toUtf8();
    } else {
        item.setSearchType(FilterItem::UTF16);
        QTextCodec * codec = QTextCodec::codecForName("UTF-16");
        if (codec == nullptr) {
            qCritical() << tr("Could not find the UTF-16 codec T_T");
        } else {
            QTextEncoder *encoder = codec->makeEncoder(QTextCodec::IgnoreHeader | QTextCodec::ConvertInvalidToNull);
            data = encoder->fromUnicode(searchTerm);
            if (encoder->hasFailure()) {
                qWarning() << tr("[FilterDialog::onAddClicked] encoding to UTF-16 failed at some point...");
            }
            delete encoder;
        }
    }

    item.setReverseSelection(ui->reverseCheckBox->isChecked());
    item.setValue(data);
    item.setBitMask(mask);

    return item;
}

bool FilterDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Tab) {
            if (obj == ui->filterNameLineEdit) {
                ui->filterLineEdit->setFocus();
                ui->filterLineEdit->selectAll();
                return true;
            } else if (obj == ui->filterLineEdit) {
                ui->startOffsetLineEdit->setFocus();
                ui->startOffsetLineEdit->selectAll();
                return true;
            }
        }

        if (key->key() == Qt::Key_Return) {
            QModelIndexList indexList = ui->filtersListWidget->selectionModel()->selectedIndexes();
            if (!indexList.isEmpty()) {
                onSave();
                return true;
            } else {
                onAdd();
                return true;
            }
        }
    }

    return QDialog::eventFilter(obj, event);
}

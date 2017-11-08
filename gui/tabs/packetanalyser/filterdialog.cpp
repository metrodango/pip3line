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
#include <QRegExp>
#include "packetsortfilterproxymodel.h"
#include "filterengine.h"
#include "packetmodelabstract.h"

NameValidator::NameValidator(QObject *parent) :
    QValidator(parent)
{

}

NameValidator::~NameValidator()
{

}

QValidator::State NameValidator::validate(QString &input, int &) const
{
    QValidator::State ret = QValidator::Invalid;

    if (FilterItem::nameRegexp.exactMatch(input)) {
        QString temp = input.toUpper();
        if (temp == "NOT" || temp == "OR" || temp == "AND" || temp == "XOR") {
            ret = QValidator::Intermediate;
        } else {
            ret = QValidator::Acceptable;
        }
    }

    return ret;
}


CIDValidator::CIDValidator(QObject *parent) :
    QValidator(parent)
{

}

CIDValidator::~CIDValidator()
{

}

QValidator::State CIDValidator::validate(QString &input, int &) const
{
    QValidator::State ret = QValidator::Acceptable;
    QStringList clist = input.split(FilterCIDs::SEP, QString::SkipEmptyParts);
    for (int i = 0; i < clist.size(); i++) {
        bool ok = false;
        clist.at(i).toInt(&ok);
        if (!ok) {
            ret = QValidator::Invalid;
            break;
        }
    }
    return ret;
}

FilterDialog::FilterDialog(PacketSortFilterProxyModel *sortFilterProxyModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterDialog),
    sortFilterProxyModel(sortFilterProxyModel)
{
    ui->setupUi(this);

    validationTimer.setInterval(300);
    validationTimer.setSingleShot(true);
    filterEngine = sortFilterProxyModel->getFilterEngine();

    setWindowTitle("Configure filters");

    initUi();

    list = filterEngine->getItems();
    updatedList();

    ui->filterNameLineEdit->setValidator(&nameValidator);
    ui->filterNameLineEdit->installEventFilter(this);
    ui->filterLineEdit->installEventFilter(this);
    ui->filtersGroupBox->installEventFilter(this);

    ui->cidLineEdit->setValidator(&cidValidator);

    ui->filterNameLineEdit->setFocus();

    connect(this, SIGNAL(rejected()), SLOT(deleteLater()));
    connect(this, SIGNAL(finished(int)), SLOT(deleteLater()));
    connect(ui->closePushButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->addPushButton, SIGNAL(clicked(bool)), this, SLOT(onAdd()));
    connect(ui->enableCheckBox, SIGNAL(toggled(bool)), this, SLOT(onFilterToggled(bool)));
    connect(ui->filtersListWidget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onItemSelected(QItemSelection)));
    connect(ui->clearPushButton, SIGNAL(clicked(bool)), this, SLOT(onClear()));
    connect(ui->applyPushButton, SIGNAL(clicked(bool)), this, SLOT(onSave()));
    connect(ui->booleanExprLineEdit, SIGNAL(textChanged(QString)), &validationTimer, SLOT(start()));
    connect(&validationTimer, SIGNAL(timeout()), this, SLOT(validate()));
}

FilterDialog::~FilterDialog()
{
  //  qDebug() << "destroying" << this;
    delete ui;
    sortFilterProxyModel = nullptr;
}

void FilterDialog::initUi()
{
    ui->booleanExprLineEdit->setText(filterEngine->getExpressionStr());
    ui->enableCheckBox->setChecked(filterEngine->getFilteringEnabled());

    // columns

    PacketModelAbstract *sourceModel = static_cast<PacketModelAbstract *>(sortFilterProxyModel->sourceModel());
    if (sourceModel != nullptr) {
        QStringList cols = sourceModel->getColumnNames();
        ui->columnComboBox->addItems(cols);

        connect(sourceModel, SIGNAL(columnsUpdated()), this, SLOT(onColumnsUpdated()));
        connect(ui->columnComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onColumnSelected(int)));

        ui->columnComboBox->setCurrentIndex(PacketModelAbstract::COLUMN_PAYLOAD);
        ui->stackedWidget->setCurrentWidget(ui->dataPage);
    } else {
        qCritical() << tr("[FilterDialog::initUi] source model is null T_T");
    }

    //length
    ui->lengthOpComboBox->addItems(FilterLength::OperatorsString);

    ui->lengthOpComboBox->setCurrentIndex(0);

    // direction
    ui->directionComboBox->addItem(QIcon(":/Images/icons/arrow-left-3.png"), "");
    ui->directionComboBox->addItem(QIcon(":/Images/icons/arrow-right-3-mod.png"), "");

    //timestamp
    ui->afterCheckBox->setChecked(true);
    ui->afterDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));
    ui->beforeCheckBox->setChecked(false);
    ui->afterDateTimeEdit->setDateTime(QDateTime::currentDateTime());

    //data
    ui->utf8RadioButton->setChecked(true);
    ui->startOffsetLineEdit->setText("0");

    //text
    ui->regexpTypeComboBox->addItems(FilterText::RegexpTypesStr);
    ui->caseSensitiveCheckBox->setChecked(true);

    ui->applyPushButton->setEnabled(false);
}

void FilterDialog::onAdd()
{
    QString name = ui->filterNameLineEdit->text();
    for (int i = 0 ; i < list.size(); i++) {
        if (list.at(i)->getName() == name) {
            QMessageBox::critical(this,tr("Duplicate filter name"),tr("Another filter already has this name. Choose another one."),QMessageBox::Ok);
            return;
        }
    }

    QSharedPointer<FilterItem> item = getCurrentConfItem();
    if (!item.isNull() && item->isValid()) {
        list.append(item);
        filterEngine->setItems(list);
        updatedList();

        if (list.size() == 1)
            ui->enableCheckBox->setChecked(true);

        ui->filtersListWidget->setCurrentRow(list.size() - 1);
    }
}

void FilterDialog::updatedList()
{
    ui->filtersListWidget->clear();

    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i)->getName());
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

    validate();
}

void FilterDialog::onDeleteItem(QString name)
{
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i)->getName() == name) {
            list.removeAt(i);
            ui->filtersListWidget->clearSelection();
            filterEngine->setItems(list);
            updatedList();
            if (list.size() == 0)
                ui->enableCheckBox->setChecked(false);
            return;// there supposed to be only one ...
        }
    }
    // we should never arrive here
    qCritical() << tr("[FilterDialog::onDeleteItem] The item \"%1\" was not found T_T").arg(name);
}

void FilterDialog::onFilterToggled(bool toggled)
{
    filterEngine->setFilteringEnabled(toggled);
}

void FilterDialog::onItemSelected(QItemSelection selection)
{
    QModelIndexList indexList =  selection.indexes();
    if (!indexList.isEmpty()) {
        int index = indexList.at(0).row();
        if (index >= 0 && index < list.size()) {
            QSharedPointer<FilterItem> item = list.at(index);
            if (item.isNull()) {
                qCritical() << tr("[FilterDialog::onItemSelected] FilterItem is null");
                return;
            }

            ui->filterNameLineEdit->setText(item->getName());
            ui->hexaRadioButton->setChecked(item->isReverseSelection());

            FilterItem::FilterType ft = item->getFilterId();
            if (ft == FilterItem::Data) {
                QSharedPointer<FilterData> ditem = item.staticCast<FilterData>();
                if (ditem.isNull()) {
                    qCritical() << tr("[FilterDialog::onItemSelected] Cast to FilterData failed");
                    return;
                }

                int offset = ditem->getStartingOffset();
                QString off_str = QString::number(offset, 10);
                ui->startOffsetLineEdit->setText(off_str);
                if (ditem->getSearchType() == FilterData::HEXA) {
                    ui->hexaRadioButton->setChecked(true);
                    ui->filterLineEdit->setText(ditem->getHexValueWithMask());
                } else if (ditem->getSearchType() == FilterData::UTF8) {
                    ui->utf8RadioButton->setChecked(true);
                    ui->filterLineEdit->setText(QString::fromUtf8(ditem->getSearchValue()));
                } else {
                    ui->utf16RadioButton->setChecked(true);
                    QTextCodec * codec = QTextCodec::codecForName("UTF-16");
                    if (codec == nullptr) {
                        qCritical() << tr("[FilterDialog::onItemSelected] Could not find the UTF-16 codec T_T");
                    } else {
                        QTextDecoder *decoder = codec->makeDecoder(QTextCodec::IgnoreHeader | QTextCodec::ConvertInvalidToNull);
                        QString textf = decoder->toUnicode(ditem->getSearchValue().constData(),ditem->getSearchValue().size());
                        if (decoder->hasFailure()) {
                            qWarning() << tr("[FilterDialog::onItemSelected] decoding from UTF-16 failed at some point...");
                        }

                        ui->filterNameLineEdit->setText(textf);
                        delete decoder;
                    }
                }

                ui->columnComboBox->setCurrentIndex(PacketModelAbstract::COLUMN_PAYLOAD);
            } else if (ft == FilterItem::Length) {
                QSharedPointer<FilterLength> ditem = item.staticCast<FilterLength>();
                if (!ditem.isNull()) {
                    ui->lengthSpinBox->setValue(ditem->getLengthValue());
                    ui->lengthOpComboBox->setCurrentIndex(ditem->getOp());
                    ui->columnComboBox->setCurrentIndex(PacketModelAbstract::COLUMN_LENGTH);
                } else {
                    qCritical() << tr("[FilterDialog::onItemSelected] Cast to FilterLength failed");
                    return;
                }
            } else if (ft == FilterItem::TimeStamp) {
                QSharedPointer<FilterTimeStamp> ditem = item.staticCast<FilterTimeStamp>();
                if (!ditem.isNull()) {
                    if (ditem->getAfter().isValid()) {
                        ui->afterCheckBox->setChecked(true);
                        ui->afterDateTimeEdit->setDateTime(ditem->getAfter());
                    } else {
                        ui->afterCheckBox->setChecked(false);
                    }

                    if (ditem->getBefore().isValid()) {
                        ui->beforeCheckBox->setChecked(true);
                        ui->beforeDateTimeEdit->setDateTime(ditem->getBefore());
                    } else {
                        ui->beforeCheckBox->setChecked(false);
                    }
                    ui->columnComboBox->setCurrentIndex(PacketModelAbstract::COLUMN_TIMESPTAMP);
                } else {
                    qCritical() << tr("[FilterDialog::onItemSelected] Cast to FilterTimeStamp failed");
                    return;
                }
            }  else if (ft == FilterItem::Direction) {
                QSharedPointer<FilterDirection> ditem = item.staticCast<FilterDirection>();
                if (!ditem.isNull()) {
                    if (ditem->getDirection() != Packet::NODIRECTION)
                        ui->directionComboBox->setCurrentIndex(ditem->getDirection());
                    ui->columnComboBox->setCurrentIndex(PacketModelAbstract::COLUMN_DIRECTION);
                } else {
                    qCritical() << tr("[FilterDialog::onItemSelected] Cast to FilterDirection failed");
                    return;
                }
            }  else if (ft == FilterItem::CID) {
                QSharedPointer<FilterCIDs> ditem = item.staticCast<FilterCIDs>();
                if (!ditem.isNull()) {
                    QList<int> cidList = ditem->getCidList();
                    QStringList flist;
                    for (int i = 0; i < cidList.size(); i++) {
                        flist.append(QString::number(cidList.at(i)));
                    }
                    ui->cidLineEdit->setText(flist.join(FilterCIDs::SEP));
                    ui->columnComboBox->setCurrentIndex(PacketModelAbstract::COLUMN_CID);
                } else {
                    qCritical() << tr("[FilterDialog::onItemSelected] Cast to FilterCIDs failed");
                    return;
                }
            } else { // left with Text
                QSharedPointer<FilterText> ditem = item.staticCast<FilterText>();
                if (!ditem.isNull()) {
                    QRegExp regexp = ditem->getRegexp();
                    ui->regexpLineEdit->setText(regexp.pattern());
                    ui->regexpTypeComboBox->setCurrentIndex(regexp.patternSyntax());
                    ui->caseSensitiveCheckBox->setChecked(regexp.caseSensitivity() == Qt::CaseSensitive);
                    QString colname = ditem->getTargetColumn();
                    if (ui->columnComboBox->findText(colname) >= 0)
                        ui->columnComboBox->setCurrentText(colname);
                    else {
                        qCritical() << tr("[FilterDialog::onItemSelected] Column not found for FilterText, defaulting to Comment");
                        ui->columnComboBox->setCurrentIndex(PacketModelAbstract::COLUMN_COMMENT);
                    }
                } else {
                    qCritical() << tr("[FilterDialog::onItemSelected] Cast to FilterText failed");
                    return;
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
            QSharedPointer<FilterItem> item = getCurrentConfItem();
            if (!item.isNull() && item->isValid()) {
                list.replace(index,item);
                filterEngine->setItems(list);
                updatedList();
                ui->filtersListWidget->blockSignals(false);
                ui->filtersListWidget->selectionModel()->select(indexList.at(0), QItemSelectionModel::Select);
                ui->filtersListWidget->blockSignals(true);
            }
        }
    }
}

void FilterDialog::onColumnsUpdated()
{
    QString current = ui->columnComboBox->currentText();
    PacketModelAbstract *sourceModel = static_cast<PacketModelAbstract *>(sortFilterProxyModel->sourceModel());
    if (sourceModel != nullptr) {
        ui->columnComboBox->clear();
        QStringList cols = sourceModel->getColumnNames();
        ui->columnComboBox->addItems(cols);

        if (cols.contains(current)) {
            ui->columnComboBox->setCurrentIndex(cols.indexOf(current));
        } else {
            ui->columnComboBox->setCurrentIndex(0);
        }
    } else {
        qCritical() << tr("[FilterDialog::onColumnsUpdated] source model is null T_T");
    }


}

void FilterDialog::onColumnSelected(int index)
{
    PacketModelAbstract *sourceModel = static_cast<PacketModelAbstract *>(sortFilterProxyModel->sourceModel());
    if (sourceModel != nullptr) {
        if (index >= 0 && index < sourceModel->columnCount()) {
                if (index == PacketModelAbstract::COLUMN_DIRECTION) {
                    ui->stackedWidget->setCurrentWidget(ui->directionPage);
                } else if (index == PacketModelAbstract::COLUMN_PAYLOAD) {
                    ui->stackedWidget->setCurrentWidget(ui->dataPage);
                } else if (index == PacketModelAbstract::COLUMN_LENGTH) {
                    ui->stackedWidget->setCurrentWidget(ui->lengthPage);
                } else if (index == PacketModelAbstract::COLUMN_TIMESPTAMP) {
                    ui->stackedWidget->setCurrentWidget(ui->timestampPage);
                } else if (index == PacketModelAbstract::COLUMN_CID) {
                    ui->stackedWidget->setCurrentWidget(ui->cidPage);
                } else {
                    ui->stackedWidget->setCurrentWidget(ui->textPage);
                }
        }
    } else {
        qCritical() << tr("[FilterDialog::onColumnSelected] source model is null T_T");
    }
}

void FilterDialog::validate()
{
    ui->errorLabel->setText("");
    if (ui->booleanExprLineEdit->text().isEmpty()) {
        ui->booleanExprLineEdit->setStyleSheet("");
        filterEngine->assert("");
        ui->enableCheckBox->setChecked(true);
    } else if (filterEngine->assert(ui->booleanExprLineEdit->text())) {
        ui->booleanExprLineEdit->setStyleSheet(GuiStyles::LineEditOk);
        ui->enableCheckBox->setChecked(true);
    } else {
        ui->errorLabel->setText(filterEngine->getErrorStr());
        ui->booleanExprLineEdit->setStatusTip(filterEngine->getErrorStr());
        ui->booleanExprLineEdit->setStyleSheet(GuiStyles::LineEditError);
    }
}

QSharedPointer<FilterItem> FilterDialog::getCurrentConfItem()
{
    QSharedPointer<FilterItem> item;
    QString name;
    if (ui->filterNameLineEdit->hasAcceptableInput()) {
        name = ui->filterNameLineEdit->text();
    }
    else {
        QMessageBox::critical(this,tr("Invalid filter name"),tr("The name field has to follow the following regexp : ^[a-z0-9]{1,20}$ and not be a boolean operator (or,and,xor,not)"),QMessageBox::Ok);
        return item;
    }

    if (name.isEmpty()) {
        QMessageBox::critical(this,tr("Empty filter name"),tr("The name field is empty"),QMessageBox::Ok);
        return item;
    }



    int colSelected = ui->columnComboBox->currentIndex();

    if (colSelected == PacketModelAbstract::COLUMN_PAYLOAD) {
        QString searchTerm = ui->filterLineEdit->text();

        if (searchTerm.isEmpty()) {
            QMessageBox::critical(this,tr("Empty filter"),tr("The search term is empty"),QMessageBox::Ok);
            return item;
        }

        QSharedPointer<FilterData> ditem;
        ditem = QSharedPointer<FilterData>(new(std::nothrow ) FilterData());


        bool ok = false;
        int offset = ui->startOffsetLineEdit->text().toInt(&ok,10);
        if (ok && offset >= 0)
            ditem->setStartingOffset(offset);

        QBitArray mask;
        QByteArray data;
        if (ui->hexaRadioButton->isChecked()) {
            ditem->setSearchType(FilterData::HEXA);
            data = GuiConst::extractSearchHexa(searchTerm, mask);
            if (data.isEmpty()) {
                QMessageBox::critical(this,tr("Invalid Hexadecimal value"),tr("The hexadecimal value to search for is invalid. Try again."),QMessageBox::Ok);
                return item;
            }
        } else if (ui->utf8RadioButton->isChecked()) {
            ditem->setSearchType(FilterData::UTF8);
            data = searchTerm.toUtf8();
        } else {
            ditem->setSearchType(FilterData::UTF16);
            QTextCodec * codec = QTextCodec::codecForName("UTF-16");
            if (codec == nullptr) {
                qCritical() << tr("[FilterDialog::onAddClicked] Could not find the UTF-16 codec T_T");
            } else {
                QTextEncoder *encoder = codec->makeEncoder(QTextCodec::IgnoreHeader | QTextCodec::ConvertInvalidToNull);
                data = encoder->fromUnicode(searchTerm);
                if (encoder->hasFailure()) {
                    qWarning() << tr("[FilterDialog::onAddClicked] encoding to UTF-16 failed at some point...");
                }
                delete encoder;
            }
        }

        ditem->setSearchValue(data);
        ditem->setBitMask(mask);
        item = ditem;

    } else if (colSelected == PacketModelAbstract::COLUMN_DIRECTION) {
        QSharedPointer<FilterDirection> ditem = QSharedPointer<FilterDirection>(new(std::nothrow ) FilterDirection());
        ditem->setDirection((Packet::Direction)ui->directionComboBox->currentIndex());
        item = ditem;
    } else if (colSelected == PacketModelAbstract::COLUMN_LENGTH) {
        QSharedPointer<FilterLength> ditem = QSharedPointer<FilterLength>(new(std::nothrow ) FilterLength());
        ditem->setLengthValue(ui->lengthSpinBox->value());
        ditem->setOp((FilterLength::Operators)ui->lengthOpComboBox->currentIndex());
        item = ditem;
    } else if (colSelected == PacketModelAbstract::COLUMN_CID) {
        QSharedPointer<FilterCIDs> ditem = QSharedPointer<FilterCIDs>(new(std::nothrow ) FilterCIDs());
        QStringList clist = ui->cidLineEdit->text().split(FilterCIDs::SEP, QString::SkipEmptyParts);
        QList<int> cilist;
        for (int i = 0; i < clist.size(); i++) {
            bool ok = false;
            int val = clist.at(i).toInt(&ok);
            if (ok)
                cilist.append(val);
        }
        ditem->setCidList(cilist);
        item = ditem;
    } else if (colSelected == PacketModelAbstract::COLUMN_TIMESPTAMP) {
        QSharedPointer<FilterTimeStamp> ditem = QSharedPointer<FilterTimeStamp>(new(std::nothrow ) FilterTimeStamp());
        if (ui->afterCheckBox->isChecked()) {
            ditem->setAfter(ui->afterDateTimeEdit->dateTime());
        } else {
            ditem->setAfter(QDateTime());
        }

        if (ui->beforeCheckBox->isChecked()) {
            ditem->setBefore(ui->beforeDateTimeEdit->dateTime());
        } else {
            ditem->setBefore(QDateTime());
        }
        item = ditem;
    } else {
        QSharedPointer<FilterText> ditem = QSharedPointer<FilterText>(new(std::nothrow ) FilterText());

        QRegExp regexp(ui->regexpLineEdit->text());
        regexp.setCaseSensitivity(ui->caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        regexp.setPatternSyntax((QRegExp::PatternSyntax) ui->regexpTypeComboBox->currentIndex());

        ditem->setRegexp(regexp);
        ditem->setTargetColumn(ui->columnComboBox->currentText());
        item = ditem;
    }

    if (!item.isNull()) {
        item->setReverseSelection(ui->reverseCheckBox->isChecked());
        item->setName(name);
    }

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

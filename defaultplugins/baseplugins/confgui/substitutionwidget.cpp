/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "substitutionwidget.h"
#include "ui_substitutionwidget.h"
#include <QTableView>
#include <QClipboard>
#include <QDebug>
#include <QMessageBox>

const int SubstitutionTables::S_TABLE_SIZE = 16;
const int SubstitutionTables::S_ARRAY_SIZE = 256;
const QByteArray SubstitutionWidget::HEXCHAR("abcdefABCDEF1234567890");

SubstitutionWidget::SubstitutionWidget(Substitution *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::SubstitutionWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::SubstitutionWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    sModel.setRawData(transform->getSTable());
    ui->sTableTableView->setModel(&sModel);
    ui->sTableTableView->resizeColumnsToContents();

    ui->comboBox->addItems(Substitution::knownTables);

    connect(&sModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onModify()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(QString)), this, SLOT(onChooseTable(QString)));
}

SubstitutionWidget::~SubstitutionWidget()
{
    delete ui;
}

void SubstitutionWidget::onModify()
{
    transform->setSTable(sModel.getRawData());
}

void SubstitutionWidget::onChooseTable(QString name)
{
    sModel.setRawData(transform->getPredeterminedTable(name));
}

void SubstitutionWidget::on_exportPushButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    QByteArray val = sModel.getRawData();

    QByteArray final;
    final.append("{");
    for (int i = 0; i < val.size(); i++) {
        final.append(" 0x").append(QByteArray(1,val.at(i)).toHex()).append(",");
    }
    final.chop(1);
    final.append(" }");

    clipboard->setText(final);
}

void SubstitutionWidget::on_importPushButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    QByteArray val = clipboard->text().toUtf8();

    QByteArray temp;
    int i = 0;
    while (i < val.size()) {
        if (val.at(i) == '0' && i < val.size() - 3 &&  val.at(i+1) == 'x') {
            if (HEXCHAR.contains(val.at(i+2)) && HEXCHAR.contains(val.at(i+3))) {
                temp.append(val.at(i+2)).append(val.at(i+3));
                i += 4;
                continue;
            }
        }
        i++;
    }

    temp = QByteArray::fromHex(temp);

    if (temp.size() < SubstitutionTables::S_ARRAY_SIZE) {
        QMessageBox::warning(this,tr("Imported table too sort"),tr("Permutation table too short (%1 intead of 256). the array will be completed with zeros.").arg(temp.size()));
    } else if (temp.size() > SubstitutionTables::S_ARRAY_SIZE) {
        QMessageBox::warning(this,tr("Imported table too long"),(tr("Permutation table too long (%1 instead of 256) - The array will be truncated.").arg(temp.size())));
    }
    sModel.setRawData(temp);

}

/* Model for the substitutions tables */

SubstitutionTables::SubstitutionTables(): highlight(S_ARRAY_SIZE)
{
    for (quint16 i = 0; i < S_ARRAY_SIZE; i++) {
        array.append((char)i);
        highlight[i] = false;
    }
}

SubstitutionTables::~SubstitutionTables()
{
}

int SubstitutionTables::columnCount(const QModelIndex & /* unused */) const
{
    return S_TABLE_SIZE;
}

int SubstitutionTables::rowCount(const QModelIndex & /* unused */) const
{
    return S_TABLE_SIZE;
}

QVariant SubstitutionTables::data(const QModelIndex &index, int role) const
{
    int pos = S_TABLE_SIZE * index.row() + index.column();
    switch (role)
    {
        case Qt::DisplayRole:
        {
            if (pos >= 0 && pos < S_ARRAY_SIZE)
                return QString::fromUtf8(QByteArray(1,array[pos]).toHex());
        }
            break;
        case Qt::BackgroundRole:
        {
            if (highlight[pos])
                return QVariant(QColor(Qt::red));
            else if ((index.column() / 4) % 2 == 0)
                return QVariant(QColor(Qt::white));
            else if ((index.column() / 4) % 2 == 1)
                return QVariant(QColor(243,243,243,255));
            else
                return QVariant(QColor(Qt::white));
        }
            break;
    }
    return QVariant();
}

bool SubstitutionTables::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        QByteArray hexVal = QByteArray::fromHex(value.toByteArray());

        quint32 pos = S_TABLE_SIZE * index.row() + index.column();
        if (pos <  (quint32)S_ARRAY_SIZE) {
            if (hexVal.isEmpty())
                array[pos] = (char)pos;
            else
                array[pos] = hexVal.at(0);
            highlighting();
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

QVariant SubstitutionTables::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section < S_TABLE_SIZE)
            return QString("x%1").arg(section,2,S_TABLE_SIZE,QChar('0'));
        else
            return QVariant();
    } else {
        if (section < S_TABLE_SIZE)
            return QString("x%1").arg(section * S_TABLE_SIZE,2,S_TABLE_SIZE,QChar('0'));
        else
            return QVariant();
    }
}

Qt::ItemFlags SubstitutionTables::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() >= S_TABLE_SIZE)
        return Qt::ItemIsEnabled;

    if (index.row() >= S_TABLE_SIZE)
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

void SubstitutionTables::setRawData(QByteArray sTable)
{
    beginResetModel();
    array = sTable;

    highlighting();
    endResetModel();
}

QByteArray SubstitutionTables::getRawData()
{
    return array;
}

void SubstitutionTables::highlighting()
{
    highlight.fill(false);
    for (int i = 0; i < S_ARRAY_SIZE - 1; i++) {
        int d = array.indexOf(array[i],i + 1);
        if (d != -1) {
            highlight[d] = true;
            highlight[i] = true;
        }
    }
}

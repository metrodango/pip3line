/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "moduletransformwidget.h"
#include "ui_moduletransformwidget.h"
#include "modulesmanagement.h"
#include <QFileDialog>
#include <QStringList>
#include <QKeyEvent>
#include <QHashIterator>
#include <QMessageBox>
#include <QMenu>
#include <QModelIndex>
#include "scripttransformabstract.h"
#include <QDebug>

const QString ModuleTransformWidget::MENU_DELETE = QObject::tr("Delete");

ModuleTransformWidget::ModuleTransformWidget(ScriptTransformAbstract *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ModuleTransformWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::ModuleTransformWidget X{");
    }
    transform = ntransform;
    tableMenu = NULL;
    reloadingParams = false;
    ui->setupUi(this);

#if QT_VERSION >= 0x050000
    ui->parameterstableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->parameterstableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
    ui->parameterstableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->parameterstableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
#endif

    ui->parameterstableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->parameterstableView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

    model  = new(std::nothrow) ParametersItemModel();
    if (model == NULL) {
        qFatal("Cannot allocate memory for ParametersItemModel X{");
    }
    QAbstractItemModel * oldModel = ui->parameterstableView->model();
    ui->parameterstableView->setModel(model);
    delete oldModel;

    ui->autoReloadCheckBox->setChecked(transform->isAutoReload());
    // for now
    //ui->autoReloadCheckBox->setVisible(false);

    connect(model, SIGNAL(parametersChanged()), this, SLOT(onParametersUpdated()));
    connect(transform, SIGNAL(confUpdated()), this, SLOT(reloadConf()));

    ui->parameterstableView->installEventFilter(this);
    ui->fileLineEdit->setText(transform->getModuleFileName());

    if (transform->getType() == ModulesManagement::PERSISTENT) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->makePersistentCheckBox->setChecked(true);
    } else if (transform->getType() == ModulesManagement::AUTO) {
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
        ui->makePersistentCheckBox->setChecked(false);
    }
    ui->tabWidget->setTabText(0,tr("%1 script").arg(transform->getScriptDescr()));

    connect(ui->choosePushButton, SIGNAL(clicked()), this, SLOT(onChooseFile()));
    connect(ui->makePersistentCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMakePersistent(bool)));
    connect(ui->addParamPushButton, SIGNAL(clicked()), this, SLOT(onAddParameter()));
    connect(ui->autoReloadCheckBox, SIGNAL(toggled(bool)), SLOT(onAutoReload(bool)));
    connect(ui->forceReloadPushButton, SIGNAL(clicked()), transform,SLOT(loadModule()));

}

ModuleTransformWidget::~ModuleTransformWidget()
{
    delete tableMenu;
    delete ui;
}

void ModuleTransformWidget::onChooseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose %1 module file").arg(transform->getScriptDescr()),"", tr("Python files (*.py)"));
    if (!fileName.isEmpty()) {
        if (transform->setModuleFile(fileName)) {
            ui->fileLineEdit->setText(fileName);
        }
    }
}

void ModuleTransformWidget::onMakePersistent(bool checked)
{
    if (checked)
        transform->setType(ModulesManagement::PERSISTENT);
    else
        transform->setType(ModulesManagement::TRANSIENT);
}

void ModuleTransformWidget::onAddParameter()
{
    model->addBlankRow();
}

void ModuleTransformWidget::onParametersUpdated()
{
    transform->setParameters(model->getParameters());
}

void ModuleTransformWidget::onAutoReload(bool val)
{
    transform->setAutoReload(val);
}

void ModuleTransformWidget::customMenuRequested(QPoint pos)
{
    QModelIndex index= ui->parameterstableView->indexAt(pos);
    if (index.isValid()) {
        if (tableMenu == NULL) {
        tableMenu = new QMenu(this);
        tableMenu->addAction(new QAction(MENU_DELETE, tableMenu));
        connect(tableMenu, SIGNAL(triggered(QAction*)), SLOT(onMenuAction(QAction*)));
        }
        tableMenu->popup(ui->parameterstableView->mapToGlobal(pos));
    }
}

void ModuleTransformWidget::onMenuAction(QAction *action)
{
    if (action->text().compare(MENU_DELETE) == 0) {
        QModelIndexList list = ui->parameterstableView->selectionModel()->selectedRows();
        if (list.size() >= 1) {
            model->removeRows(list.at(0).row(),1, list.at(0).parent());
        }
    }
}

void ModuleTransformWidget::reloadConf()
{
    model->setParameters(transform->getParameters());
}

bool ModuleTransformWidget::eventFilter(QObject *sender, QEvent *event)
{
    if (sender == ui->parameterstableView && event->type() == QEvent::KeyPress) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            QModelIndexList list = ui->parameterstableView->selectionModel()->selectedRows();
            for (int i = 0; i < list.size(); i++){
                model->removeRows(list.at(i).row(),1, list.at(0).parent());
            }
        }
    }
    return false; // never, ever, ever return true
}

ParametersItemModel::ParametersItemModel(QObject *parent):
    QAbstractTableModel(parent)
{

}

ParametersItemModel::~ParametersItemModel()
{

}

int ParametersItemModel::columnCount(const QModelIndex &) const
{
    return 2;
}

int ParametersItemModel::rowCount(const QModelIndex &) const
{
    return parametersNames.size();
}

QVariant ParametersItemModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= parametersNames.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.row() < parametersNames.size()) {
            if (index.column() == 0) {
                return parametersNames.at(index.row());
            } else if (index.row() < parametersValues.size()) {
                return parametersValues.at(index.row());
            }
        }
    }

    return QVariant();
}

QVariant ParametersItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return section == 0 ? QString("Name") : QString("Value");
        }
    } else if (orientation == Qt::Vertical) {
        if (role == Qt::DisplayRole) {
            return QString::number(section);
        }
    }

    return QVariant();
}

Qt::ItemFlags ParametersItemModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool ParametersItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= parametersNames.size()) {
        return false;
    }

    if (role == Qt::EditRole) {
        if (index.column() == 0) {
            if (parametersNames.contains(value.toString())) {
                QMessageBox::critical(NULL,tr("Error"), tr("Parameter name already exists"),QMessageBox::Ok);
                return false;
            }
            if (parametersNames.at(index.row()) != value.toString()) {
                parametersNames.replace(index.row(), value.toString());
                emit parametersChanged();
                return true;
            }
        } else {
            if (parametersValues.at(index.row()) != value.toString()) {
                parametersValues.replace(index.row(), value.toString());
                emit parametersChanged();
                return true;
            }
        }
    }

    return false;
}

void ParametersItemModel::addBlankRow()
{
    QString empty;
    if (parametersNames.contains(empty)) {
        QMessageBox::critical(NULL,tr("Error"), tr("Empty parameter is available, use this one first"),QMessageBox::Ok);
        return;
    }
    beginResetModel();

    parametersNames.append(empty);
    parametersValues.append(empty);
    endResetModel();
}

bool ParametersItemModel::removeRows(int row, int count, const QModelIndex & parent)
{
    if (row >= parametersNames.size() // valid row ?
            || count < 1 // valid row count ?
            || parametersNames.size() - count < row) // no overflow ??
    {
            return false;
    }

    beginRemoveRows(parent,row, row + count - 1);

    for (int i = row; i < row + count; i++) {
        parametersNames.removeAt(i);
        parametersValues.removeAt(i);
    }

    endRemoveRows();

    return true;
}

void ParametersItemModel::setParameters(QHash<QByteArray, QByteArray> newParameters)
{
    beginResetModel();
    parametersNames.clear();
    parametersValues.clear();
    QHashIterator<QByteArray, QByteArray> i(newParameters);
     while (i.hasNext()) {
         i.next();
         parametersNames.append(QString::fromUtf8(i.key()));
         parametersValues.append(QString::fromUtf8(i.value()));
     }
     endResetModel();
}

QHash<QByteArray, QByteArray> ParametersItemModel::getParameters()
{

    QHash<QByteArray, QByteArray> finalParams;
    for (int i = 0; i < parametersNames.size(); i++) {
        finalParams.insert(parametersNames.at(i).toUtf8(), parametersValues.at(i).toUtf8());
    }

    return finalParams;
}

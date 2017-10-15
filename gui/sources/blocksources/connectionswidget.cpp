#include "connectionswidget.h"
#include "ui_connectionswidget.h"
#include <QDebug>

const int TargetModel::COLUMN_COUNT = 4;
const QStringList TargetModel::headersList = QStringList()
                            << "Created"
                            << "BlockSource"
                            << "CID"
                            << "Description";

TargetModel::TargetModel(BlocksSource *source, QObject *parent) :
    QAbstractTableModel(parent),
    source(source)
{
    targetList = source->getAvailableConnections();
}

TargetModel::~TargetModel()
{

}

int TargetModel::columnCount(const QModelIndex &) const
{
    return COLUMN_COUNT;
}

int TargetModel::rowCount(const QModelIndex &) const
{
    return targetList.size();
}

QVariant TargetModel::data(const QModelIndex &index, int role) const
{
    int column = index.column();
    int row = index.row();

    if (row < targetList.size() && role == Qt::DisplayRole) {
        switch (column) {
            case 0:
                return targetList.at(row).getTimestamp().toString();
            case 1:
                return tr("0x%1").arg(QString::number((quintptr)targetList.at(row).getSource(), 16));
            case 2:
                return QString::number(targetList.at(row).getConnectionID());
            case 3:
                return targetList.at(row).getDescription();
        }
    }

    return QVariant();
}

QVariant TargetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return headersList.at(section);
        } else {
            return QString("%1").arg(section);
        }
    }

    return QVariant();
}

void TargetModel::setConnections(QList<Target<BlocksSource *> > connections)
{
    beginResetModel();
    targetList = connections;
    endResetModel();
}

int TargetModel::size()
{
    return targetList.size();
}

ConnectionsWidget::ConnectionsWidget(BlocksSource * source, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionsWidget),
    source(source)
{
    ui->setupUi(this);
    QHeaderView * hHeaders = ui->connectionTableView->horizontalHeader();
#if QT_VERSION >= 0x050000
    hHeaders->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    hHeaders->setResizeMode(QHeaderView::ResizeToContents);
#endif
    hHeaders->setStretchLastSection(true);

    ui->connectionTableView->verticalHeader()->hide();

    model = new(std::nothrow) TargetModel(source);
    if (model == nullptr) {
        qFatal("Cannot allocate TargetModel");
    }

    QAbstractItemModel * omodel = ui->connectionTableView->model();
    ui->connectionTableView->setModel(model);
    delete omodel;

    connect(source,SIGNAL(updated()), this, SLOT(connectionsUpdated()));
    setVisible(model->size() > 0);
}

ConnectionsWidget::~ConnectionsWidget()
{
    delete model; // the view is supposed to take care of that ...
    delete ui;
}

void ConnectionsWidget::connectionsUpdated()
{
    model->setConnections(source->getAvailableConnections());
    ui->statusLabel->setText(tr("Connections: %1").arg(model->size()));
}

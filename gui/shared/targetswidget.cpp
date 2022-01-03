#include "targetswidget.h"
#include "ui_targetswidget.h"
#include <QAbstractItemModel>
#include <QDebug>

class QAbstractItemModel;

TargetsWidget::TargetsWidget(QAbstractItemModel *model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TargetsWidget)
{
    ui->setupUi(this);
    QAbstractItemModel *oldmodel = ui->targetsView->model();
    ui->targetsView->setModel(model);
    delete oldmodel;
    ui->targetsView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->targetsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->targetsView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->targetsView->setSelectionBehavior(QAbstractItemView::SelectRows);
}


TargetsWidget::~TargetsWidget()
{
    delete ui;
}

void TargetsWidget::onSendData(const QByteArray &data)
{
    qDebug() << "Data to send: " << QString::fromUtf8(data);
}

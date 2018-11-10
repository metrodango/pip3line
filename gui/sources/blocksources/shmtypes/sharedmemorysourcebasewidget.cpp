#include "sharedmemorysourcebasewidget.h"
#include "sources/blocksources/sharedmemorysource.h"
#include "ui_sharedmemorysourcebasewidget.h"
#include <QDebug>
#include <QStandardItem>

SharedMemorySourceBaseWidget::SharedMemorySourceBaseWidget(SharedMemorySource *source, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SharedMemorySourceBaseWidget),
    source(source)
{
    ui->setupUi(this);

    QString tempLabel = tr("Choose type");
    ui->shmTypescomboBox->addItem(tempLabel);

    // Set the flag of the item within the combobox model
    QStandardItem * item = qobject_cast<QStandardItemModel *>(ui->shmTypescomboBox->model())->item(0);
    item->setEnabled( false );

    QStringList list = SharedMemorySource::getAvailableSHMTypes();
    ui->shmTypescomboBox->addItems(list);

    int t = source->getShmType();

    if (t > 0 && t <= list.size()) {
        qDebug() << tr("setting type to %1").arg(t);
        ui->shmTypescomboBox->setCurrentIndex(t);
    }

    connect(ui->shmTypescomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypechanged(int)));
}

SharedMemorySourceBaseWidget::~SharedMemorySourceBaseWidget()
{
    delete ui;
}

void SharedMemorySourceBaseWidget::addWidget(QWidget *gui)
{
    ui->verticalLayout->addWidget(gui);
}

void SharedMemorySourceBaseWidget::onTypechanged(int index)
{
    source->setShmType(index);
}

#include "filesourcewidget.h"
#include "ui_filesourcewidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QSpinBox>

#include <hexwidget.h>

#include "shared/guiconst.h"
#include "fileblocksource.h"

FileSourceWidget::FileSourceWidget(FileBlockSource *source, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileSourceWidget),
    source(source)
{
    ui->setupUi(this);

    ui->delaySpinBox->setValue(source->getReadDelay());
    if (source->getSourcefiletype() == FileBlockSource::Reader) {
        ui->optionsStackedWidget->setCurrentWidget(ui->readPage);
    } else {
        ui->optionsStackedWidget->setCurrentWidget(ui->writePage);
    }

    ui->delaySpinBox->setMinimum(FileBlockSource::MIN_DELAY);
    ui->delaySpinBox->setMaximum(FileBlockSource::MAX_DELAY);

    ui->nameLineEdit->setText(source->getFilename());

    connect(ui->fileChoosePushButton, &QPushButton::clicked, this, &FileSourceWidget::onChooseFile);
    //connect(ui->delaySpinBox, qOverload<int>(&QSpinBox::valueChanged),this, &FileSourceWidget::onDelayChanged);
    connect(ui->delaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(onDelayChanged(int)));
}

FileSourceWidget::~FileSourceWidget()
{
    delete ui;
}

void FileSourceWidget::onChooseFile()
{
    QString fileName;
    if (source->getSourcefiletype() == FileBlockSource::Writer) {
        fileName = QFileDialog::getSaveFileName(this,tr("Choose file to save to"),GuiConst::GLOBAL_LAST_PATH, tr("all (*)"));
    } else {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"),GuiConst::GLOBAL_LAST_PATH, tr("all (*)"));
    }
    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
        ui->nameLineEdit->setText(fileName);
        source->setFilename(fileName);
    }
}

void FileSourceWidget::onDelayChanged(int value)
{
    source->setReadDelay(value);
}

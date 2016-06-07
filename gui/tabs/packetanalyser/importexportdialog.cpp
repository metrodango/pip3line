#include "importexportdialog.h"
#include "ui_importexportdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

ImportExportDialog::ImportExportDialog(GuiConst::FileOperations type, bool hasSelection, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportExportDialog),
    type(type)
{
    ui->setupUi(this);
    setModal(true);

    ui->allPacketsRadioButton->setChecked(true);

#if QT_VERSION >= 0x050000
    ui->jsonRadioButton->setEnabled(true);
#else
    ui->jsonRadioButton->setEnabled(false);
    ui->jsonRadioButton->setToolTip(tr("JSON format not supported with Qt 4"));
#endif

    if (type == GuiConst::EXPORT_OPERATION) {
        setWindowTitle("Exporting");
        ui->optionsStackedWidget->setVisible(true);
        ui->packetsOptionsGroupBox->setVisible(true);
        ui->selectionOnlyRadioButton->setEnabled(hasSelection);
    } else {
        setWindowTitle("Importing");
        ui->optionsStackedWidget->setVisible(false);
        ui->packetsOptionsGroupBox->setVisible(false);
    }

    connect(ui->fileNamePushButton, SIGNAL(clicked(bool)), SLOT(onChooseFileName()));
    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(onAccept()));

    connect(ui->opPacketRadioButton, SIGNAL(toggled(bool)),this, SLOT(onOpTypeToggled(bool)));
}

ImportExportDialog::~ImportExportDialog()
{
    delete ui;
}

bool ImportExportDialog::opGuiConfSelected()
{
    return ui->opGuiConfRadioButton->isChecked();
}

QString ImportExportDialog::getFileName() const
{
    return ui->filenameLineEdit->text();
}

GuiConst::FileFormat ImportExportDialog::getFormat() const
{
    if (ui->pcapRadioButton->isChecked()) return GuiConst::PCAP_FORMAT;
    if (ui->jsonRadioButton->isChecked()) return GuiConst::JSON_FORMAT;
    if (ui->xmlRadioButton->isChecked()) return GuiConst::XML_FORMAT;

    return GuiConst::INVALID_FORMAT;
}

bool ImportExportDialog::getSelectionOnly() const
{
    return ui->selectionOnlyRadioButton->isChecked();
}

void ImportExportDialog::onChooseFileName()
{
    QString fileName;
    if (type == GuiConst::EXPORT_OPERATION) {
        fileName = QFileDialog::getSaveFileName(this,tr("Choose file to save to"),QDir::home().absolutePath(), tr("all (*)"));
    } else {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"),QDir::home().absolutePath(), tr("all (*)"));
    }
    if (!fileName.isEmpty()) {
        ui->filenameLineEdit->setText(fileName);
    }
}

void ImportExportDialog::onAccept()
{
    if (ui->filenameLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Need a file"),tr("Need a file name to perform the action"), QMessageBox::Ok);
    } else {
        accept();
    }
}

void ImportExportDialog::onOpTypeToggled(bool enable)
{
    ui->formatGroupBox->setEnabled(enable);
    ui->packetsOptionsGroupBox->setEnabled(enable);
}

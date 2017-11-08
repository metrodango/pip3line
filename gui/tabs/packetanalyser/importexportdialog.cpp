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

    ui->jsonRadioButton->setEnabled(true);

    if (type == GuiConst::EXPORT_OPERATION) {
        setWindowTitle("Exporting");
        ui->packetsOptionsGroupBox->setVisible(true);
        ui->selectionOnlyRadioButton->setEnabled(hasSelection);
        ui->plainWarningLabel->setVisible(false);
    } else {
        setWindowTitle("Importing");
        ui->packetsOptionsGroupBox->setVisible(false);
        if (ui->pcapRadioButton->isChecked()) {
            ui->compressCheckBox->setVisible(false);
        }
        ui->plainDestGroupBox->setTitle("Source");
        ui->plainWarningLabel->setVisible(true);
    }

    ui->optionsStackedWidget->setVisible(false); // only used by the Plain format right now

    ui->toFileRadioButton->setChecked(true);
    ui->base64CheckBox->setChecked(true);

    ui->compressCheckBox->setChecked(true);
    connect(ui->fileNamePushButton, SIGNAL(clicked(bool)), SLOT(onChooseFileName()));
    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(onAccept()));

    connect(ui->opPacketRadioButton, SIGNAL(toggled(bool)),this, SLOT(onOpTypeToggled(bool)));
    connect(ui->pcapRadioButton, SIGNAL(toggled(bool)), this, SLOT(onPcapToggled(bool)));
    connect(ui->plainRadioButton, SIGNAL(toggled(bool)), this, SLOT(onPlainToggled(bool)));
    connect(ui->xmlRadioButton, SIGNAL(toggled(bool)), this, SLOT(onXmlorJsonToggled(bool)));
    connect(ui->jsonRadioButton, SIGNAL(toggled(bool)), this, SLOT(onXmlorJsonToggled(bool)));
    connect(ui->toClipboardRadioButton, SIGNAL(toggled(bool)), this, SLOT(onToClipboardToggled(bool)));
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
    if (ui->plainRadioButton->isChecked()) return GuiConst::PLAIN_FORMAT;

    return GuiConst::INVALID_FORMAT;
}

bool ImportExportDialog::isPlainBase64() const
{
    return ui->base64CheckBox->isChecked();
}

bool ImportExportDialog::isPlainFile() const
{
    return ui->toFileRadioButton->isChecked();
}

bool ImportExportDialog::getSelectionOnly() const
{
    return ui->selectionOnlyRadioButton->isChecked();
}

bool ImportExportDialog::getEnableCompression() const
{
    return ui->compressCheckBox->isChecked();
}

void ImportExportDialog::onChooseFileName()
{
    QString fileName;
    if (type == GuiConst::EXPORT_OPERATION) {
        fileName = QFileDialog::getSaveFileName(this,tr("Choose file to save to"),GuiConst::GLOBAL_LAST_PATH, tr("all (*)"));
    } else {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"),GuiConst::GLOBAL_LAST_PATH, tr("all (*)"));
    }
    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
        ui->filenameLineEdit->setText(fileName);
    }
}

void ImportExportDialog::onAccept()
{
    if (!(getFormat() == GuiConst::PLAIN_FORMAT && !isPlainFile()) && ui->filenameLineEdit->text().isEmpty()) {
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

void ImportExportDialog::onPcapToggled(bool enable)
{
    if (enable) {
        ui->compressCheckBox->setVisible(false);
        ui->optionsStackedWidget->setVisible(false);
        ui->filenameWidget->setEnabled(true);
    }
}

void ImportExportDialog::onPlainToggled(bool enable)
{
    if (enable) {
        ui->compressCheckBox->setVisible(false);
        ui->optionsStackedWidget->setVisible(true);
        ui->optionsStackedWidget->setCurrentWidget(ui->plainOptionsPage);
    }
}

void ImportExportDialog::onXmlorJsonToggled(bool enable)
{
    if (enable) {
        ui->compressCheckBox->setVisible(true);
        ui->optionsStackedWidget->setVisible(false);
        ui->filenameWidget->setEnabled(true);
    }
}

void ImportExportDialog::onToClipboardToggled(bool enable)
{
    if (enable) { // clipboard
        ui->filenameWidget->setEnabled(false);
    } else { // file
        ui->filenameWidget->setEnabled(true);
    }
}

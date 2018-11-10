#ifndef IMPORTEXPORTDIALOG_H
#define IMPORTEXPORTDIALOG_H

#include <QDialog>
#include "shared/guiconst.h"

namespace Ui {
class ImportExportDialog;
}

class ImportExportDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit ImportExportDialog(GuiConst::FileOperations type, bool hasSelection = false, QWidget *parent = nullptr);
        ~ImportExportDialog();
        QString getFileName() const;
        GuiConst::FileFormat getFormat() const;
        bool isPlainBase64() const;
        bool isPlainFile() const;
        bool getSelectionOnly() const;
        bool getEnableCompression() const;
        bool opGuiConfSelected();

    private slots:
        void onChooseFileName();
        void onAccept();
        void onOpTypeToggled(bool enable);
        void onPcapToggled(bool enable);
        void onPlainToggled(bool enable);
        void onXmlorJsonToggled(bool enable);
        void onToClipboardToggled(bool enable);
    private:
        Q_DISABLE_COPY(ImportExportDialog)
        Ui::ImportExportDialog *ui;
        GuiConst::FileOperations type;
};

#endif // IMPORTEXPORTDIALOG_H

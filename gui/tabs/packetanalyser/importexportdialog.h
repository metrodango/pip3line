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
        explicit ImportExportDialog(GuiConst::FileOperations type, bool hasSelection = false, QWidget *parent = 0);
        ~ImportExportDialog();
        QString getFileName() const;
        GuiConst::FileFormat getFormat() const;
        bool getSelectionOnly() const;

    private slots:
        void onChooseFileName();
        void onAccept();
    private:
        Ui::ImportExportDialog *ui;
        GuiConst::FileOperations type;
};

#endif // IMPORTEXPORTDIALOG_H

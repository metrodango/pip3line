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
        bool opGuiConfSelected();

    private slots:
        void onChooseFileName();
        void onAccept();
        void onOpTypeToggled(bool enable);
    private:
        Ui::ImportExportDialog *ui;
        GuiConst::FileOperations type;
};

#endif // IMPORTEXPORTDIALOG_H

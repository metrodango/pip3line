#ifndef FILESOURCEWIDGET_H
#define FILESOURCEWIDGET_H

#include <QWidget>

namespace Ui {
    class FileSourceWidget;
}

class FileBlockSource;

class FileSourceWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit FileSourceWidget(FileBlockSource *source, QWidget *parent = nullptr);
        ~FileSourceWidget() override;
    private slots:
        void onChooseFile();
        void onDelayChanged(int value);
    private:
        Ui::FileSourceWidget *ui;
        FileBlockSource *source;
};

#endif // FILESOURCEWIDGET_H

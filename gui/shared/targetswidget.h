#ifndef TARGETSWIDGET_H
#define TARGETSWIDGET_H

#include <QWidget>
#include <QAbstractItemModel>

namespace Ui {
    class TargetsWidget;
}

class TargetsWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit TargetsWidget(QAbstractItemModel *model, QWidget *parent = nullptr);
        ~TargetsWidget();
    public slots:
        void onSendData(const QByteArray &data);
    private:
        Ui::TargetsWidget *ui;
};

#endif // TARGETSWIDGET_

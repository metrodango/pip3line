#ifndef JSONVALUEWIDGET_H
#define JSONVALUEWIDGET_H

#include <QWidget>
#include "../jsonvalue.h"

namespace Ui {
    class JsonValueWidget;
}

class JsonValueWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit JsonValueWidget(JsonValue *transform, QWidget *parent = nullptr);
        ~JsonValueWidget();
    private slots:
        void onNameChanged(const QString & name);
        void onCompactToggled(bool checked);
    private:
        Ui::JsonValueWidget *ui;
        JsonValue *transform;
};

#endif // JSONVALUEWIDGET_H

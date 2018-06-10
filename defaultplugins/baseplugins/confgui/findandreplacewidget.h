#ifndef FINDANDREPLACEWIDGET_H
#define FINDANDREPLACEWIDGET_H

#include <QWidget>
#include <QValidator>
#include "../findandreplace.h"

namespace Ui {
    class FindAndReplaceWidget;
}

class Hexvalidator : public QValidator {
        Q_OBJECT

    public:
        explicit Hexvalidator();
        ~Hexvalidator();
        QValidator::State validate(QString &input, int &pos) const;
    private:
        static const QString HEXCHAR;
};

class FindAndReplaceWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit FindAndReplaceWidget(FindAndReplace * ntransform, QWidget *parent = 0);
        ~FindAndReplaceWidget();

    private:
        Ui::FindAndReplaceWidget *ui;
        FindAndReplace * transform;
};

#endif // FINDANDREPLACEWIDGET_H

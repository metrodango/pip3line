#ifndef TRANSFORMGUIBUTTON_H
#define TRANSFORMGUIBUTTON_H

#include <QPushButton>
#include <transformabstract.h>
#include <QSharedPointer>
#include <QDialog>

class TransformGuiButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit TransformGuiButton(TransformAbstract * transform, QWidget *parent = nullptr);
        ~TransformGuiButton();
    signals:

    public slots:
        void onGuiDeleted();
        void onTransformDeleted();
        void onGuiRequested();
    private:
        Q_DISABLE_COPY(TransformGuiButton)
        TransformAbstract * transform;
        QDialog * confDialog;
};

#endif // TRANSFORMGUIBUTTON_H

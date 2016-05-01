#ifndef TRANSFORMSELECTORWIDGET_H
#define TRANSFORMSELECTORWIDGET_H

#include <QWidget>

namespace Ui {
    class TransformSelectorWidget;
}

class GuiHelper;
class TransformAbstract;

class TransformSelectorWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit TransformSelectorWidget(GuiHelper *guiHelper, QWidget *parent = 0);
        ~TransformSelectorWidget();
    signals:
        void newTransform(TransformAbstract * transform);
    private slots:
        void onTransformSelected(QString name);
        void onTransformDelete();
    private:
        Ui::TransformSelectorWidget *ui;
        GuiHelper *guiHelper;
        TransformAbstract * currentTransform;
        QWidget * confGui;
};

#endif // TRANSFORMSELECTORWIDGET_H

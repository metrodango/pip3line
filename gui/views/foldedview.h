#ifndef FOLDEDVIEW_H
#define FOLDEDVIEW_H

#include <QFrame>
#include "transformwidget.h"

class QMouseEvent;

namespace Ui {
    class FoldedView;
}

class FoldedView : public QFrame
{
        Q_OBJECT

    public:
        explicit FoldedView(TransformWidget * transformWidget, QWidget *parent = nullptr);
        ~FoldedView();
        TransformWidget *getTransformWidget() const;
        void enableDelete(bool enable);
    signals:
        void unfoldRequested();
        void deleteRequested();
        void indertRequested();
    private:
        Q_DISABLE_COPY(FoldedView)
        void mouseDoubleClickEvent(QMouseEvent * event);
        Ui::FoldedView *ui;
        TransformWidget * transformWidget;
};

#endif // FOLDEDVIEW_H

#include "foldedview.h"
#include "ui_foldedview.h"

FoldedView::FoldedView(TransformWidget * transformWidget, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FoldedView),
    transformWidget(transformWidget)
{
    ui->setupUi(this);
    ui->descLabel->setText(transformWidget->getDescription());
    connect(ui->deletePushButton, &QPushButton::clicked, transformWidget, &TransformWidget::deletionRequest);
    connect(transformWidget, &TransformWidget::destroyed, this, &FoldedView::deleteLater);
    connect(ui->insertPushButton, &QPushButton::clicked, transformWidget, &TransformWidget::insertRequest);
    connect(transformWidget, &TransformWidget::resetDone, this, &FoldedView::unfoldRequested);
}

FoldedView::~FoldedView()
{
    delete ui;
    transformWidget = nullptr;
}

TransformWidget *FoldedView::getTransformWidget() const
{
    return transformWidget;
}

void FoldedView::enableDelete(bool enable)
{
    ui->deletePushButton->setEnabled(enable);
}

void FoldedView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit unfoldRequested();
    }
}

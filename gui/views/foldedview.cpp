#include "foldedview.h"
#include "ui_foldedview.h"

FoldedView::FoldedView(TransformWidget * transformWidget, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FoldedView),
    transformWidget(transformWidget)
{
    ui->setupUi(this);
    ui->descLabel->setText(transformWidget->getDescription());
    connect(ui->deletePushButton, SIGNAL(clicked(bool)), transformWidget, SIGNAL(deletionRequest()));
    connect(transformWidget, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
    connect(ui->insertPushButton, SIGNAL(clicked(bool)), transformWidget, SIGNAL(insertRequest()));
    connect(transformWidget, SIGNAL(resetDone()), this, SIGNAL(unfoldRequested()));
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

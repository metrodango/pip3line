#include "baseblocksourcewidget.h"
#include "ui_baseblocksourcewidget.h"
#include "blockssource.h"
#include "shared/defaultcontrolgui.h"
#include "connectionswidget.h"
#include <transformabstract.h>
#include <QDebug>

BaseBlockSourceWidget::BaseBlockSourceWidget(BlocksSource *bs, QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::BaseBlockSourceWidget),
    bsource(bs),
    inboundTranformWidget(nullptr),
    outboundTranformWidget(nullptr)
{
    ui->setupUi(this);

    if (bs->getType() == BlocksSource::SERVER) {
        setTabText(0,tr("Server"));
    } else if (bs->getType() == BlocksSource::CLIENT) {
        setTabText(0,tr("Client"));
    }
    ui->reflexionCheckBox->setChecked(bsource->isReflexive());

    if (bsource->getFlags() & BlocksSource::REFLEXION_OPTIONS) {
        connect(ui->reflexionCheckBox, SIGNAL(toggled(bool)), bsource, SLOT(setReflexive(bool)));
    } else {
        ui->reflexionCheckBox->setVisible(false);
    }

    ui->tlsCheckBox->setChecked(bsource->isTLSEnable());

    if (bsource->getFlags() & BlocksSource::TLS_OPTIONS) {
        connect(ui->tlsCheckBox, SIGNAL(toggled(bool)), bsource, SLOT(setTlsEnable(bool)));
        connect(bsource,SIGNAL(sslChanged(bool)), this, SLOT(onTLSToggled(bool)));
    } else {
        ui->tlsCheckBox->setVisible(false);
    }

    ui->b64BlocksGroupBox->setChecked(bsource->isB64Blocks());
    ui->b64MaxLengthSpinBox->setValue(bsource->getB64MaxBlockLength());
    ui->b64SeparatorWidget->setChar(bsource->getB64BlocksSeparator());

    if (bsource->getFlags() & BlocksSource::B64BLOCKS_OPTIONS) {
        connect(ui->b64BlocksGroupBox, SIGNAL(toggled(bool)), bsource, SLOT(setB64Blocks(bool)));
        connect(ui->b64MaxLengthSpinBox, SIGNAL(valueChanged(int)), bsource, SLOT(setB64MaxBlockLength(int)));
        connect(ui->b64SeparatorWidget, SIGNAL(charChanged(char)), bsource, SLOT(setB64BlocksSeparator(char)));
    } else {
        ui->b64BlocksGroupBox->setVisible(false);
    }

    DefaultControlGui * gcg = new(std::nothrow) DefaultControlGui(this);
    if (gcg == nullptr) {
        qFatal("Cannot allocate memory for DefaultControlGui X{");
    }
    gcg->setConfButtonVisible(false);
    gcg->setStateStopped(!bsource->isStarted());

    onInboundTransformModified();
    onOutboundTransformModified();

    connect(gcg, SIGNAL(start()), bsource, SLOT(startListening()), Qt::QueuedConnection);
    connect(gcg, SIGNAL(stop()), bsource, SLOT(stopListening()), Qt::QueuedConnection);
    connect(gcg, SIGNAL(reset()), bsource, SLOT(restart()), Qt::QueuedConnection);
    connect(bsource, SIGNAL(started()), gcg, SLOT(receiveStart()), Qt::QueuedConnection);
    connect(bsource, SIGNAL(stopped()), gcg, SLOT(receiveStop()), Qt::QueuedConnection);

    connect(ui->selectInboundTransformPushButton, SIGNAL(clicked(bool)), bsource, SIGNAL(inboundTranformSelectionRequested()));
    connect(ui->selectOutboundTransformPushButton, SIGNAL(clicked(bool)), bsource, SIGNAL(outboundTranformSelectionRequested()));
    connect(bsource, SIGNAL(inboundTransformModfied()), this, SLOT(onInboundTransformModified()));
    connect(bsource, SIGNAL(outboundTranformModfied()), this, SLOT(onOutboundTransformModified()));

    ui->genericLayout->insertWidget(ui->genericLayout->count() - 1 , gcg);

    ConnectionsWidget * cw = new(std::nothrow) ConnectionsWidget(bsource,this);
    if (cw == nullptr) {
        qFatal("Cannot allocate memory for ConnectionsWidget X{");
    }

    addTab(cw, tr("Connections"));
}

BaseBlockSourceWidget::~BaseBlockSourceWidget()
{
    delete ui;
}

void BaseBlockSourceWidget::insertWidgetInGeneric(int index, QWidget *widget)
{
    if (widget == nullptr) { // nothing to do here
        qCritical() << tr("[BaseBlockSourceWidget::insertWidgetInGeneric] widget is nullptr, ignoring");
        return;
    }

    if (index < 0 || index > ui->genericLayout->count()) {
        qCritical() << tr("[BaseBlockSourceWidget::insertWidgetInGeneric] index is invalid, ignoring");
        return;
    }

    ui->genericLayout->insertWidget(index,widget);
}

void BaseBlockSourceWidget::setTLSWidget(QWidget *widget)
{
    sslgui = widget;
    onTLSToggled(ui->tlsCheckBox->isChecked());
}

void BaseBlockSourceWidget::onTLSToggled(bool checked)
{
    if (sslgui != nullptr) {
        if (checked)
            addTab(sslgui, tr("SSL/TLS"));
        else {
            int index = indexOf(sslgui);
            if (index != -1)
                removeTab(index);
        }
    }

    emit tlsEnabled(checked);
}

void BaseBlockSourceWidget::onInboundTransformModified()
{
    if (inboundTranformWidget != nullptr) {
        delete inboundTranformWidget;
        inboundTranformWidget = nullptr;
    }

    TransformAbstract *ta = bsource->getInboundTranform();
    if (ta != nullptr) {
        inboundTranformWidget = ta->getGui(nullptr);
        ui->inboundTransformTab->layout()->addWidget(inboundTranformWidget);
        connect(inboundTranformWidget, SIGNAL(destroyed(QObject*)), this, SLOT(onInboundTransformWidgetDDestroyed()));
    }
}

void BaseBlockSourceWidget::onOutboundTransformModified()
{
    if (outboundTranformWidget != nullptr) {
        delete outboundTranformWidget;
        outboundTranformWidget = nullptr;
    }

    TransformAbstract *ta = bsource->getOutboundTranform();
    if (ta != nullptr) {
        outboundTranformWidget = ta->getGui(nullptr);
        ui->outboundTransformTab->layout()->addWidget(outboundTranformWidget);
        connect(outboundTranformWidget, SIGNAL(destroyed(QObject*)), this, SLOT(onOutboundTransformWidgetDDestroyed()));
    }
}

void BaseBlockSourceWidget::onInboundTransformWidgetDDestroyed()
{
    inboundTranformWidget = nullptr;
}

void BaseBlockSourceWidget::onOutboundTransformWidgetDDestroyed()
{
    outboundTranformWidget = nullptr;
}

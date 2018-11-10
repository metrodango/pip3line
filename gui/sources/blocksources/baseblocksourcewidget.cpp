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
        connect(ui->reflexionCheckBox, &QCheckBox::toggled, bsource, &BlocksSource::setReflexive);
    } else {
        ui->reflexionCheckBox->setVisible(false);
    }

    ui->tlsCheckBox->setChecked(bsource->isTLSEnabled());

    if (bsource->getFlags() & BlocksSource::TLS_OPTIONS) {
        connect(ui->tlsCheckBox, &QCheckBox::toggled, bsource, &BlocksSource::setTlsEnable);
        connect(bsource, &BlocksSource::sslChanged, this, &BaseBlockSourceWidget::onTLSToggled);
    } else {
        ui->tlsCheckBox->setVisible(false);
    }

    ui->b64BlocksGroupBox->setChecked(bsource->isB64Blocks());
    ui->b64MaxLengthSpinBox->setValue(bsource->getB64MaxBlockLength());
    ui->b64SeparatorWidget->setChar(bsource->getB64BlocksSeparator());

    if (bsource->getFlags() & BlocksSource::B64BLOCKS_OPTIONS) {
        connect(ui->b64BlocksGroupBox, &QGroupBox::toggled, bsource, &BlocksSource::setB64Blocks);
        //connect(ui->b64MaxLengthSpinBox, qOverload<int>(&QSpinBox::valueChanged), bsource, &BlocksSource::setB64MaxBlockLength);
        connect(ui->b64MaxLengthSpinBox, SIGNAL(valueChanged(int)), bsource, SLOT(setB64MaxBlockLength(int)));
        connect(ui->b64SeparatorWidget, &HexWidget::charChanged, bsource, &BlocksSource::setB64BlocksSeparator);
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

    connect(gcg, &DefaultControlGui::start, bsource, &BlocksSource::startListening, Qt::QueuedConnection);
    connect(gcg, &DefaultControlGui::stop, bsource, &BlocksSource::stopListening, Qt::QueuedConnection);
    connect(gcg, &DefaultControlGui::reset, bsource, &BlocksSource::restart, Qt::QueuedConnection);
    connect(bsource, &BlocksSource::started, gcg, &DefaultControlGui::receiveStart, Qt::QueuedConnection);
    connect(bsource, &BlocksSource::stopped, gcg, &DefaultControlGui::receiveStop, Qt::QueuedConnection);

    connect(ui->selectInboundTransformPushButton, &QPushButton::clicked, bsource, &BlocksSource::inboundTranformSelectionRequested);
    connect(ui->selectOutboundTransformPushButton, &QPushButton::clicked, bsource, &BlocksSource::outboundTranformSelectionRequested);
    connect(bsource, &BlocksSource::inboundTransformModfied, this, &BaseBlockSourceWidget::onInboundTransformModified);
    connect(bsource, &BlocksSource::outboundTranformModfied, this, &BaseBlockSourceWidget::onOutboundTransformModified);

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
        connect(inboundTranformWidget, &QWidget::destroyed, this, &BaseBlockSourceWidget::onInboundTransformWidgetDDestroyed);
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
        connect(outboundTranformWidget, &QWidget::destroyed, this, &BaseBlockSourceWidget::onOutboundTransformWidgetDDestroyed);
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

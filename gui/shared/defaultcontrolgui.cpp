#include "defaultcontrolgui.h"
#include "ui_defaultcontrolgui.h"
#include "guiconst.h"

DefaultControlGui::DefaultControlGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DefaultControlGui)
{
    ui->setupUi(this);
    ui->startStopPushButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    setStateStopped(stateStopped);
    connect(ui->resetPushButton, &QPushButton::clicked, this, &DefaultControlGui::reset);
    connect(ui->startStopPushButton, &QPushButton::clicked, this, &DefaultControlGui::onStartStop);
    connect(ui->configPushButton, &QPushButton::clicked, this, &DefaultControlGui::requestConfPanel);
}

DefaultControlGui::~DefaultControlGui()
{
    delete ui;
}

void DefaultControlGui::receiveStart()
{
    setStateStopped(false);
}

void DefaultControlGui::receiveStop()
{
    setStateStopped(true);
}

void DefaultControlGui::onStartStop()
{
    if (stateStopped) {
        emit start();
    }
    else {
        emit stop();
    }
}

void DefaultControlGui::setStateStopped(bool startState)
{
    stateStopped = startState;

    if (startState) {
        ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-start.png"));
    } else {
        ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-stop-7.png"));
    }
}

void DefaultControlGui::appendWidget(QWidget *widget)
{
    if (widget != nullptr)
        ui->horizontalLayout->addWidget(widget);
}

void DefaultControlGui::setConfButtonVisible(bool visible)
{
    ui->configPushButton->setVisible(visible);
}

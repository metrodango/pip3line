/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "debugdialog.h"
#include "ui_debugdialog.h"
#include <transformabstract.h>
#include <QHash>
#include <QHashIterator>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QAction>
#include <transformmgmt.h>
#include <QStringList>
#include "guihelper.h"
#include "views/hexview.h"
#include "sources/basicsource.h"
#include "sources/currentmemorysource.h"

DebugDialog::DebugDialog(GuiHelper *helper, QWidget *parent) :
    AppDialog(helper, parent)
{
    ui = new(std::nothrow) Ui::DebugDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::DebugDialog X{");
    }


    source = new(std::nothrow) CurrentMemorysource();
    if (source == nullptr) {
        qFatal("Cannot allocate memory for BasicSource X{");
    }
    source->setReadOnly();

    hexview = new(std::nothrow) HexView(source,guiHelper,this);
    if (hexview == nullptr) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    ui->setupUi(this);

    ui->debugLayout->insertWidget(ui->debugLayout->indexOf(ui->confLabel) + 1, hexview);

    ui->addressLineEdit->installEventFilter(this);
    ui->addressLineEdit->setText(QString::number(source->startingRealOffset(), 16));

    ui->viewSizeSpinBox->setValue(source->viewSize());

    refreshTransformInstances();

    //connect(ui->viewSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged),source, &CurrentMemorysource::setViewSize);
    connect(ui->viewSizeSpinBox, SIGNAL(valueChanged(int)), source, SLOT(setViewSize(int)));
    connect(ui->addressLineEdit, &QLineEdit::returnPressed, this, &DebugDialog::onLoad);
    connect(ui->previousPushButton, &QPushButton::clicked, source, &CurrentMemorysource::historyBackward);
    connect(ui->nextPushButton, &QPushButton::clicked, source, &CurrentMemorysource::historyForward);
    connect(ui->refreshPushButton, &QPushButton::clicked, this, &DebugDialog::refreshTransformInstances);
    connect(ui->transformListWidget, &QListWidget::doubleClicked, this, &DebugDialog::onInstanceClick);
}

DebugDialog::~DebugDialog()
{
    delete ui;
    delete source;
    qDebug() << "Destroyed " << this;
}

void DebugDialog::onLoad()
{
    QString addrString = ui->addressLineEdit->text();
    if (addrString.startsWith("0x"))
        addrString = addrString.mid(2);
    else if(addrString.startsWith("x"))
        addrString = addrString.mid(1);

    bool ok;
    quint64 addr = addrString.toULongLong(&ok, 16);

    if (!ok) {
        qWarning() << "Debugger: Invalid address entered";
        return;
    } else if (addr == 0) {
        qWarning() << "Debugger: nullptr address entered";
        return;
    }

    // QObject *obj = (QObject *) addr;
    ui->addressLineEdit->blockSignals(true);
    source->setStartingOffset(addr);
    ui->addressLineEdit->blockSignals(false);
}

void DebugDialog::refreshTransformInstances()
{
    ui->transformListWidget->clear();
    QList<TransformAbstract *> list = guiHelper->getTransformFactory()->getTransformInstances();
    QStringList slist;
    for (int i = 0; i < list.size(); i++) {
        QString value = QString("0x%1").arg(QString::number(reinterpret_cast<quintptr>(list.at(i)),16));
        slist.append(value);
    }

    slist.sort();
    ui->transformListWidget->addItems(slist);
}

void DebugDialog::onInstanceClick(QModelIndex index)
{
    ui->addressLineEdit->setText(index.data().toString());
    onLoad();
    loadQObject(reinterpret_cast<QObject *>(index.data().toString().toULongLong(nullptr,16)));
}

void DebugDialog::loadQObject(QObject *obj)
{
    const QMetaObject *superClassObj = obj->metaObject()->superClass();

    QObject *parentObj = obj->parent();


    if (superClassObj != nullptr) {
        QString parentName = superClassObj->className();
        ui->nameValLabel->setText(tr("%1 (%2)").arg(obj->metaObject()->className()).arg(parentName));

        if (parentName == "TransformAbstract" || parentName == "ScriptTransformAbstract") {
            TransformAbstract * ta = static_cast<TransformAbstract *> (obj);

            QString stringConf;
            QHash<QString, QString> conf = ta->getConfiguration();
            QHashIterator<QString, QString> i(conf);
            while (i.hasNext()) {
                i.next();
                stringConf.append(i.key()).append(" => ").append(i.value()).append("\n");
            }
            if (stringConf.endsWith("\n"))
                stringConf.chop(1);
            ui->confLabel->setText(stringConf);

        } else {
            ui->confLabel->clear();
        }
    } else
        ui->nameValLabel->setText(tr("%1").arg(obj->metaObject()->className()));

    if (parentObj == nullptr)
        ui->parentValLabel->setText(tr("None"));
    else
        ui->parentValLabel->setText(tr("0x%1 %2").arg(QString::number(reinterpret_cast<quint64>(parentObj), 16)).arg(parentObj->metaObject()->className()));

    obj->dumpObjectInfo();
    obj->dumpObjectTree();
}

bool DebugDialog::eventFilter(QObject *o, QEvent *event)
{
    if (o == ui->addressLineEdit && event->type() == QEvent::KeyPress) {

        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent == nullptr) {
            qWarning() << "[DebugDialog]nullptr KeyEvent";
            return false;
        }
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            onLoad();
            keyEvent->accept();
            return true;
        }
    }
    return false;
}

void DebugDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
}

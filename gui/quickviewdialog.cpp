/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "quickviewdialog.h"
#include "guihelper.h"
#include "quickviewitem.h"
#include "ui_quickviewdialog.h"
#include "loggerwidget.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

using namespace GuiConst;

#include <QDebug>

QuickViewDialog::QuickViewDialog(GuiHelper *nguiHelper, QWidget *parent) :
    AppDialog(nguiHelper, parent)
{
    ui = new(std::nothrow) Ui::QuickViewDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::QuickViewDialog X{");
    }
    ui->setupUi(this);
    setModal(false);
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(newItem()));
    connect(ui->resetPushButton, SIGNAL(clicked()), this, SLOT(onReset()));

    QStringList saved = guiHelper->getQuickViewConf();
    for (int i = 0; i < saved.size(); i++) {
        addItem(saved.at(i));
    }

    // make the dialog adjust to its layout
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

QuickViewDialog::~QuickViewDialog()
{
    guiHelper->saveQuickViewConf(getConf());
    guiHelper = nullptr;
    clear();
    delete ui;
}

BaseStateAbstract *QuickViewDialog::getStateMngtObj()
{
    BaseStateAbstract *stateObj = new(std::nothrow) QuickViewDialogStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for QuickViewDialogStateObj X{");
    }

    return stateObj;
}

void QuickViewDialog::newItem()
{
    QuickViewItem * qvi = new(std::nothrow) QuickViewItem(guiHelper, this);
    if (qvi != nullptr) {
        itemList.append(qvi);
        connect(qvi, SIGNAL(destroyed()), this, SLOT(itemDeleted()));
        if (qvi->configure()) {
            ui->itemLayout->addWidget(qvi);
            qvi->processData(currentData);
        }
    } else {
        qFatal("Cannot allocate memory for QuickViewItem for newItem X{");
    }
}

void QuickViewDialog::itemDeleted()
{
    QuickViewItem * item = static_cast<QuickViewItem *>(sender());

    int i = itemList.removeAll(item);

    if (i == 0) { // something is wrong
        guiHelper->getLogger()->logError(tr("QuickView item not found 0x%1").arg(QString::number((quintptr)item,16)),"QuickView");
    } else if (i > 1) { // something is really wrong
        guiHelper->getLogger()->logError(tr("Multiple QuickView item found 0x%1: %2").arg(QString::number((quintptr)item,16)).arg(i),"QuickView");
    }
}

void QuickViewDialog::onReset()
{
    clear();

    guiHelper->saveQuickViewConf(QStringList());

    QStringList saved = guiHelper->getQuickViewConf();
    for (int i = 0; i < saved.size(); i++) {
        addItem(saved.at(i));
    }

}

void QuickViewDialog::addItem(const QString &conf)
{
    QuickViewItem * qvi = new(std::nothrow) QuickViewItem(guiHelper, this, conf);
    if (qvi != nullptr) {
        if (!qvi->isConfigured()) {
            delete qvi;
            return;
        }
        itemList.append(qvi);
        ui->itemLayout->addWidget(qvi);
        qvi->processData(currentData);
        connect(qvi, SIGNAL(destroyed()), this, SLOT(itemDeleted()));
    } else {
        qFatal("Cannot allocate memory for QuickViewItem for addItem X{");
    }
}

void QuickViewDialog::clear()
{
    QList<QuickViewItem *> list = itemList;

    for (int i = 0; i < list.size(); i++) {
        delete list.at(i);
    }

    itemList.clear();
}

QStringList QuickViewDialog::getConf()
{
    QStringList saving;
    for (int i = 0; i < itemList.size(); i++) {
        saving.append(itemList.at(i)->getXmlConf());
        // don't need to delete them, the ui will do it
    }

    return saving;
}

void QuickViewDialog::receivingData(const QByteArray &data)
{
    currentData = data;

    for (int i = 0; i < itemList.size(); i++) {
        itemList.at(i)->processData(currentData);
    }

}




QuickViewDialogStateObj::QuickViewDialogStateObj(QuickViewDialog *diag) :
    AppStateObj(diag)
{

}

QuickViewDialogStateObj::~QuickViewDialogStateObj()
{

}

void QuickViewDialogStateObj::internalRun()
{
    QuickViewDialog * diag = dynamic_cast<QuickViewDialog *>(dialog);
    if (diag == nullptr) {
        qFatal("Could not cast AppDialog to QuickViewDialog X{");
    }

    if (flags & GuiConst::STATE_LOADSAVE_QUICKVIEW_CONF) {
        if (flags & GuiConst::STATE_SAVE_REQUEST) {
            QStringList saved = diag->getConf();
            writer->writeAttribute(GuiConst::STATE_QUICKVIEW_ITEM_COUNT, write(saved.size()));
            for (int i = 0; i < saved.size(); i++) {
                writer->writeStartElement(GuiConst::STATE_QUICKVIEW_ITEM);
                writer->writeAttribute(GuiConst::STATE_CONF, write(saved.at(i).toUtf8()));
                genCloseElement();
            }

        } else {
            QXmlStreamAttributes attrList = reader->attributes();
            bool ok = false;
            int size = readInt(attrList.value(GuiConst::STATE_QUICKVIEW_ITEM_COUNT),&ok);
            qDebug() << "loading" << size << "item(s)";
            if (ok && size > 0) {
                diag->clear();
                for (int i = 0; i < size; i++) {
                    if (readNextStart(GuiConst::STATE_QUICKVIEW_ITEM)) {
                        attrList = reader->attributes();
                        QByteArray conf = readByteArray(attrList.value(GuiConst::STATE_CONF));
                        if (!conf.isEmpty()) {
                            diag->addItem(QString::fromUtf8(conf.constData(), conf.size()));
                        } else {
                            emit log(tr("Item conf is empty"), metaObject()->className(), Pip3lineConst::LERROR);
                        }

                        genCloseElement();
                    }
                }
            }

        }
    }
}

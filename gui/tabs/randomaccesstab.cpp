/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "randomaccesstab.h"
#include "ui_randomaccesstab.h"
#include "../shared/offsetgotowidget.h"
#include "../shared/searchwidget.h"
#include "../sources/bytesourceabstract.h"
#include "../views/hexview.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../shared/readonlybutton.h"
#include "../shared/clearallmarkingsbutton.h"
#include "../shared/bytesourceguibutton.h"
#include "../shared/detachtabbutton.h"
#include "../views/bytetableview.h"
#include "../views/bytetableview.h"
#include <QScrollBar>
#include <QDebug>

#include "../shared/guiconst.h"
using namespace GuiConst;

const QString RandomAccessTab::LOGID = "RandomAccessTab";

RandomAccessTab::RandomAccessTab(ByteSourceAbstract *nbytesource, GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper,parent),
    bytesource(nbytesource)
{
    ui = new(std::nothrow) Ui::RandomAccessTab();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::RandomAccessTab X{");
    }
    ui->setupUi(this);

    setName(bytesource->name());
    connect(bytesource,SIGNAL(nameChanged(QString)), SLOT(setName(QString)));

    hexView = new(std::nothrow) HexView(bytesource, guiHelper,this);
    if (hexView == nullptr) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    ui->mainLayout->insertWidget(ui->mainLayout->indexOf(ui->logsWidget) + 1,hexView);

    searchWidget = new(std::nothrow) SearchWidget(bytesource, guiHelper, this);
    if (searchWidget == nullptr) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }

    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    ui->mainLayout->insertWidget( 1,searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,QBitArray,bool)), SLOT(onSearch(QByteArray,QBitArray,bool)));
    connect(searchWidget, SIGNAL(jumpTo(quint64,quint64)),hexView,  SLOT(gotoSearch(quint64,quint64)));

    hexView->installEventFilter(this);
    hexView->getHexTableView()->verticalScrollBar()->setVisible(true);
    connect(hexView,SIGNAL(askForFileLoad()), SLOT(fileLoadRequest()));
    gotoWidget = new(std::nothrow) OffsetGotoWidget(guiHelper,this);
    if (gotoWidget == nullptr) {
        qFatal("Cannot allocate memory for OffsetGotoWidget X{");
    }
    gotoWidget->setMaximumWidth(150);
    ui->toolsLayout->insertWidget(0,gotoWidget);
    connect(gotoWidget,SIGNAL(gotoRequest(quint64,bool,bool,bool)), SLOT(onGotoOffset(quint64,bool,bool,bool)));

    roButton = new(std::nothrow) ReadOnlyButton(bytesource,this);
    if (roButton == nullptr) {
        qFatal("Cannot allocate memory for ReadOnlyButton X{");
    }
    ui->toolsLayout->insertWidget(2,roButton);

    clearAllMarksButton = new(std::nothrow) ClearAllMarkingsButton(bytesource,this);
    if (clearAllMarksButton == nullptr) {
        qFatal("Cannot allocate memory for clearAllMarkingsButton X{");
    }
    ui->toolsLayout->insertWidget(2,clearAllMarksButton);

    guiButton = new(std::nothrow) ByteSourceGuiButton(bytesource,guiHelper,this);
    if (guiButton == nullptr) {
        qFatal("Cannot allocate memory for ByteSourceGuiButton X{");
    }

    ui->toolsLayout->insertWidget(3, guiButton);

    // Checking if there are some additional buttons
    QWidget *gui = bytesource->getGui(this, ByteSourceAbstract::GUI_BUTTONS);
    if (gui != nullptr) {
        ui->toolsLayout->insertWidget(2, gui);
    }

    if (bytesource->hasDiscreetView()) {
        ui->viewSizeSpinBox->setValue(bytesource->viewSize());
        connect(ui->viewSizeSpinBox, SIGNAL(valueChanged(int)),bytesource, SLOT(setViewSize(int)));
    } else {
        ui->viewSizeSpinBox->setVisible(false);
    }
    ui->logsWidget->setVisible(false);

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == nullptr) {
        qFatal("Cannot allocate memory for detachButton X{");
    }

    ui->toolsLayout->insertWidget(0,detachButton);

    connect(ui->prevPushButton, SIGNAL(clicked()), bytesource,SLOT(historyBackward()));
    connect(ui->nextPushButton, SIGNAL(clicked()), bytesource, SLOT(historyForward()));
    connect(ui->closeLogsPushButton, SIGNAL(clicked()), SLOT(onCloseLogView()));
    connect(bytesource, SIGNAL(askFileLoad()), SLOT(fileLoadRequest()));
    connect(bytesource, SIGNAL(updated(quintptr)), SLOT(onSourceUpdated()));
}

RandomAccessTab::~RandomAccessTab()
{
    delete gotoWidget;
    delete searchWidget;
    delete roButton;
    delete ui;
    delete bytesource;
    bytesource = nullptr;
}

int RandomAccessTab::getBlockCount() const
{
    return 1;
}

ByteSourceAbstract *RandomAccessTab::getSource(int)
{
    return bytesource;
}

ByteTableView *RandomAccessTab::getHexTableView(int)
{
    return hexView->getHexTableView();
}

void RandomAccessTab::loadFromFile(QString fileName)
{
    if (fileName.isEmpty()) {
        logger->logError("Empty file name, ignoring",LOGID);
        return;
    }

    if (bytesource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        bytesource->fromLocalFile(fileName);
        integrateByteSource();
    }  else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
    }
}

void RandomAccessTab::setData(const QByteArray &)
{
    QMessageBox::critical(this,tr("Action rejected T_T"), tr("Cannot set data in this tab"),QMessageBox::Ok);
}

bool RandomAccessTab::canReceiveData()
{
    return false;
}

BaseStateAbstract *RandomAccessTab::getStateMngtObj()
{
    RandomAccessStateObj *stateObj = new(std::nothrow) RandomAccessStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for RandomAccessStateObj X{");
    }

    return stateObj;
}

void RandomAccessTab::fileLoadRequest()
{
    QString fileName;
    if (bytesource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
        if (!fileName.isEmpty()) {
            bytesource->fromLocalFile(fileName);
            integrateByteSource();
        }
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
    }
}

void RandomAccessTab::integrateByteSource()
{
    roButton->refreshStateValue();

    ui->prevPushButton->setEnabled(bytesource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
    ui->nextPushButton->setEnabled(bytesource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
}
SearchWidget *RandomAccessTab::getSearchWidget() const
{
    return searchWidget;
}

OffsetGotoWidget *RandomAccessTab::getGotoWidget() const
{
    return gotoWidget;
}

void RandomAccessTab::onSearch(QByteArray item, QBitArray mask, bool)
{
    hexView->search(item, mask);
}

void RandomAccessTab::onGotoOffset(quint64 offset, bool absolute,bool negative, bool select)
{
    if (!hexView->goTo(offset,absolute,negative,select)) {
        gotoWidget->setStyleSheet(GuiStyles::LineEditError);
    } else {
        gotoWidget->setStyleSheet(qApp->styleSheet());
    }
}

void RandomAccessTab::log(QString mess, QString , Pip3lineConst::LOGLEVEL level)
{
    QColor color = Qt::black;
    if (level == Pip3lineConst::LERROR)
        color = Qt::red;
    else if (level == Pip3lineConst::LWARNING)
        color = Qt::blue;

    ui->logsTextEdit->setTextColor(color);
    ui->logsTextEdit->append(mess);
    ui->logsWidget->setVisible(true);
}

void RandomAccessTab::onCloseLogView()
{
    ui->logsWidget->hide();
    ui->logsTextEdit->clear();
}

void RandomAccessTab::onSourceUpdated()
{
    ui->viewSizeSpinBox->blockSignals(true);
    ui->viewSizeSpinBox->setValue(bytesource->viewSize());
    ui->viewSizeSpinBox->blockSignals(false);
}

bool RandomAccessTab::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            if (keyEvent->key() == Qt::Key_N)  {
                searchWidget->nextFind(bytesource->getRealOffset(hexView->getLowPos()));
                return true;
            } else if (keyEvent->key() == Qt::Key_G) {
                gotoWidget->setFocus();
                return true;
            } else if (keyEvent->key() == Qt::Key_F) {
                searchWidget->setFocus();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}


RandomAccessStateObj::RandomAccessStateObj(RandomAccessTab *tab) :
    TabStateObj(tab)
{
    name = metaObject()->className();
}

RandomAccessStateObj::~RandomAccessStateObj()
{

}

void RandomAccessStateObj::run()
{
    RandomAccessTab * rTab = dynamic_cast<RandomAccessTab *> (tab);
    TabStateObj::run();

    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeAttribute(GuiConst::STATE_SEARCH_WIDGET, write(rTab->getSearchWidget()->text(),true));
        writer->writeAttribute(GuiConst::STATE_GOTOOFFSET_WIDGET, write(rTab->getGotoWidget()->text()));
        writer->writeAttribute(GuiConst::STATE_SCROLL_INDEX, write(rTab->hexView->getHexTableView()->verticalScrollBar()->value()));

    } else {
        QXmlStreamAttributes attrList = reader->attributes();
        if (attrList.hasAttribute(GuiConst::STATE_SEARCH_WIDGET)) {
            rTab->getSearchWidget()->setText(readString(attrList.value(GuiConst::STATE_SEARCH_WIDGET)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_GOTOOFFSET_WIDGET)) {
            rTab->getGotoWidget()->setText(readString(attrList.value(GuiConst::STATE_GOTOOFFSET_WIDGET)));
        }

        bool ok = false;

        if (attrList.hasAttribute(GuiConst::STATE_SCROLL_INDEX)) {
            int index = readInt(attrList.value(GuiConst::STATE_SCROLL_INDEX), &ok);
            if (ok) {
                RandomAccessClosingStateObj *tempState = new(std::nothrow) RandomAccessClosingStateObj(rTab);
                if (tempState == nullptr) {
                    qFatal("Cannot allocate memory for RandomAccessClosingStateObj X{");
                }

                tempState->setScrollIndex(index);

                emit addNewState(tempState);
            }
        }
    }

    BaseStateAbstract *state = rTab->getSource(0)->getStateMngtObj();
    emit addNewState(state);
}

RandomAccessClosingStateObj::RandomAccessClosingStateObj(RandomAccessTab *tab) :
    tab(tab)
{
    scrollIndex = 0;
    name = metaObject()->className();
}

RandomAccessClosingStateObj::~RandomAccessClosingStateObj()
{

}

void RandomAccessClosingStateObj::run()
{
    qDebug() << "set Index" << scrollIndex << tab->hexView->getHexTableView()->verticalScrollBar()->minimum() << tab->hexView->getHexTableView()->verticalScrollBar()->maximum();
    qDebug() << "data size" << tab->getSource(0)->size();
    tab->hexView->getHexTableView()->verticalScrollBar()->setValue(scrollIndex);
}

void RandomAccessClosingStateObj::setScrollIndex(int value)
{
    scrollIndex = value;
}

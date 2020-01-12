/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "comparisondialog.h"
#include "ui_comparisondialog.h"
#include <QVariant>
#include <QMessageBox>
#include "views/byteitemmodel.h"
#include "sources/bytesourceabstract.h"
#include "tabs/tababstract.h"
#include "views/bytetableview.h"
#include "loggerwidget.h"
#include "guihelper.h"
#include "shared/guiconst.h"
#include <QColorDialog>
#include <QPixmap>
#include <QComboBox>
#include <QThread>
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>

CompareWorker::CompareWorker(ByteSourceAbstract *sA, ByteSourceAbstract *sB, QObject *parent) :
    QObject(parent)
{
    curProgress.storeRelaxed(0);
    sourceA = sA;
    connect(this, &CompareWorker::newMarkingsA, sourceA, &ByteSourceAbstract::setNewMarkings, Qt::QueuedConnection);
    sourceB = sB;
    connect(this, &CompareWorker::newMarkingsB, sourceB, &ByteSourceAbstract::setNewMarkings, Qt::QueuedConnection);
    startA = 0;
    startB = 0;
    sizeA = sourceA->size();
    sizeB = sourceB->size();
    markA = true;
    markB = true;
    markSame = false;
    stopped = false;
    ismarkingA = false;
    ismarkingB = false;
    rangesA = nullptr;
    rangesB = nullptr;
}

CompareWorker::~CompareWorker()
{

}

void CompareWorker::compare()
{
    quint64 compSize = getComparisonSize();
    tooltipDiffA = tr("%1 \"%2\"").arg(markSame ? "Same as" : "Different from").arg(nameB);
    tooltipDiffB = tr("%1 \"%2\"").arg(markSame ? "Same as" : "Different from").arg(nameA);
    quint64 realA = 0;
    quint64 realB = 0;
    rangesA = new(std::nothrow) BytesRangeList();
    if (rangesA == nullptr) {
        qFatal("Cannot allocate memory for BytesRangeList X{");
    }

    rangesB = new(std::nothrow) BytesRangeList();
    if (rangesB == nullptr) {
        qFatal("Cannot allocate memory for BytesRangeList X{");
    }

    quint64 BLOCKSIZE = GEN_BLOCK_SIZE;

    quint64 count = (compSize / BLOCKSIZE) + ((compSize % BLOCKSIZE) == 0 ? 0 : 1);
    for(quint64 j = 0; j < count; j++) {
        quint64 curBlockIndex = j * BLOCKSIZE;
        quint64 size = qMin(BLOCKSIZE, qMin(sizeA - curBlockIndex, sizeB - curBlockIndex));
        QByteArray dataA = sourceA->extract(startA + curBlockIndex, static_cast<int>(size));
        QByteArray dataB = sourceB->extract(startB + curBlockIndex, static_cast<int>(size));
        if (dataA.size() != dataB.size()) {
            qCritical() << tr("Extracted data size different .. something wrong here T_T");
            endingThread();
            return;
        } else { // getting the actual size read
            size = static_cast<quint64>(dataA.size());
        }

        for (quint64 k = 0; k < size; k++){
            quint64 curindex = k + curBlockIndex;
            realA = startA + curindex;
            realB = startB + curindex;
            if (dataA.at(static_cast<int>(k)) != dataB.at(static_cast<int>(k))) {
                if (!markSame) {
                    markingA(realA);
                    markingB(realB);
                } else {
                    endAMarking();
                    endBMarking();
                }
            } else if (markSame) {
                markingA(realA);
                markingB(realB);
            } else {
                endAMarking();
                endBMarking();
            }

            if (stopped) {
                endingThread();
                return;
            }

            curProgress = curindex;
        }
    }

    if (sizeA != sizeB && !markSame) {
        if (sizeA > sizeB) {
            if (markA)
                BytesRange::addMarkToList(rangesA,sizeB + startA,sizeA + startA -1,marksColor,QColor(),tooltipDiffA);
        } else {
            if (markB)
                BytesRange::addMarkToList(rangesB,sizeA + startB,sizeB + startB -1,marksColor,QColor(),tooltipDiffB);
        }
    }

    endingThread();
}

void CompareWorker::stop()
{
    stopped = true;
}

void CompareWorker::endingThread()
{
    endAMarking();
    endBMarking();

    resultDifferences = qMax(rangesA->byteSize(), rangesB->byteSize());
    if (resultDifferences > 0) {
        emit newMarkingsA(rangesA);
        emit newMarkingsB(rangesB);
    }

    emit finishComparing(resultDifferences);
    deleteLater();
}

void CompareWorker::markingA(quint64 offset)
{
    if (markA) {
        if (!ismarkingA) {
            markerStartA = offset;
            markerEndA = offset;
            ismarkingA = true;
        } else {
            markerEndA = offset;
        }
    }
}

void CompareWorker::endAMarking()
{
    if (markA && ismarkingA)
        BytesRange::addMarkToList(rangesA,markerStartA,markerEndA,marksColor,QColor(),tooltipDiffA);
    ismarkingA = false;
}

void CompareWorker::markingB(quint64 offset)
{
    if (markB) {
        if (!ismarkingB) {
            markerStartB = offset;
            markerEndB = offset;
            ismarkingB = true;
        } else {
            markerEndB = offset;
        }
    }
}

void CompareWorker::endBMarking()
{
    if (markB && ismarkingB)
        BytesRange::addMarkToList(rangesB,markerStartB,markerEndB,marksColor,QColor(),tooltipDiffB);
    ismarkingB = false;
}

QColor CompareWorker::getMarkColor() const
{
    return marksColor;
}

void CompareWorker::setMarkColor(const QColor &value)
{
    marksColor = value;
}

quint64 CompareWorker::getProgress() const
{
    return curProgress.loadRelaxed();
}

quint64 CompareWorker::getComparisonSize() const
{
    return qMin(sizeA, sizeB);
}

QString CompareWorker::getNameB() const
{
    return nameB;
}

void CompareWorker::setNameB(const QString &value)
{
    nameB = value;
}

QString CompareWorker::getNameA() const
{
    return nameA;
}

void CompareWorker::setNameA(const QString &value)
{
    nameA = value;
}

quint64 CompareWorker::getEndB() const
{
    return endB;
}

quint64 CompareWorker::getEndA() const
{
    return endA;
}

bool CompareWorker::getMarkSame() const
{
    return markSame;
}

void CompareWorker::setMarkSame(bool value)
{
    markSame = value;
}

bool CompareWorker::getMarkB() const
{
    return markB;
}

void CompareWorker::setMarkB(bool value)
{
    markB = value;
}

bool CompareWorker::getMarkA() const
{
    return markA;
}

void CompareWorker::setMarkA(bool value)
{
    markA = value;
}

quint64 CompareWorker::getSizeB() const
{
    return sizeB;
}

quint64 CompareWorker::getSizeA() const
{
    return sizeA;
}

quint64 CompareWorker::getStartA() const
{
    return startA;
}

void CompareWorker::setARange(const quint64 start, const quint64 end)
{
    if (start < end) {
        startA = start;
        endA = end;
    } else {
        startA = end;
        endA = start;
    }

    sizeA = end - start + 1;
}

quint64 CompareWorker::getStartB() const
{
    return startB;
}

void CompareWorker::setBRange(const quint64 start, const quint64 end)
{
    if (start < end) {
        startB = start;
        endB = end;
    } else {
        startB = end;
        endB = start;
    }
    sizeB = end - start + 1;
}

const int ComparisonDialog::BUTTONS_TO_SAVE = 7;

ComparisonDialog::ComparisonDialog(GuiHelper *nguiHelper, QWidget *parent) :
    AppDialog(nguiHelper, parent)
{
    ui = new(std::nothrow) Ui::ComparisonDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::ComparisonDialog X{");
    }
    worker = nullptr;
    workerThread = nullptr;
    ui->setupUi(this);
    ui->resultLabel->setVisible(false);
    marksColor = GuiStyles::DEFAULT_MARKING_COLOR;
    changeIconColor(marksColor);

    ui->advancedWidget->hide();

    connect(guiHelper, &GuiHelper::tabsUpdated, this, &ComparisonDialog::loadTabs, Qt::QueuedConnection);
    //connect(ui->tabAComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ComparisonDialog::onTabSelection);
    connect(ui->tabAComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(onTabSelection(int)));
    //connect(ui->tabBComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ComparisonDialog::onTabSelection);
    connect(ui->tabBComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(onTabSelection(int)));
    //connect(ui->entryAComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ComparisonDialog::onEntrySelected);
    connect(ui->entryAComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(onEntrySelected(int)));
    //connect(ui->entryBComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ComparisonDialog::onEntrySelected);
    connect(ui->entryBComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(onEntrySelected(int)));
    connect(ui->tabBComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(onTabSelection(int)));
    connect(this, &ComparisonDialog::finished, this, &ComparisonDialog::hide);
    connect(ui->acceptPushButton, &QPushButton::clicked, this, &ComparisonDialog::onCompare);
    connect(ui->colorPushButton, &QPushButton::clicked, this, &ComparisonDialog::oncolorChange);
    connect(ui->advancedPushButton, &QPushButton::toggled, this, &ComparisonDialog::onAdvancedClicked);
    loadTabs();

    //qDebug() << "Created" << this;

}

ComparisonDialog::~ComparisonDialog()
{
    //qDebug() << "Destroying " << this;
    tabs.clear();
    delete ui;
    guiHelper = nullptr;
    if (workerThread != nullptr) {
        workerThread->quit();
        if (!workerThread->wait(10000))
            qCritical() << "Could not stop the comparison thread.";
        delete workerThread;
    }
    // qDebug() << "Destroyed " << this;
}

BaseStateAbstract *ComparisonDialog::getStateMngtObj()
{
    ComparisonDialogStateObj *stateObj = new(std::nothrow) ComparisonDialogStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for ComparisonDialogStateObj X{");
    }

    return stateObj;
}

QBitArray ComparisonDialog::getUiConf() const
{
    QBitArray conf(BUTTONS_TO_SAVE);
    conf.setBit(0,ui->advancedPushButton->isChecked());
    conf.setBit(1,ui->clearACheckBox->isChecked());
    conf.setBit(2,ui->markACheckBox->isChecked());
    conf.setBit(3,ui->clearBCheckBox->isChecked());
    conf.setBit(4,ui->markBCheckBox->isChecked());
    conf.setBit(5,ui->selectedCheckBox->isChecked());
    conf.setBit(6,ui->differentRadioButton->isChecked());

    return conf;
}

void ComparisonDialog::setUiConf(QBitArray conf)
{
    if (BUTTONS_TO_SAVE == conf.size()) {
        ui->advancedPushButton->setChecked(conf.at(0));
        ui->clearACheckBox->setChecked(conf.at(1));
        ui->markACheckBox->setChecked(conf.at(2));
        ui->clearBCheckBox->setChecked(conf.at(3));
        ui->markBCheckBox->setChecked(conf.at(4));
        ui->selectedCheckBox->setChecked(conf.at(5));
        if (conf.at(6))
            ui->differentRadioButton->setChecked(true);
        else
            ui->sameRadioButton->setChecked(true);
    }
}

void ComparisonDialog::onTabSelection(int index)
{
    QObject *tabCombo = sender();

    if (index < tabs.size()) {

        QComboBox * entryCombo = nullptr;

        if (tabCombo == ui->tabAComboBox) {
            entryCombo = ui->entryAComboBox;
        } else {
            entryCombo = ui->entryBComboBox;
        }
        refreshEntries(entryCombo, tabs.at(index)->getBlockCount());
        connect(tabs.at(index), &TabAbstract::entriesChanged, this, &ComparisonDialog::onTabEntriesChanged,Qt::UniqueConnection);

    } else {
        guiHelper->getLogger()->logError(tr("Invalid index for tabs %1").arg(index));
    }

    checkIfComparable();
}

void ComparisonDialog::onTabEntriesChanged()
{
    TabAbstract *tab = dynamic_cast<TabAbstract *>(sender());
    if (tab == nullptr) {
        qWarning() << "[ComparisonDialog::onTabEntriesChanged] nullptr tab";
    }

    int index = tabs.indexOf(tab);
    if (index != -1) {
        if (ui->tabAComboBox->currentIndex() == index) {
            refreshEntries(ui->entryAComboBox, tabs.at(index)->getBlockCount());
        }

        if (ui->tabBComboBox->currentIndex() == index) {
            refreshEntries(ui->entryBComboBox, tabs.at(index)->getBlockCount());
        }
    } else {
       guiHelper->getLogger()->logError(tr("Tab not found for entries changed").arg(index));
    }
}

void ComparisonDialog::onCompare()
{
    TabAbstract *tabA = tabs.at(ui->tabAComboBox->currentIndex());
    TabAbstract *tabB = tabs.at(ui->tabBComboBox->currentIndex());

    ByteSourceAbstract * sourceA = tabA->getSource(ui->entryAComboBox->currentIndex());
    ByteSourceAbstract * sourceB = tabB->getSource(ui->entryBComboBox->currentIndex());
    if (sourceA == nullptr || sourceB == nullptr)
        return;

    worker = new(std::nothrow) CompareWorker(sourceA, sourceB);
    if (worker == nullptr) {
        qFatal("Cannot allocate memory for CompareWorker X{");
    }

    if (ui->selectedCheckBox->isChecked()) {
        worker->setARange(sourceA->getRealOffset(tabA->getHexTableView(ui->entryAComboBox->currentIndex())->getLowerSelected()),
                          sourceA->getRealOffset(tabA->getHexTableView(ui->entryAComboBox->currentIndex())->getHigherSelected())
                          );
        worker->setBRange(sourceB->getRealOffset(tabB->getHexTableView(ui->entryBComboBox->currentIndex())->getLowerSelected()),
                         sourceB->getRealOffset(tabB->getHexTableView(ui->entryBComboBox->currentIndex())->getHigherSelected())
                         );
    }

    worker->setMarkA(ui->markACheckBox->isChecked());
    worker->setMarkB(ui->markBCheckBox->isChecked());
    worker->setMarkSame(ui->sameRadioButton->isChecked());
    worker->setNameA(tabA->getName());
    worker->setNameB(tabB->getName());
    worker->setMarkColor(marksColor);

    if (ui->clearACheckBox->isChecked()) {
        sourceA->clearAllMarkings();
    }

    if (ui->clearBCheckBox->isChecked()) {
        sourceB->clearAllMarkings();
    }
    workerThread = new(std::nothrow) QThread();
    if (workerThread == nullptr) {
        qFatal("Cannot allocate memory for workerThread X{");
    }
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::started, worker, &CompareWorker::compare);
  //  connect(worker, &CompareWorker::progress, ui->progressBar, &QProgressBar::setValue);
    connect(worker, &CompareWorker::finishComparing, this, &ComparisonDialog::endOfComparison);

    ui->stackedWidget->setCurrentIndex(1);
    compareTimer.restart();
    workerThread->start();
    statsTimer.start(100,this);
}

void ComparisonDialog::loadTabs()
{
    tabs = guiHelper->getTabs();
    if (tabs.size() > 0) {
        setEnabled(true);
        refreshTabs(ui->tabAComboBox);
        refreshTabs(ui->tabBComboBox);

        refreshEntries(ui->entryAComboBox,tabs.at(ui->tabAComboBox->currentIndex())->getBlockCount());
        refreshEntries(ui->entryBComboBox,tabs.at(ui->tabBComboBox->currentIndex())->getBlockCount());
    } else {
        setDisabled(true);
        ui->tabAComboBox->blockSignals(true);
        ui->tabAComboBox->clear();
        ui->tabAComboBox->blockSignals(false);
        ui->tabBComboBox->blockSignals(true);
        ui->tabBComboBox->clear();
        ui->tabAComboBox->blockSignals(false);
        ui->entryAComboBox->blockSignals(true);
        ui->entryAComboBox->clear();
        ui->entryAComboBox->blockSignals(false);
        ui->entryBComboBox->blockSignals(true);
        ui->entryBComboBox->clear();
        ui->entryBComboBox->blockSignals(false);
    }

}

void ComparisonDialog::oncolorChange()
{
    marksColor = QColorDialog::getColor(Qt::yellow, this);
    changeIconColor(marksColor);
}

void ComparisonDialog::onAdvancedClicked(bool status)
{
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->advancedWidget->setVisible(status);

}

void ComparisonDialog::onEntrySelected(int)
{
    checkIfComparable();
}

void ComparisonDialog::endOfComparison(quint64 differences)
{
    worker = nullptr;
    statsTimer.stop();
    guiHelper->getLogger()->logStatus(tr("Comparison done in %1 ms").arg(compareTimer.elapsed()));
    workerThread->quit();
    workerThread->deleteLater();
    workerThread = nullptr;
    ui->stackedWidget->setCurrentIndex(0);

    if (differences == 0) {
        QMessageBox::warning(this, tr("No difference"), tr("No difference was found"), QMessageBox::Ok);
    }
    ui->resultLabel->setVisible(true);
    ui->resultLabel->setText(tr("Differences found: <b>%1</b>").arg(differences));
}

void ComparisonDialog::refreshEntries(QComboBox *entryBox, int count)
{
    QString oldSelection = entryBox->currentText();
    int index = 0;
    entryBox->clear();
    for (int i = 1; i <= count; i++){
        QString item = tr("%1").arg(i);
        entryBox->addItem(item);
        if (item == oldSelection) {
            index = i - 1;
        }
    }
    entryBox->setCurrentIndex(index);
    if (count == 1) {
        entryBox->setVisible(false);
    } else {
        entryBox->setVisible(true);
    }

    ui->entryALabel->setVisible(ui->entryAComboBox->isVisible());
    ui->entryBLabel->setVisible(ui->entryBComboBox->isVisible());


    entryBox->adjustSize();
    adjustSize();
}

void ComparisonDialog::refreshTabs(QComboBox *tabBox)
{
    QString oldSelection = tabBox->currentText();
    int index = 0;
    tabBox->blockSignals(true);
    tabBox->clear();
    for (int i = 0; i < tabs.size(); i++) {
        QString item = tabs.at(i)->getName();
        tabBox->addItem(item);
        if (item == oldSelection)
            index = i;
    }
    if (tabs.size() > 0) {
        connect(tabs.at(tabBox->currentIndex()), &TabAbstract::entriesChanged, this, &ComparisonDialog::onTabEntriesChanged,Qt::UniqueConnection);
    }
    tabBox->setCurrentIndex(index);
    tabBox->blockSignals(false);
    tabBox->adjustSize();
    adjustSize();
}

void ComparisonDialog::changeIconColor(QColor color)
{
    QPixmap pix(20,20);
    pix.fill(color);
    ui->colorPushButton->setIcon(QIcon(pix));
}

void ComparisonDialog::checkIfComparable()
{
    int indexA = ui->entryAComboBox->currentIndex();
    int indexB = ui->entryBComboBox->currentIndex();

    if (indexA < 0 || indexB < 0) {
        return; // nothing to see here
    }

    TabAbstract *tabA = tabs.at(ui->tabAComboBox->currentIndex());
    TabAbstract *tabB = tabs.at(ui->tabBComboBox->currentIndex());

    indexA = ui->entryAComboBox->currentIndex();
    indexB = ui->entryBComboBox->currentIndex();

    ByteSourceAbstract * sourceA = tabA->getSource(indexA);
    ByteSourceAbstract * sourceB = tabB->getSource(indexB);

    if (sourceA == nullptr) {
        ui->acceptPushButton->setEnabled(false);
        ui->acceptPushButton->setToolTip("Sample A is invalid");
    } else if (sourceB == nullptr) {
        ui->acceptPushButton->setEnabled(false);
        ui->acceptPushButton->setToolTip("Sample B is invalid");
    } else if (sourceA == sourceB) {
        ui->acceptPushButton->setEnabled(false);
        ui->acceptPushButton->setToolTip("The selected samples refer to the same source");
    } else if (!sourceA->hasCapability(ByteSourceAbstract::CAP_COMPARE)) {
        ui->acceptPushButton->setEnabled(false);
        ui->acceptPushButton->setToolTip("Source A is no suitable for comparison");
    } else if (!sourceB->hasCapability(ByteSourceAbstract::CAP_COMPARE)) {
        ui->acceptPushButton->setEnabled(false);
        ui->acceptPushButton->setToolTip("Source B is no suitable for comparison");
    } else {
        ui->acceptPushButton->setEnabled(true);
        ui->acceptPushButton->setToolTip("click here to begin comparison");
    }
}

void ComparisonDialog::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    if (worker != nullptr) {
        ui->progressBar->setValue(qRound(float(worker->getProgress()) * (float(100)/float(worker->getComparisonSize()))));
    }

}
QColor ComparisonDialog::getMarksColor() const
{
    return marksColor;
}

void ComparisonDialog::setMarksColor(const QColor &value)
{
    if (value.isValid()) {
        marksColor = value;
        changeIconColor(marksColor);
    }
}

ComparisonDialogStateObj::ComparisonDialogStateObj(ComparisonDialog *diag) :
    AppStateObj(diag)
{
    setName(GuiConst::STATE_COMPARISON_DIALOG);
}

ComparisonDialogStateObj::~ComparisonDialogStateObj()
{

}

void ComparisonDialogStateObj::internalRun()
{
    ComparisonDialog * diag = dynamic_cast<ComparisonDialog *>(dialog);
    if (diag == nullptr) {
        qFatal("Could not cast AppDialog to ComparisonDialog X{");
    }

    if (flags & GuiConst::STATE_LOADSAVE_COMPARISON) {
        if (flags & GuiConst::STATE_SAVE_REQUEST) {
            writer->writeAttribute(GuiConst::STATE_UI_CONF, write(diag->getUiConf()));
            writer->writeAttribute(GuiConst::STATE_MARKING_COLOR, write(diag->getMarksColor()));
        } else {

            QXmlStreamAttributes attrList = reader->attributes();
            diag->setMarksColor(readColor(attrList.value(GuiConst::STATE_MARKING_COLOR)));
            diag->setUiConf(readBitArray(attrList.value(GuiConst::STATE_UI_CONF)));
        }
    }

    QTimer::singleShot(0, diag, SLOT(loadTabs()));
}

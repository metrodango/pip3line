/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "analysedialog.h"
#include "ui_analysedialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QMutexLocker>
#include <QTimer>
#include <QTextStream>
#include <QBuffer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include "shared/guiconst.h"

using namespace GuiConst;

AnalyseDialog::AnalyseDialog(GuiHelper *guiHelper, QWidget *parent) :
    AppDialog(guiHelper, parent)
{
    ui = new(std::nothrow) Ui::AnalyseDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::AnalyseDialog X{");
    }
    ui->setupUi(this);
    ui->inputPlainTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);

    QRect dim = this->geometry();
    dim.setWidth(600);
    dim.setHeight(300);
    this->setGeometry(dim);
    connect(ui->cleanPushButton, &QPushButton::clicked, this, &AnalyseDialog::cleaning);
    connect(ui->inputPlainTextEdit, &QPlainTextEdit::customContextMenuRequested, this , &AnalyseDialog::onInputRightClick);
    connect(this, &AnalyseDialog::rejected, this, &AnalyseDialog::hide);
    connect (this, &AnalyseDialog::results, this, &AnalyseDialog::gatheringResults);
    connect(ui->charsetPushButton, &QPushButton::clicked, this, &AnalyseDialog::onCharsetAnalysis);
    connect(ui->charFrequencyPushButton, &QPushButton::clicked, this, &AnalyseDialog::onCharFrequencyAnalysis);
    connect(ui->guessHashpushButton, &QPushButton::clicked, this, &AnalyseDialog::onHashGuess);
    connect(ui->clearInputPushButton, &QPushButton::clicked, ui->inputPlainTextEdit, &QPlainTextEdit::clear);
}

AnalyseDialog::~AnalyseDialog()
{
    delete ui;
}

BaseStateAbstract *AnalyseDialog::getStateMngtObj()
{
    AnalyseDialogStateObj *stateObj = new(std::nothrow) AnalyseDialogStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for AnalyseDialogStateObj X{");
    }

    return stateObj;
}

QString AnalyseDialog::getInputValue()
{
    return ui->inputPlainTextEdit->toPlainText();
}

QString AnalyseDialog::getOutputValue()
{
    return ui->resultsPlainTextEdit->toPlainText();
}

void AnalyseDialog::setInputValue(const QString &val)
{
    ui->inputPlainTextEdit->appendPlainText(val);
}

void AnalyseDialog::setOutputValue(const QString &val)
{
    ui->resultsPlainTextEdit->appendPlainText(val);
}

void AnalyseDialog::gatheringResults(QString mess)
{
    QMutexLocker locker(&lock);
    ui->resultsPlainTextEdit->appendPlainText(mess);
}

void AnalyseDialog::cleaning()
{
    ui->resultsPlainTextEdit->clear();
}

void AnalyseDialog::onInputRightClick(QPoint pos) {
    plainTextContextMenu = ui->inputPlainTextEdit->createStandardContextMenu();
    plainTextContextMenu->addSeparator();
    plainTextContextMenu->addAction(ui->actionImportFile);

    plainTextContextMenu->exec(ui->inputPlainTextEdit->mapToGlobal(pos));
    delete plainTextContextMenu;
}

void AnalyseDialog::onCharsetAnalysis()
{

    QTimer::singleShot(0,this,SLOT(charSetAnalysis()));
}

void AnalyseDialog::on_actionImportFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"),GuiConst::GLOBAL_LAST_PATH);
    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this,tr("Error"),tr("Error while opening the file:\n %1").arg(file.errorString()),QMessageBox::Ok);
            return;
        }

        ui->inputPlainTextEdit->appendPlainText(file.readAll());
    }
}

void AnalyseDialog::charSetAnalysis()
{
    QString input = ui->inputPlainTextEdit->toPlainText();
    if (input.isEmpty()) {
        emit results(tr("No Input data"));
        return;
    }

    QMap<QChar,ulong> charAnalysis;
    QTextStream in(&input, QIODevice::ReadOnly);
    QString line;

    while (!in.atEnd()) {
        line = in.readLine();
        for (int i = 0; i < line.size(); i++) {
            if (charAnalysis.contains(line.at(i))) {
                charAnalysis[line.at(i)]++;
            }
            else {
                charAnalysis[line.at(i)] = 1;
            }
        }
    }

    QString mess;

    QMapIterator<QChar,ulong> i(charAnalysis);
    while (i.hasNext()) {
        i.next();
        mess.append(i.key());
    }

    if (!mess.isEmpty()) {
        mess.append(QString("] Size: %1").arg(charAnalysis.size()));
        emit results(mess.prepend(tr("Character set: [")));
    }

}

void AnalyseDialog::onCharFrequencyAnalysis()
{
    QTimer::singleShot(0,this,SLOT(charFrequencyAnalysis()));
}

void AnalyseDialog::charFrequencyAnalysis()
{
    QString input = ui->inputPlainTextEdit->toPlainText();
    if (input.isEmpty()) {
        emit results(tr("No Input data"));
        return;
    }

    QString mess;
    QMap<QChar,ulong> charAnalysis;
    QTextStream in(&input, QIODevice::ReadOnly);
    QString line;
    ulong totalChar = 0;

    while (!in.atEnd()) {
        line = in.readLine();
        for (int i = 0; i < line.size(); i++) {
            if (charAnalysis.contains(line.at(i))) {
                charAnalysis[line.at(i)]++;
            }
            else {
                charAnalysis[line.at(i)] = 1;
            }
            totalChar++;
        }
    }
    QList<ulong> vals = QList<ulong>::fromSet(charAnalysis.values().toSet());
    std::sort(vals.begin(),vals.end(), std::greater<ulong>());
    for (int i = 0; i < vals.size(); i++) {
        double percent = ((double)vals.at(i) * 100) / (double)totalChar;
        QList<QChar> list = charAnalysis.keys(vals.at(i));
        for (int j = 0; j < list.size(); j++)
            mess.append(QString("    \'%1\': %2 (%3%)\n").arg(list.at(j)).arg(vals.at(i)).arg(percent));
    }
    if (!mess.isEmpty())
        emit results(mess.prepend(tr("Character occurences (over %1 characters): \n").arg(totalChar)));
}

void AnalyseDialog::onHashGuess()
{
    QTimer::singleShot(0,this,SLOT(hashGuess()));
}

void AnalyseDialog::hashGuess()
{
    QList<QByteArray> input = getInput();
    if (input.size() == 0) {
        emit results(tr("No Input data"));
        return;
    }

    QMap<int,ulong> lenAnalysis;
    for (int i = 0; i < input.size(); i++) {
        int length = input.at(i).size();
        if (lenAnalysis.contains(length)) {
            lenAnalysis[length]++;
        } else {
            lenAnalysis.insert(length,1);
        }
    }

    QMapIterator<int, ulong> i(lenAnalysis);
    int count = 0;
    emit results(tr("Matching sizes found (only the first 10):"));
     while (i.hasNext() && count < 10) {
         i.next();
         emit results(tr("%1 : %3 (%2 occurrence(s))").arg(i.key()).arg(i.value()).arg(potentialHashes(i.key())));
         count++;
     }
}

QString AnalyseDialog::potentialHashes(int length)
{
    switch (length) {
        case 16:
            return "md4, mdc2, md5, ripemd, ripemd128";
        case 20:
            return "sha0, sha1, ripemd160";
        case 24:
            return "Tiger";
        case 28:
            return "sha224";
        case 32:
            return "sha256,ripemd256, GOST";
        case 40:
            return "ripemd320";
        case 48:
            return "sha384";
        case 64:
            return "sha512, Whirlpool";
        default:
            return "??";
    }
}

QList<QByteArray> AnalyseDialog::getInput()
{
    QList<QByteArray> ret;
    QList<QByteArray> temp;
    QByteArray raw = ui->inputPlainTextEdit->toPlainText().toUtf8();
    QByteArray line;
    int decode = 0;


    temp = raw.split('\n');
    if (ui->base64RadioButton->isChecked()) {
        decode = 1;
    } else if (ui->hexadecimalRadioButton->isChecked()) {
        decode = 2;
    }


    for (int i = 0; i < temp.size(); i++) {
        line = temp.at(i);
        if (line.size() != 0) {
            switch (decode) {
                case 0:
                    ret.append(line);
                    break;
                case 1:
                    ret.append(QByteArray::fromBase64(line));
                    break;
                case 2:
                    ret.append(QByteArray::fromHex(line));
                    break;
            }
        }
    }

    return ret;
}


AnalyseDialogStateObj::AnalyseDialogStateObj(AnalyseDialog *diag) :
    AppStateObj(diag)
{
    setName(GuiConst::STATE_ANALYSE_DIALOG);
}

AnalyseDialogStateObj::~AnalyseDialogStateObj()
{

}

void AnalyseDialogStateObj::internalRun()
{
    AnalyseDialog * diag = dynamic_cast<AnalyseDialog *>(dialog);
    if (diag == nullptr) {
        qFatal("Could not cast AppDialog to AnalyseDialog X{");
    }

    if (flags & GuiConst::STATE_SAVE_REQUEST) {

        QString tempS = diag->getInputValue();
        if (tempS.size() > 0)
            writer->writeAttribute(GuiConst::STATE_INPUT_CONTENT, qCompress(tempS.toUtf8()).toBase64());

        tempS = diag->getOutputValue();
        if (tempS.size() > 0)
            writer->writeAttribute(GuiConst::STATE_OUTPUT_CONTENT, qCompress(tempS.toUtf8()).toBase64());
    } else {
        if (reader->name() == name) {
            QXmlStreamAttributes attrList = reader->attributes();
            QByteArray tempB = attrList.value(GuiConst::STATE_INPUT_CONTENT).toString().toUtf8();
            if (tempB.size() > 0) {
                tempB = qUncompress(QByteArray::fromBase64(tempB));
                if (tempB.size() > 0) {
                    diag->setInputValue(QString::fromUtf8(tempB));
                }
            }

            tempB = attrList.value(GuiConst::STATE_OUTPUT_CONTENT).toString().toUtf8();
            if (tempB.size() > 0) {
                tempB = qUncompress(QByteArray::fromBase64(tempB));
                if (tempB.size() > 0) {
                    diag->setOutputValue(QString::fromUtf8(tempB));
                }
            }
        }
    }
}

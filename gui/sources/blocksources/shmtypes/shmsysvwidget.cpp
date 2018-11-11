#include "shmsysvwidget.h"

#ifdef Q_OS_UNIX
#include "sysv_shm.h"
#include "ui_shmsysvwidget.h"
#include <QDebug>
#include <QPushButton>

KeyValidator::KeyValidator(QObject *parent) : QValidator(parent)
{

}

QValidator::State KeyValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos)

    bool ok = false;

    if (input.isEmpty())
        return QValidator::Intermediate;

    convert(input, &ok);
    if (ok)
        return QValidator::Acceptable;


    return QValidator::Invalid;
}

int KeyValidator::convert(QString input, bool *ok)
{
    int ret = 0;
    if (input.startsWith("0x")) {
        input = input.mid(2);

        ret = input.toInt(ok,16);
    } else if (input.startsWith("0")) {
        input = input.mid(1);

        ret = input.toInt(ok,8);
    } else {
        ret = input.toInt(ok);
    }

    return ret;
}

ShmSysVWidget::ShmSysVWidget(SysV_Shm * obj, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShmSysVWidget),
    connector(obj)
{
    ui->setupUi(this);

    ui->keyLineEdit->setValidator(&numberVal);
    ui->shmLineEdit->setValidator(&numberVal);
    ui->permsLineEdit->setValidator(&numberVal);
    ui->fileLineEdit->setText(connector->getFilename());
    ui->keyLineEdit->setText(QString::number(connector->getKey()));
    ui->shmLineEdit->setText(QString::number(connector->getShmid()));
    ui->sizeSpinBox->setValue(connector->getSize());
    ui->permsLineEdit->setText(QString("%1").arg(connector->getPerms(),4,8, QChar('0')));
    ui->errorLabel->setVisible(false);

    connect(ui->applyPushButton, &QPushButton::clicked, this, &ShmSysVWidget::onApply);
    connect(ui->fileLineEdit, &QLineEdit::textEdited, this, &ShmSysVWidget::onFilenameChanged);
}

ShmSysVWidget::~ShmSysVWidget()
{
    delete ui;
}

void ShmSysVWidget::onApply()
{
    QString filename = ui->fileLineEdit->text();

    if (filename.isEmpty()) {
        QString key = ui->keyLineEdit->text();

        bool ok = false;

        if (!key.isEmpty()) {
            int val = KeyValidator::convert(key, &ok);
            if (ok)
                connector->setKey(val);
        }
        key = ui->shmLineEdit->text();

        if (!key.isEmpty()) {
            int val = KeyValidator::convert(key, &ok);
            if (ok)
                connector->setShmid(val);
        }


    } else {
        connector->setFilename(filename);
        ui->keyLineEdit->setText(QString::number(connector->getKey()));
        ui->shmLineEdit->setText(QString::number(connector->getShmid()));
    }

    connector->setSize(ui->sizeSpinBox->value());

    QString perms = ui->permsLineEdit->text();

    if (!perms.isEmpty()) {
        bool ok = false;
        int val = KeyValidator::convert(perms, &ok);
        if (ok) {
            qDebug() << tr("perms 0%1").arg(val,0, 8);
            connector->setPerms(val);
        }
    }

    if (connector->hasError()) {
         ui->errorLabel->setText(connector->getLastError());
         ui->errorLabel->setVisible(true);
    } else {
        ui->errorLabel->setVisible(false);
        emit requestReset();
    }
}

void ShmSysVWidget::onFilenameChanged(const QString &value)
{
    if (value.isEmpty()) {
        ui->keyLineEdit->setDisabled(false);
        ui->shmLineEdit->setDisabled(false);
    } else {
        ui->keyLineEdit->setDisabled(true);
        ui->shmLineEdit->setDisabled(true);
    }
}

void ShmSysVWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    ui->shmLineEdit->setText(QString::number(connector->getShmid()));
}

#endif

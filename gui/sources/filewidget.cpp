/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "filewidget.h"
#include "ui_filewidget.h"
#include "largefile.h"
#include "shared/searchwidget.h"
#include "guihelper.h"
#include "shared/searchresultswidget.h"
#include <QFileInfo>
#include <QTimer>
#include <QTimerEvent>
#include <QDebug>

FileWidget::FileWidget(LargeFile *fsource, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::FileWidget;
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::FileWidget X{");
    }
    source = fsource;
    ui = new(std::nothrow) Ui::FileWidget;
    ui->setupUi(this);

    refresh();
    connect(source,&LargeFile::infoUpdated, this, &FileWidget::refresh);
    connect(ui->refreshPushButton, &QPushButton::clicked, this, &FileWidget::refresh);
}

FileWidget::~FileWidget()
{
    qDebug() << this << "destroyed";
    delete ui;
}

QSize FileWidget::sizeHint() const
{
    return QSize(400,500);
}

void FileWidget::refresh()
{
    QFileInfo info(source->fileName());
//    if (parentWidget() != nullptr && QString(parentWidget()->metaObject()->className()) == "FloatingDialog")
//        parentWidget()->setWindowTitle(source->description());
    ui->pathTextLabel->setText(info.absoluteFilePath());
    ui->sizeTextLabel->setText(QString::number(info.size()).append(" bytes"));
    QString perms;
    if (info.isReadable()) {
        perms.append("Readable\n");
    }

    if (info.isWritable())
        perms.append("Writable\n");

    if (info.isExecutable())
        perms.append("Executable\n");

    if (info.isSymLink())
        perms.append("Symlink\n");

    if (info.isFile())
        perms.append("Normal file");
    else if (info.isSymLink())
        perms.append("symbolic link");
    else if (info.isDir())
        perms.append("Directory");

    if (info.isHidden())
        perms.append("Hidden file");

    ui->rightsTextLabel->setText(perms);
}



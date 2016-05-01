/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "../version.h"
#include <commonstrings.h>
#include <QStyleFactory>
#include <QStyle>
#include <QDebug>
#include <QSettings>

AboutDialog::AboutDialog(GuiHelper *guiHelper, QWidget *parent) :
    AppDialog(guiHelper, parent)
{
    ui = new(std::nothrow) Ui::AboutDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::AboutDialog X{");
    }

    ui->setupUi(this);
    ui->versionLabel->setText(tr("%1 %2 (libtransform v%3)").arg(APPNAME).arg(VERSION_STRING).arg(LIB_TRANSFORM_VERSION));
    QString info;

    info.append(tr("<p>Compiled with <b>Qt %1</b> (currently running with Qt %2)</p>").arg(QT_VERSION_STR).arg(qVersion()));
    info.append(tr("Architecture: %1bit").arg(QSysInfo::WordSize));
    info.append(tr("<p>Git branch: %1<br/>").arg(GIT_BRANCH));
    info.append(tr("Git commit: %1</p>").arg(GIT_COMMIT));
  //  QStyle *currentStyle = QApplication::style();
    //qDebug() << "Style" << currentStyle;
    info.append(tr("<p>Current style: %1</p>").arg(QApplication::style()->objectName()));
    info.append(tr("<p>Style(s) available on this platform: <ul>"));
    QStringList stylelist = QStyleFactory::keys();
    for (int i = 0; i < stylelist.size(); i++) {
        info.append("<li>");
        info.append(stylelist.at(i));
        info.append("</li>");
    }
    info.append("</ul></p>");

    info.append(tr("<p>Saved regular font: %1").arg(GuiStyles::GLOBAL_REGULAR_FONT.family()));
#ifdef Q_OS_WIN
    info.append(tr("<p>Current PATH: %1").arg(QString::fromUtf8(qgetenv("PATH"))));
    QSettings setting("HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\3.5\\", QSettings::NativeFormat);
    info.append(tr("<p>Current value: %1").arg(setting.value("InstallPath").toString()));
#endif
    ui->infoTextEdit->append(info);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

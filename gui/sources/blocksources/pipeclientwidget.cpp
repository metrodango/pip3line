#include "pipeclientwidget.h"
#include "ui_pipeclientwidget.h"
#include <QDir>
#include <QFileDialog>
#include "shared/guiconst.h"

PipeClientWidget::PipeClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PipeClientWidget)
{
    ui->setupUi(this);


    model = new(std::nothrow) PipesModel();
    if (model == nullptr) {
        qFatal("Cannot allocate memory for PipesModel X{");
    }

    proxyModel = new(std::nothrow) QSortFilterProxyModel();
    if (proxyModel == nullptr) {
        qFatal("Cannot allocate memory for QSortFilterProxyModel X{");
    }

    proxyModel->setSourceModel(model);


    QAbstractItemModel * old = ui->pipeTableView->model();
    ui->pipeTableView->setModel(proxyModel);
    delete old;

    ui->pipeTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pipeTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->pipeTableView->verticalHeader()->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    ui->pipeTableView->horizontalHeader()->setFont(GlobalsValues::GLOBAL_REGULAR_FONT);
    ui->pipeTableView->setSortingEnabled(true);

    ui->pipeTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->pipeTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->pipeTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
   // ui->pipeTableView->setColumnWidth(0, 100);
 //   ui->pipeTableView->setColumnWidth(3, 100);
   // ui->pipeTableView->horizontalHeader()->setStretchLastSection(true);


    connect(ui->refreshPushButton, &QPushButton::clicked, model, &PipesModel::refresh);
    connect(model, &PipesModel::modelReset, this, &PipeClientWidget::onModelUpdated);
    connect(ui->pipeTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PipeClientWidget::onSelectionChanged);
    connect(ui->nameLineEdit, &QLineEdit::textChanged, this, &PipeClientWidget::onNamedPipeChanged);

    model->refresh();
}

PipeClientWidget::~PipeClientWidget()
{
    delete ui;
    delete proxyModel;
    delete model;
}

void PipeClientWidget::onModelUpdated()
{
    ui->pipeTableView->resizeColumnToContents(0);
    ui->pipeTableView->resizeColumnToContents(1);
    qDebug() << tr("Column 0 resized");
}

void PipeClientWidget::onSelectPipe()
{
    QString path = QDir::home().absolutePath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Pipe"), path);

    if (!fileName.isEmpty()) {
        ui->nameLineEdit->setText(fileName);
        emit newPipeName(fileName);
    }
}

void PipeClientWidget::onSelectionChanged(const QItemSelection &selection)
{
    if (!selection.isEmpty()) {
        QModelIndex last_i = selection.last().topLeft();
        if (last_i.isValid()) {
            QVariant path = last_i.data();
            ui->nameLineEdit->setText(path.toString());
        }
    }
}

void PipeClientWidget::onNamedPipeChanged(const QString &value)
{
    ui->nameLineEdit->setStyleSheet(GuiStyles::LineEditOk);
    ui->nameLineEdit->setToolTip(QString());
    if (value.startsWith(QChar('@'))) {
        // abstract socket, nothing to be done
        ui->nameLineEdit->setStyleSheet(GuiStyles::LineEditError);
        ui->nameLineEdit->setToolTip(QString("Abstract Socket, cannot connect to it"));
    } else {
        #if defined(Q_OS_LINUX)
        QFileInfo fi(value);
        #elif defined(Q_OS_WIN32)
        QFileInfo fi(QString("\\\\.\\Pipe\\").append(value));
        #elif defined(Q_OS_OSX)
        QFileInfo fi(value);
        //qCritical() << tr("[PipesModel::onNamedPipeChanged] Not implemented on Mac platform");
        #endif
        if (!fi.exists()) {
            ui->nameLineEdit->setStyleSheet(GuiStyles::LineEditError);
            ui->nameLineEdit->setToolTip(QString("Socket Path does not exist"));
        } else {
            emit newPipeName(value);
        }
    }
}

const int PipesModel::COLUMN_COUNT = 4;
const QStringList PipesModel::headersList = QStringList() << "Path" << "perms" << "process" << "bonus";

PipesModel::PipesModel(QObject *parent) :
    QAbstractTableModel(parent)
{

}

PipesModel::~PipesModel()
{

}

int PipesModel::columnCount(const QModelIndex &) const
{
    return COLUMN_COUNT;
}

int PipesModel::rowCount(const QModelIndex &) const
{
    return pipesList.size();
}

QVariant PipesModel::data(const QModelIndex &index, int role) const
{
    int column = index.column();
    int row = index.row();

    if (row < pipesList.size() && role == Qt::DisplayRole) {
        switch (column) {
            case 0:
                return pipesList.at(row).path;
            case 1:
                return pipesList.at(row).perms;
            case 2:
                return pipesList.at(row).processOwner;
            case 3:
                return pipesList.at(row).bonus;
            default:
                return GuiConst::UNDEFINED_TEXT;
        }
    } else if (role == Qt::FontRole) {
        return GlobalsValues::GLOBAL_REGULAR_FONT;
    } else if (role == Qt::SizeHintRole) {
        switch (column) {
            case 0:
                return QSize(300,20);
            case 1:
                return 100;
            case 2:
                return 100;
            default:
                return 100;
        }
    }

    return QVariant();
}

QVariant PipesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return headersList.at(section);
        } else {
            return QString("%1").arg(section);
        }
    }
    return QVariant();
}

void PipesModel::refresh()
{
    beginResetModel();

    pipesList.clear();

#if defined(Q_OS_LINUX)
    QFile linuxSockets("/proc/net/unix");
    if (linuxSockets.open(QIODevice::ReadOnly)) {
        QByteArray data = linuxSockets.readAll();
        linuxSockets.close();
        bool ok = false;
        QMultiHash<quint64, ProcessProperties> procList = getProcList();

        QList<QByteArray> lines = data.split('\n');
        if (lines.size() > 0) {
            lines.takeAt(0); // removing headers
            QByteArray l;
            foreach (l,  lines) {
                QList<QByteArray> fields = l.split(' ');
                int i = 0;
                while (i < fields.size()) {
                    if (fields.at(i).isEmpty())
                        fields.takeAt(i);
                    else
                        i++;
                }
                if (fields.size() > 6 &&
                        fields.at(3) == QByteArray("00010000") &&
                        fields.at(4) == QByteArray("0001")) {
                    PipePropperties prop;
                //    qDebug() << l << fields.size();
                    if (fields.size() > 7) {
                        prop.path = QString::fromUtf8(fields.at(7)); // pathname
                    } else  {
                        prop.path = QString::fromUtf8(fields.at(6)); // inode
                    }

                    prop.inode = fields.at(6).toULong(&ok);
                    if (!ok) qCritical() << tr("[PipesModel::refresh] Cannot parse inode");

                    QFileInfo fi(prop.path);
                    if (fi.exists()) {
                        QFile::Permissions perms = fi.permissions();
                        prop.perms = QString();
                        prop.perms.append((perms & QFileDevice::ReadUser) ? "r" : "-");
                        prop.perms.append((perms & QFileDevice::WriteUser) ? "w" : "-");
                        prop.perms.append((perms & QFileDevice::ExeUser) ? "x" : "-");
                        prop.perms.append((perms & QFileDevice::ReadGroup) ? "r" : "-");
                        prop.perms.append((perms & QFileDevice::WriteGroup) ? "w" : "-");
                        prop.perms.append((perms & QFileDevice::ExeGroup) ? "x" : "-");
                        prop.perms.append((perms & QFileDevice::ReadOther) ? "r" : "-");
                        prop.perms.append((perms & QFileDevice::WriteOther) ? "w" : "-");
                        prop.perms.append((perms & QFileDevice::ExeOther) ? "x" : "-");
                        prop.perms.append("|");
                        prop.perms.append(QString("%1(%2)").arg(fi.owner()).arg(fi.ownerId()));
                        prop.perms.append(":");
                        prop.perms.append(QString("%1(%2)").arg(fi.group()).arg(fi.groupId()));
                    } else {
                        prop.perms = QString("Inexistent path");
                    }
                    prop.bonus = tr("%1").arg(prop.inode);
                    QList<ProcessProperties> assocL = procList.values(prop.inode);
                    for (int i = 0; i < assocL.size(); i++) {
                        ProcessProperties proc = assocL.at(i);
                        prop.processOwner.append(QString("%1(%2)\n").arg(proc.desc).arg(proc.pid));
                    }
                    pipesList.append(prop);
                }
            }


        } else {
            qCritical() << tr("[PipesModel::refresh] /proc/net/unix was empty T_T");
        }
    } else {
        qCritical() << tr("[PipesModel::refresh] Cannot open /proc/net/unix for reading");
    }
#elif defined(Q_OS_WIN32)
    QDir dpipes("\\\\.\\Pipe\\");
    QFileInfoList slist = dpipes.entryInfoList();
    for (int j = 0; j < slist.size(); j++) {
        QFileInfo fdi = slist.at(j);
        PipePropperties pp;
        pp.path = fdi.baseName();
        pipesList.append(pp);
    }
#elif defined(Q_OS_OSX)
    qCritical() << tr("[PipesModel::refresh] Not implemented on Mac platform, if someone knows how to get the global list if UNIX socket on OSX, please give me a shout.");
#endif
    endResetModel();
}

QMultiHash<quint64, PipesModel::ProcessProperties> PipesModel::getProcList()
{
    QMultiHash<quint64, ProcessProperties> procList;

#if defined(Q_OS_LINUX)
    QDir dproc("/proc");
    QFileInfoList dlist = dproc.entryInfoList(QDir::Dirs);
    for (int i = 0; i < dlist.size(); i++) {
        QFileInfo fi = dlist.at(i);
        bool ok = false;
        pid_t pid = fi.baseName().toInt(&ok);
        if (ok) { // checking if it is a number
            QDir dfd(QString("%1/fd").arg(fi.absoluteFilePath()));
            QString cmdline = QString("/proc/%1/cmdline").arg(pid);
            QFile cmdf(cmdline);
            ProcessProperties pp;
            if (!cmdf.open(QIODevice::ReadOnly)) {
                qCritical() << tr("Cannot open file: %1").arg(cmdline);
            } else {
                QByteArray data = cmdf.readAll();
                pp.desc = QString::fromUtf8(data).split(' ').at(0);
                cmdf.close();
            }
            QFileInfoList slist = dfd.entryInfoList();
            for (int j = 0; j < slist.size(); j++) {
                QFileInfo fdi = slist.at(j);
                QRegExp socketRegexp("socket:\\[(\\d+)\\]");
                if (fdi.isSymLink() && socketRegexp.indexIn(fdi.symLinkTarget()) != -1) {
                    quint64 inode = socketRegexp.cap(1).toULong(&ok);
                    if (ok) {
                        pp.pid = pid;
                        procList.insert(inode, pp);
                      //  qDebug() << fdi.symLinkTarget();
                    }
                }
            }
        }
    }

//    QHashIterator<quint64, ProcessProperties> k(procList);
//    while (k.hasNext()) {
//        k.next();
//        qDebug() << k.key() << ": " << k.value().pid << k.value().desc << endl;
//    }

#elif defined(Q_OS_WIN32)
    qCritical() << tr("[PipesModel::getProcList] Not implemented on Windows platform");
#elif defined(Q_OS_OSX)
    qCritical() << tr("[PipesModel::getProcList] Not implemented on Mac platform");
#endif
    return procList;
}

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SUBSTITUTIONWIDGET_H
#define SUBSTITUTIONWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include "../substitution.h"

namespace Ui {
    class SubstitutionWidget;
}

class SubstitutionTables : public QAbstractTableModel {
        Q_OBJECT
    public:
        static const int S_TABLE_SIZE;
        static const int S_ARRAY_SIZE;
        explicit SubstitutionTables();
        ~SubstitutionTables();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;

        void setRawData(QByteArray sTable);
        QByteArray getRawData();
    private:
        void highlighting();
        QByteArray array;
        QVector<bool> highlight;
};

class SubstitutionWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit SubstitutionWidget(Substitution * transform, QWidget *parent = 0);
        ~SubstitutionWidget();
    private slots:
        void onModify();
        void onChooseTable(QString name);
        void on_exportPushButton_clicked();

        void on_importPushButton_clicked();

    private:
        static const QByteArray HEXCHAR;
        Ui::SubstitutionWidget *ui;
        SubstitutionTables sModel;
        Substitution *transform;
};



#endif // SUBSTITUTIONWIDGET_H

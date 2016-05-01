/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef COMPARISONDIALOG_H
#define COMPARISONDIALOG_H

#include "appdialog.h"
#include <QList>
#include <QColor>
#include <QTime>
#include <QBitArray>
#include "sources/bytesourceabstract.h"

namespace Ui {
class ComparisonDialog;
}

class QComboBox;
class TabAbstract;
class ByteSourceAbstract;
class QThread;
class BytesRange;

class CompareWorker : public QObject
{
        Q_OBJECT
    public:
        explicit CompareWorker(ByteSourceAbstract * sA, ByteSourceAbstract * sB, QObject *parent = 0);
        ~CompareWorker();
        quint64 getStartA() const;
        void setARange(const quint64 start, const quint64 end);
        quint64 getStartB() const;
        void setBRange(const quint64 start, const quint64 end);
        quint64 getSizeA() const;
        quint64 getSizeB() const;
        bool getMarkA() const;
        void setMarkA(bool value);
        bool getMarkB() const;
        void setMarkB(bool value);
        bool getMarkSame() const;
        void setMarkSame(bool value);
        quint64 getEndA() const;
        quint64 getEndB() const;
        QString getNameA() const;
        void setNameA(const QString &value);
        QString getNameB() const;
        void setNameB(const QString &value);
        QColor getMarkColor() const;
        void setMarkColor(const QColor &value);

        int getResultDifferences() const;

    public slots:
        void compare();
        void stop();
    signals:
        void newMarkingsA(BytesRangeList *);
        void newMarkingsB(BytesRangeList *);
        void finishComparing(int differences);
        void progress(int percent);
    private:
        void endingThread();
        void markingA(quint64 offset);
        void endAMarking();
        void markingB(quint64 offset);
        void endBMarking();
        ByteSourceAbstract * sourceA;
        ByteSourceAbstract * sourceB;
        quint64 startA;
        quint64 startB;
        quint64 endA;
        quint64 endB;
        quint64 sizeA;
        quint64 sizeB;
        bool markA;
        bool markB;
        QString nameA;
        QString nameB;
        bool markSame;
        bool stopped;
        QColor marksColor;
        QString tooltipDiffA;
        QString tooltipDiffB;
        BytesRangeList *rangesA;
        BytesRangeList *rangesB;
        bool ismarkingA;
        quint64 markerStartA;
        quint64 markerEndA;
        quint64 markerStartB;
        quint64 markerEndB;
        bool ismarkingB;
        int resultDifferences;
};

class ComparisonDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit ComparisonDialog(GuiHelper *guiHelper ,QWidget *parent = 0);
        ~ComparisonDialog();
        BaseStateAbstract *getStateMngtObj();
        QBitArray getUiConf() const;
        void setUiConf(QBitArray conf);
        QColor getMarksColor() const;
        void setMarksColor(const QColor &value);

    public slots:
        void loadTabs();
    private slots:
        void onTabSelection(int index);
        void onTabEntriesChanged();
        void onCompare();
        void oncolorChange();
        void onAdvancedClicked(bool status);
        void onEntrySelected(int index);
        void endOfComparison(int differences);
    private:
        void refreshEntries(QComboBox *entryBox, int count);
        void refreshTabs(QComboBox *tabBox);
        void changeIconColor(QColor color);
        void checkIfComparable();
        Ui::ComparisonDialog *ui;
        QList<TabAbstract *> tabs;
        QColor marksColor;
        QThread * workerThread;
        QTime compareTimer;
        static const int BUTTONS_TO_SAVE;
};

class ComparisonDialogStateObj : public AppStateObj
{
        Q_OBJECT
    public:
        explicit ComparisonDialogStateObj(ComparisonDialog *diag);
        ~ComparisonDialogStateObj();
    private:
        void internalRun();
};

#endif // COMPARISONDIALOG_H

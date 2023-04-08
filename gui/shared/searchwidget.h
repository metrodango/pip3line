/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QLineEdit>
#include <QWidget>
#include <QColor>
#include <QBitArray>
#include <QAbstractListModel>
#include <QTime>
#include <QBasicTimer>
#include <QElapsedTimer>
#include "../sources/bytesourceabstract.h"

class GuiHelper;
class ByteSourceAbstract;
class QFocusEvent;
class QPushButton;
class QEvent;
class SearchAbstract;
class SearchResultsWidget;
class BytesRange;
class FloatingDialog;

class FoundOffsetsModel : public QAbstractListModel
{
        Q_OBJECT
    public:
        static const int INVALID_POS;
        explicit FoundOffsetsModel(QObject * parent = nullptr);
        ~FoundOffsetsModel();
        void setSearchDelegate(SearchAbstract * searchDelegate);
        SearchAbstract *getSearchObject() const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        quint64 getStartingOffset(const QModelIndex & index);
        quint64 getStartingOffset(const int index);
        quint64 getEndOffset(const QModelIndex & index);
        static bool lessThanFoundOffset(BytesRange* i1, BytesRange* i2);
        void startSearch();
        int elapsed();
        BytesRangeList *getRanges() const;
        int getLastSelected() const;
        void setLastSelected(int value);
        int size();
    signals:
        void updated();
        void lastUpdated(int pos);
    public slots:
        void clear();
        void setNewList(BytesRangeList *list);
        void onRangeDestroyed();
    private:
        Q_DISABLE_COPY(FoundOffsetsModel)
        BytesRangeList * ranges;
        SearchAbstract * searchDelegate;
        QElapsedTimer searchTimer;
        int lastSelected;
};

class SearchLine : public QLineEdit
{
        Q_OBJECT
    public:
        explicit SearchLine(ByteSourceAbstract *source, QWidget *parent = nullptr);
        ~SearchLine();
        ByteSourceAbstract *getBytesource() const;
        void setBytesource(ByteSourceAbstract *source);
        void updateProgress(quint64 val);
    public slots:
        void setError(bool val);
        void onSearchStarted();
        void onSearchEnded();
    private slots:
        void onSourceUpdated(quintptr);
    signals:
        void newSearch(QString val, unsigned int modifiers);
        void requestJumpToNext();
    private:
        static const int MAX_TEXT_SIZE;
        void focusInEvent(QFocusEvent * event);
        void paintEvent(QPaintEvent * event);
        void keyPressEvent(QKeyEvent * event);
        double progress;
        bool searching;
        quint64 sourceSize;
        ByteSourceAbstract *bytesource;
        static const QColor LOADING_COLOR;
};

class SearchWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit SearchWidget(ByteSourceAbstract *source, GuiHelper *nguiHelper, QWidget *parent = nullptr);
        ~SearchWidget();
        QString text();
        void setText(QString data);
        void setStopVisible(bool val);
        FoundOffsetsModel *getModel() const;
        ByteSourceAbstract *getBytesource() const;

        SearchAbstract *getSearchDelegate() const;
        void setSearchDelegate(SearchAbstract *delegate);

        bool isViewIsText() const;
        void setViewIsText(bool value);

    public slots:
        void setError(bool val);
        void onSearchStarted();
        void onSearchEnded();
        void clearSearch();
        void nextFind(quint64 pos = 0);
    private slots:
        void onSearch(QString val, unsigned int modifiers);
        void onAdvanced();
        void processJump(quint64 start, quint64 end);
        void onRequestNext();
    signals:
        void searchRequest(QByteArray data,QBitArray mask, bool couldBeText);
        void stopSearch();
        void jumpTo(quint64 start, quint64 end);
    private:
        void timerEvent(QTimerEvent *event);
        static const QString FIND_PLACEHOLDER_TEXT;
        static const QString TOOLTIP_TEXT;
        static const QString PLACEHOLDER_DISABLED_TEXT;
        SearchLine * lineEdit;
        QPushButton *stopPushButton;
        QPushButton *advancedPushButton;
        FoundOffsetsModel *model;
        SearchResultsWidget * resultWidget;
        ByteSourceAbstract *bytesource;
        SearchAbstract * searchDelegate;
        GuiHelper *guiHelper;
        FloatingDialog *advancedSearchDialog;
        quint64 lastJumpStart;
        bool viewIsText;
        QBasicTimer statsTimer;
};

#endif // SEARCHWIDGET_H

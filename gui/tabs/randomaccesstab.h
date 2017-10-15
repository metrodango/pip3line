/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef RANDOMACCESSTAB_H
#define RANDOMACCESSTAB_H

#include "tababstract.h"
#include <commonstrings.h>
#include <QBitArray>

namespace Ui {
class RandomAccessTab;
}

class OffsetGotoWidget;
class SearchWidget;
class ByteSourceAbstract;
class HexView;
class GuiHelper;
class ReadOnlyButton;
class ClearAllMarkingsButton;
class ByteSourceGuiButton;
class DetachTabButton;
class ByteTableView;

class RandomAccessTab : public TabAbstract
{
        Q_OBJECT
    public:
        explicit RandomAccessTab(ByteSourceAbstract *nbytesource, GuiHelper *guiHelper, QWidget *parent = 0);
        ~RandomAccessTab();
        int getBlockCount() const;
        ByteSourceAbstract *getSource(int blockIndex);
        ByteTableView *getHexTableView(int blockIndex);
        void loadFromFile(QString);
        void setData(const QByteArray &);
        bool canReceiveData();
        BaseStateAbstract *getStateMngtObj();

        OffsetGotoWidget *getGotoWidget() const;
        SearchWidget *getSearchWidget() const;

    private slots:
        void fileLoadRequest();
        void onSearch(QByteArray item, QBitArray mask,  bool);
        void onGotoOffset(quint64 offset, bool absolute, bool negative, bool select);
        void log(QString mess,QString source,Pip3lineConst::LOGLEVEL level);
        void onCloseLogView();
        void onSourceUpdated();

    private:
        Q_DISABLE_COPY(RandomAccessTab)
        static const QString LOGID;
        void integrateByteSource();
        ByteSourceAbstract *bytesource;
        HexView *hexView;
        OffsetGotoWidget * gotoWidget;
        SearchWidget *searchWidget;
        Ui::RandomAccessTab *ui;
        ReadOnlyButton * roButton;
        ClearAllMarkingsButton * clearAllMarksButton;
        ByteSourceGuiButton * guiButton;
        DetachTabButton *detachButton;
        bool eventFilter(QObject *obj, QEvent *event);

        friend class RandomAccessStateObj;
        friend class RandomAccessClosingStateObj;
};

class RandomAccessStateObj : public TabStateObj
{
        Q_OBJECT
    public:
        explicit RandomAccessStateObj(RandomAccessTab *tab);
        ~RandomAccessStateObj();
        void run();
};

class RandomAccessClosingStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit RandomAccessClosingStateObj(RandomAccessTab *tab);
        ~RandomAccessClosingStateObj();
        void run();
        void setScrollIndex(int value);

    protected:
        RandomAccessTab *tab;
        int scrollIndex;
};

#endif // RANDOMACCESSTAB_H

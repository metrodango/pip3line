#ifndef FILTERITEM_H
#define FILTERITEM_H

#include <QBitArray>
#include <QObject>
#include <QHash>
#include <QByteArray>

class FilterItem : public QObject
{
        Q_OBJECT
    public:
        enum SEARCH_TYPE {
            HEXA = 0,
            UTF8 = 1,
            UTF16 = 2
        };
        explicit FilterItem(QObject *parent = nullptr);
        FilterItem(const FilterItem &other);
        virtual ~FilterItem();

        QString getName() const;
        void setName(const QString &value);
        QByteArray getValue() const;
        void setValue(const QByteArray &value);
        int getStartingOffset() const;
        void setStartingOffset(const int &value);
        bool isReverseSelection() const;
        void setReverseSelection(bool value);
        void setBitMask(QBitArray bitMask);
        virtual bool selectable(QByteArray data);
        virtual FilterItem& operator = (FilterItem const &other);
        virtual bool operator == (FilterItem const &other) const;
        FilterItem::SEARCH_TYPE getSearchType() const;
        void setSearchType(const FilterItem::SEARCH_TYPE &value);
        bool isValid();
        QString getHexValueWithMask() const;

        QHash<QString, QString> getConfiguration();
        void setConfiguration(const QHash<QString, QString> &conf);

    private:
        QString name;
        QByteArray value;
        int startingOffset;
        bool reverseSelection;
        char *mask;
        FilterItem::SEARCH_TYPE searchType;
};

#endif // FILTERITEM_H

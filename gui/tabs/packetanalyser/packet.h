/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PACKET_H
#define PACKET_H

#include <QByteArray>
#include <QColor>
#include "sources/bytesourceabstract.h"
#include <QObject>

class Block;

class Packet : public QObject
{
        Q_OBJECT
    public:
        enum Direction {RIGHTLEFT = 0, LEFTRIGHT = 1, NODIRECTION = -1};
        static const int USER_SOURCE;
        explicit Packet(QByteArray data = QByteArray(), int sourceid = USER_SOURCE);
        explicit Packet(Block *block);
        ~Packet();

        QByteArray getOriginalData() const;
        void setOriginalData(const QByteArray &value, bool setDataAsWell = true);
        QByteArray getData() const;
        void setData(const QByteArray &value, bool setOriginalAsWell = false);
        QList<ByteSourceAbstract::HistItem> getHistory() const;
        void setHistory(const QList<ByteSourceAbstract::HistItem> &value);
        int getSourceid() const;
        void setSourceid(int value);
        QString getSourceString() const;
        void setSourceString(const QString &value);
        QDateTime getTimestamp() const;
        void setTimestamp(const QDateTime &value);
        Direction getDirection() const;
        void setDirection(const Direction &value);
        int size();
        Block * toBlock();
        bool hasBeenModified() const;

        QHash<QString, QString> getAdditionalFields() const;
        void setAdditionalFields(const QHash<QString, QString> &value);
        QString fetchAdditionalField(const QString &fieldName) const;
        void setAdditionalField(const QString &FieldName, const QString & value);
        void removeAdditionalFields(const QString &FieldName);

        QColor getForeground() const;
        void setForeground(const QColor &value);
        QColor getBackground() const;
        void setBackground(const QColor &value);

        quint32 getMicrosec() const;
        void setMicrosec(const quint32 &value);

        QString getComment() const;
        void setComment(const QString &value);

        bool isInjected() const;
        void setInjected(bool value);

    signals:
        void updated();
    private:
        QByteArray originalData;
        QByteArray data;
        QList<ByteSourceAbstract::HistItem> history;
        int sourceid;
        QString sourceString;
        QDateTime timestamp;
        quint32 microsec;
        Direction direction;
        QString comment;
        QColor foreground;
        QColor background;
        bool injected;
        QHash<QString, QString> additionalFields;
};

#endif // PACKET_H

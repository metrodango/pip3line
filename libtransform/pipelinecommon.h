/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef PIPELINECOMMON_H
#define PIPELINECOMMON_H

#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QObject>
#include <QHash>
#include <QSharedPointer>
#include "libtransform_global.h"

namespace Pip3lineCommon {

    enum HistAction {INSERT = 0, REMOVE = 1, REPLACE = 2};

    struct HistItem {
           HistAction action;
           quint64 offset;
           QByteArray before;
           QByteArray after;
    };

    enum DataProtocol {
        unknown_protocol = 0,
        dtls_protocol = 1,
        stun_protocol = 2
    };

    class LIBTRANSFORMSHARED_EXPORT Block {
        public:
            static const int INVALID_ID;
            explicit Block(QByteArray data, int sourceid);
            ~Block();
            QByteArray getData() const;
            void setData(const QByteArray &value);
            int getSourceid() const;
            void setSourceid(int value);
            DataProtocol getProtocol() const;
            void setProtocol(const DataProtocol &value);
            quint16 getSrcPort() const;
            void setSrcPort(const quint16 &value);
            quint16 getDstPort() const;
            void setDstPort(const quint16 &value);
            bool isMultiplexed() const;
            void setMultiplexed(bool value);
        private:
            Q_DISABLE_COPY(Block)
            QByteArray data;
            int sourceid;
            DataProtocol protocol;
            quint16 srcPort;
            quint16 dstPort;
            bool multiplexed;
    };

    class LIBTRANSFORMSHARED_EXPORT Packet : public QObject
    {
            Q_OBJECT
        public:
            enum Direction {RIGHTLEFT = 0, LEFTRIGHT = 1, NODIRECTION = -1};
            static const int USER_SOURCE;
            explicit Packet(QByteArray data = QByteArray(), int sourceid = USER_SOURCE);
            explicit Packet(Block *block);
            ~Packet();
            Packet * clone();
            QByteArray getOriginalData() const;
            void setOriginalData(const QByteArray &value, bool setDataAsWell = true);
            QByteArray getData() const;
            void setData(const QByteArray &value, bool setOriginalAsWell = false);
            QList<Pip3lineCommon::HistItem> getHistory() const;
            void setHistory(const QList<Pip3lineCommon::HistItem> &value);
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
            static bool lessThanFunc(QSharedPointer<Packet> or1, QSharedPointer<Packet> or2);
            DataProtocol getProtocol() const;
            void setProtocol(const DataProtocol &value);
            quint16 getSrcPort() const;
            void setSrcPort(const quint16 &value);
            quint16 getDstPort() const;
            void setDstPort(const quint16 &value);
            bool isMultiplexed() const;
            void setMultiplexed(bool value);

        signals:
            void updated();
        private:
            Q_DISABLE_COPY(Packet)
            QByteArray originalData;
            QByteArray data;
            QList<Pip3lineCommon::HistItem> history;
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
            DataProtocol protocol;
            quint16 srcPort;
            quint16 dstPort;
            bool multiplexed;
    };
}

#endif // PIPELINECOMMON_H

#include "packetstyleditemdelegate.h"
#include <QDateTime>
#include <QDebug>



PacketStyledItemDelegate::PacketStyledItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

PacketStyledItemDelegate::~PacketStyledItemDelegate()
{
    qDebug() << "destroying" << this;
}

QString PacketStyledItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    return QStyledItemDelegate::displayText(value,locale);
}


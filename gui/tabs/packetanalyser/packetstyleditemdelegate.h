#ifndef PACKETSTYLEDITEMDELEGATE_H
#define PACKETSTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

class PacketStyledItemDelegate : public QStyledItemDelegate
{

    public:
        explicit PacketStyledItemDelegate(QObject * parent = nullptr);
        ~PacketStyledItemDelegate();
        QString displayText(const QVariant &value, const QLocale &locale) const;
};

#endif // PACKETSTYLEDITEMDELEGATE_H

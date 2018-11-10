#ifndef PACKETSTYLEDITEMDELEGATE_H
#define PACKETSTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

class PacketStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    public:
        explicit PacketStyledItemDelegate(QObject * parent = nullptr);
        ~PacketStyledItemDelegate();
        QString displayText(const QVariant &value, const QLocale &locale) const;
    private:
        Q_DISABLE_COPY(PacketStyledItemDelegate)
};

#endif // PACKETSTYLEDITEMDELEGATE_H

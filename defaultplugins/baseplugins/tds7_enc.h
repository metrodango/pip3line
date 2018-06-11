#ifndef TDS7_ENC_H
#define TDS7_ENC_H

#include <transformabstract.h>

class TDS7_ENC : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit TDS7_ENC();
        ~TDS7_ENC();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QString help() const;
        static const QString id;
    signals:

    public slots:
};

#endif // TDS7_ENC_H

#ifndef TDS7_ENC_H
#define TDS7_ENC_H

#include <transformabstract.h>

class TDS7_ENC : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit TDS7_ENC();
        ~TDS7_ENC() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QString help() const override;
        static const QString id;
};

#endif // TDS7_ENC_H

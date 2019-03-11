#ifndef CRC32_H
#define CRC32_H

#include <transformabstract.h>

class Crc32 : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        static const quint32 crctable[];
        explicit Crc32();
        ~Crc32() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QWidget * requestGui(QWidget * parent) override;
        QString help() const override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;

        bool isLittleendian() const;
        void setLittleendian(bool value);
        bool getAppendToInput() const;
        void setAppendToInput(bool value);
        bool isHexOutput() const;
        void setHexOutput(bool value);

    private:
        bool littleendian;
        bool hexOutput;
        bool appendToInput;
};

#endif // CRC32_H

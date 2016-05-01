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
        ~Crc32();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        QString help() const;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);

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

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef DISTORMTRANSF_H
#define DISTORMTRANSF_H

#include <transformabstract.h>

class DistormTransf : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        enum ASMType { A16Bits = 2, A32Bits = 4, A64Bits = 8};
        explicit DistormTransf();
        ~DistormTransf();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);

        void setDecodeType(ASMType val);
        ASMType getDecodeType() const;
        void setOffset(quint64 val);
        quint64 getOffset() const;
        void setMaxInstruction(uint val);
        uint getMaxInstruction() const;

        void setShowOffset(bool val);
        bool getShowOffset() const;

        void setShowOpcodes(bool val);
        bool getShowOpcodes() const;
        QString help() const;
    private:
        Q_DISABLE_COPY(DistormTransf)
        static const QString XMLASMTYPE;
        static const QString XMLMAXINSTRUCTION;
        static const QString XMLSHOWOFFSET;
        static const QString XMLSHOWOPCODES;
        quint64 codeOffset;
        ASMType asmType;
        uint maxInstruction;
        bool showoffset;
        bool showopcodes;
};

#endif // DISTORM_H

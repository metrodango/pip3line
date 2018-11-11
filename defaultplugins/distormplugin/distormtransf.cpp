/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "distormtransf.h"
#include <distorm.h>
#include "distormwidget.h"
#include <QDebug>

const QString DistormTransf::id = "Distorm";
const QString DistormTransf::XMLASMTYPE = "AsmType";
const QString DistormTransf::XMLMAXINSTRUCTION = "MaxInstruction";
const QString DistormTransf::XMLSHOWOFFSET = "ShowOffset";
const QString DistormTransf::XMLSHOWOPCODES = "ShowOpcodes";

DistormTransf::DistormTransf()
{
    codeOffset = 0;
    asmType = A32Bits;
    maxInstruction = 200;
    showoffset = true;
    showopcodes = true;
}

DistormTransf::~DistormTransf()
{
}

QString DistormTransf::name() const
{
    return id;
}

QString DistormTransf::description() const
{
    return tr("Distorm disassembler");
}

void DistormTransf::transform(const QByteArray &input, QByteArray &output)
{
    QByteArray temp;
    _DecodeType dt = Decode32Bits;
    int maxOffsetSize = sizeof(_OffsetType);
    switch (asmType) {
        case A16Bits:
            dt = Decode16Bits;
            maxOffsetSize = 4;
            break;
        case A32Bits:
            dt = Decode32Bits;
            maxOffsetSize = 8;
            break;
        case A64Bits:
            dt = Decode64Bits;
            maxOffsetSize = 16;
            break;
    }


    unsigned int decodedInstruction = 0;

    _DecodedInst * resultInstr = new _DecodedInst[maxInstruction];
#ifdef SUPPORT_64BIT_OFFSET
    _DecodeResult result = distorm_decode64(static_cast<_OffsetType>(codeOffset), reinterpret_cast<const unsigned char*>(input.constData()), input.size(), dt, resultInstr, maxInstruction, &decodedInstruction);
#else
    _DecodeResult result = distorm_decode32(static_cast<_OffsetType>(codeOffset), reinterpret_cast<const unsigned char*>(input.constData()), input.size(), dt, resultInstr, maxInstruction, &decodedInstruction);
#endif
    if (result == DECRES_SUCCESS || result == DECRES_MEMORYERR) {
        if (result == DECRES_MEMORYERR) {
            emit error(tr("Result instruction array not big enough (%1)").arg(maxInstruction), id);
        }


        int entrySize = 0;
        for (uint i = 0; i < decodedInstruction; i++) {
            _DecodedInst instruction = resultInstr[i];

            if (showoffset) {
                output.append("0x");
                temp = QByteArray::number(static_cast<qulonglong>(instruction.offset),16);
                entrySize = temp.size();
                for (int j = 0; j < maxOffsetSize - entrySize; j++) {
                    temp.prepend('0');
                }
                output.append(temp);
                output.append(' ');
            }

            int length = 0;
            if (showopcodes) {

                if (instruction.instructionHex.length < MAX_TEXT_SIZE) {
                    length = static_cast<int>(instruction.instructionHex.length);
                } else {
                    length = 0;
                    emit error(tr("Result instruction length Invalid"), id);
                }
                temp = QByteArray(reinterpret_cast<char *>(instruction.instructionHex.p),length);
                entrySize = temp.size();
                for (int j = 0; j < maxOffsetSize + 2 - entrySize; j++) {
                    temp.append(' ');
                }
                output.append(temp);
                output.append(' ');
            }

            if (instruction.mnemonic.length < MAX_TEXT_SIZE) {
                length = static_cast<int>(instruction.mnemonic.length);
            } else {
                length = 0;
                emit error(tr("Result mnemonic length Invalid"), id);
            }
            output.append(reinterpret_cast<char *>(instruction.mnemonic.p),length);
            output.append(' ');
            if (instruction.operands.length < MAX_TEXT_SIZE) {
                length = static_cast<int>(instruction.operands.length);
            } else {
                length = 0;
                emit error(tr("Result operands length Invalid"), id);
            }
            output.append(reinterpret_cast<char *>(instruction.operands.p),length);
            output.append('\n');
        }

        delete [] resultInstr;
    } else {
        emit error(tr("There was an error during the disassembling"), id);
    }
}

bool DistormTransf::isTwoWays()
{
    return false;
}

QWidget *DistormTransf::requestGui(QWidget *parent)
{
    return new(std::nothrow) DistormWidget(this, parent);
}

QHash<QString, QString> DistormTransf::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLOFFSET,QString::number(codeOffset));
    properties.insert(XMLASMTYPE, QString::number(asmType));
    properties.insert(XMLMAXINSTRUCTION,QString::number(maxInstruction));
    properties.insert(XMLSHOWOFFSET,QString::number(showoffset ? 1 : 0));
    properties.insert(XMLSHOWOPCODES,QString::number(showopcodes ? 1 : 0));
    return properties;
}

bool DistormTransf::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    bool ok = true;
    quint64 val = 0;

    val = propertiesList.value(XMLOFFSET).toULongLong(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLOFFSET),id);
    } else {
        setOffset(val);
    }

    int val1 = propertiesList.value(XMLASMTYPE).toInt(&ok);
    if (!ok || (val1 != A16Bits && val1 != A32Bits && val1 != A64Bits) ) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLASMTYPE),id);
    } else {
        setDecodeType(static_cast<ASMType>(val1));
    }

    uint val2 = propertiesList.value(XMLMAXINSTRUCTION).toUInt(&ok);
    if (!ok ) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLMAXINSTRUCTION),id);
    } else {
        setMaxInstruction(val2);
    }

    val1 = propertiesList.value(XMLSHOWOFFSET).toInt(&ok);
    if (!ok || (val1 != 0 && val1 != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLSHOWOFFSET),id);
    } else {
        setShowOffset(val1 == 1);
    }

    val1 = propertiesList.value(XMLSHOWOPCODES).toInt(&ok);
    if (!ok || (val1 != 0 && val1 != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLSHOWOPCODES),id);
    } else {
        setShowOpcodes(val1 == 1);
    }

    return res;
}

QString DistormTransf::help() const
{
    QString help;
    help.append("<p>Distorm disassembler</p><p>Disassemble a chunk of bytes using the Distorm library</p>");
    quint32 version = distorm_version();
    quint32 major = version >> 16;
    quint32 minor = ((version & 0x00FF00) >> 8);
    help.append(tr("<p>Distorm version in use: %1.%2</p>").arg(major).arg(minor));
    return help;
}

void DistormTransf::setDecodeType(DistormTransf::ASMType val)
{
    if (asmType != val) {
        asmType = val;
        emit confUpdated();
    }
}

DistormTransf::ASMType DistormTransf::getDecodeType() const
{
    return asmType;
}

void DistormTransf::setOffset(quint64 val)
{
    if (codeOffset != val) {
        codeOffset = val;
        emit confUpdated();
    }
}

quint64 DistormTransf::getOffset() const
{
    return codeOffset;
}

void DistormTransf::setMaxInstruction(uint val)
{
    if (maxInstruction != val) {
        maxInstruction = val;
        emit confUpdated();
    }
}

uint DistormTransf::getMaxInstruction() const
{
    return maxInstruction;
}

void DistormTransf::setShowOffset(bool val)
{
    if (showoffset != val) {
        showoffset = val;
        emit confUpdated();
    }
}

bool DistormTransf::getShowOffset() const
{
    return showoffset;
}

void DistormTransf::setShowOpcodes(bool val)
{
    if (showopcodes != val) {
        showopcodes = val;
        emit confUpdated();
    }
}

bool DistormTransf::getShowOpcodes() const
{
    return showopcodes;
}


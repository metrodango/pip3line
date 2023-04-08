/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include <QTextStream>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <pip3linecallback.h>
#include "transformabstract.h"
#include "baseplugins.h"
#include "urlencode.h"
#include "md5.h"
#include "md4.h"
#include "sha1.h"
#include "xor.h"
#include "charencoding.h"
#include "base64.h"
#include "base32.h"
#include "hexencode.h"
#include "reverse.h"
#include "binary.h"
#include "padding.h"
#include "html.h"
#include "rotx.h"
#include "cut.h"
#include "split.h"
#include "regularexp.h"
#include "randomcase.h"
#include "oracleconcat.h"
#include "mysqlconcat.h"
#include "postgresconcat.h"
#include "mssqlconcat.h"
#include "javascriptconcat.h"
#include "mysqlconcatv2.h"
#include "xmlquery.h"
#include "iptranslateipv4.h"
#include "networkmaskipv4.h"
#include "networkmaskipv6.h"
#include "fixprotocol.h"
#include "ciscosecret7.h"
#include "substitution.h"
#include "numbertochar.h"
#include "hieroglyphy.h"
#include "zlib.h"
#include "basex.h"
#include "microsofttimestamp.h"
#include "timestamp.h"
#include "bytesinteger.h"
#include "ntlmssp.h"
#include "bytestofloat.h"
#include "byterot.h"
#include "hmactransform.h"
#include "crc32.h"
#include "findandreplace.h"
#include "tds7_enc.h"
#include "jsonvalue.h"
#include "../../version.h"

#if QT_VERSION >= 0x050000
#include "sha224.h"
#include "sha256.h"
#include "sha384.h"
#include "sha512.h"
#endif

#if QT_VERSION >= 0x050100
#include "sha3_224.h"
#include "sha3_256.h"
#include "sha3_384.h"
#include "sha3_512.h"
#endif

const QString BasePlugins::Base64Url = "Base64 && Url Encode";
const QString BasePlugins::BinaryNum = "Binary (num)";
const QString BasePlugins::OctalNum = "Octal (num)";
const QString BasePlugins::HexaNum = "Hexadecimal (num)";

BasePlugins::BasePlugins()
{
    callback = nullptr;
}


BasePlugins::~BasePlugins()
{
    qDebug() << "Destroying " << this;
}

QString BasePlugins::pluginName() const
{
    return tr("Base Transformations");
}

TransformAbstract *BasePlugins::getTransform(QString name)
{
    TransformAbstract *ta = nullptr;

    if (name == Base64Url) {
        return getTransformFromFile(":/harcoded/composedxml/base64UrlEncode.xml");
    } else if (name == BinaryNum) {
        return getTransformFromFile(":/harcoded/composedxml/binarynumber.xml");
    } else if (name == OctalNum) {
        return getTransformFromFile(":/harcoded/composedxml/octalnumber.xml");
    } else if (name == HexaNum) {
        return getTransformFromFile(":/harcoded/composedxml/hexanumber.xml");
    } else

    if (Sha224::id == name) {
        ta = new(std::nothrow) Sha224();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Sha224 X{");
        }
    } else if (Sha256::id == name) {
        ta = new(std::nothrow) Sha256();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Sha256 X{");
        }
    } else if (Sha384::id == name) {
        ta = new(std::nothrow) Sha384();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Sha384 X{");
        }
    } else if (Sha512::id == name) {
        ta = new(std::nothrow) Sha512();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Sha512 X{");
        }
    } else

#if QT_VERSION >= 0x050100
        if (Sha3_224::id == name) {
            ta = new(std::nothrow) Sha3_224();
            if (ta == nullptr) {
                qFatal("Cannot allocate memory for Sha3_224 X{");
            }
        } else if (Sha3_256::id == name) {
            ta = new(std::nothrow) Sha3_256();
            if (ta == nullptr) {
                qFatal("Cannot allocate memory for Sha3_256 X{");
            }
        } else if (Sha3_384::id == name) {
            ta = new(std::nothrow) Sha3_384();
            if (ta == nullptr) {
                qFatal("Cannot allocate memory for Sha3_384 X{");
            }
        } else if (Sha3_512::id == name) {
            ta = new(std::nothrow) Sha3_512();
            if (ta == nullptr) {
                qFatal("Cannot allocate memory for Sha3_512 X{");
            }
        } else
#endif

    if (ByteRot::id == name) {
        ta = new(std::nothrow) ByteRot();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for ByteRot X{");
        }
    } else if (HMACTransform::id == name) {
        ta = new(std::nothrow) HMACTransform();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for HMACTransform X{");
        }
    } else if (Crc32::id == name) {
        ta = new(std::nothrow) Crc32();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Crc32 X{");
        }
    } else if (BytesToFloat::id == name) {
        ta = new(std::nothrow) BytesToFloat();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for BytesToFloat X{");
        }
    } else if (Ntlmssp::id == name) {
        ta = new(std::nothrow) Ntlmssp();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Ntlmssp X{");
        }
    } else if (BaseX::id == name) {
        ta = new(std::nothrow) BaseX();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for BaseX X{");
        }
    } else if (BytesInteger::id == name) {
        ta = new(std::nothrow) BytesInteger();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for BytesInteger X{");
        }
    } else if (TimeStamp::id == name) {
        ta = new(std::nothrow) TimeStamp();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for TimeStamp X{");
        }
    } else if (MicrosoftTimestamp::id == name) {
        ta = new(std::nothrow) MicrosoftTimestamp();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for MicrosoftTimestamp X{");
        }
    } else if (name == Zlib::id) {
        ta = new(std::nothrow) Zlib();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Zlib X{");
        }
    } else if (name == Hieroglyphy::id) {
        ta = new(std::nothrow) Hieroglyphy();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Hieroglyphy X{");
        }
    } else if (name == NumberToChar::id) {
        ta = new(std::nothrow) NumberToChar();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for NumberToChar X{");
        }
    } else if (name == Substitution::id) {
        ta = new(std::nothrow) Substitution();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Substitution X{");
        }
    } else if (name == CiscoSecret7::id) {
        ta = new(std::nothrow) CiscoSecret7();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for CiscoSecret7 X{");
        }
    } else if (name == FixProtocol::id) {
        ta = new(std::nothrow) FixProtocol();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for FixProtocol X{");
        }
    } else if (name == IPTranslateIPv4::id) {
        ta = new(std::nothrow) IPTranslateIPv4();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for IPTranslateIPv4 X{");
        }
    } else if (name == NetworkMaskIPv4::id) {
        ta = new(std::nothrow) NetworkMaskIPv4();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for NetworkMaskIPv4 X{");
        }
    } else if (name == NetworkMaskIPv6::id) {
        ta = new(std::nothrow) NetworkMaskIPv6();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for NetworkMaskIPv6 X{");
        }
    } else if (name == MySqlConcatv2::id) {
        ta = new(std::nothrow) MySqlConcatv2();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for MySqlConcatv2 X{");
        }
    } else if (name == JavaScriptConcat::id) {
        ta = new(std::nothrow) JavaScriptConcat();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for JavaScriptConcat X{");
        }
    } else if (name == MSSqlConcat::id) {
        ta = new(std::nothrow) MSSqlConcat();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for MSSqlConcat X{");
        }
    } else if (name == PostgresConcat::id) {
        ta = new(std::nothrow) PostgresConcat();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for PostgresConcat X{");
        }
    } else if (name == MysqlConcat::id) {
        ta = new(std::nothrow) MysqlConcat();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for MysqlConcat X{");
        }
    } else if (name == OracleConcat::id) {
        ta = new(std::nothrow) OracleConcat();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for OracleConcat X{");
        }
    } else if (name == UrlEncode::id) {
        ta = new(std::nothrow) UrlEncode();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for UrlEncode X{");
        }
    } else if (name == Padding::id) {
        ta = new(std::nothrow) Padding();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Padding X{");
        }
    } else if (name == Rotx::id) {
        ta = new(std::nothrow) Rotx();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Rotx X{");
        }
    } else if (name == Cut::id) {
        ta = new(std::nothrow) Cut();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Cut X{");
        }
    } else if (name == RandomCase::id) {
        ta = new(std::nothrow) RandomCase();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for RandomCase X{");
        }
    } else if (name == RegularExp::id) {
        ta = new(std::nothrow) RegularExp();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for RegularExp X{");
        }
    } else if (name == Split::id) {
        ta = new(std::nothrow) Split();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Split X{");
        }
    } else if (name == Md5::id) {
        ta = new(std::nothrow) Md5();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Md5 X{");
        }
    } else if (name == Sha1::id) {
        ta = new(std::nothrow) Sha1();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Sha1 X{");
        }
    } else if (name == Md4::id) {
        ta = new(std::nothrow) Md4();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Md4 X{");
        }
    } else if (name == Xor::id) {
        ta = new(std::nothrow) Xor();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Xor X{");
        }
    } else if (name == CharEncoding::id) {
        ta = new(std::nothrow) CharEncoding();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for CharEncoding X{");
        }
    } else if (name == Base64::id) {
        ta = new(std::nothrow) Base64();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Base64 X{");
        }
    } else if (name == Base32::id) {
        ta = new(std::nothrow) Base32();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Base32 X{");
        }
    } else if (name == Html::id) {
        ta = new(std::nothrow) Html();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Html X{");
        }
    } else if (name == HexEncode::id) {
        ta = new(std::nothrow) HexEncode();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for HexEncode X{");
        }
    } else if (name == Reverse::id) {
        ta = new(std::nothrow) Reverse();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for Reverse X{");
        }
    } else if (name == Binary::id) {
        ta = new(std::nothrow) Binary();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for BasBinaryeX X{");
        }
    } else if (name == XmlQuery::id) {
        ta = new(std::nothrow) XmlQuery();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for XmlQuery X{");
        }
    } else if (name == FindAndReplace::id) {
        ta = new(std::nothrow) FindAndReplace();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for FindAndReplace X{");
        }
    } else if (name == TDS7_ENC::id) {
        ta = new(std::nothrow) TDS7_ENC();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for TDS7_ENC X{");
        }
    } else if (name == JsonValue::id) {
        ta = new(std::nothrow) JsonValue();
        if (ta == nullptr) {
            qFatal("Cannot allocate memory for JsonValue X{");
        }
    }

    return ta;
}

const QStringList BasePlugins::getTransformList(QString typeName)
{
    QStringList enclist;

    if (typeName == DEFAULT_TYPE_ENCODER) {
        enclist.append(UrlEncode::id);
        enclist.append(CharEncoding::id);
        enclist.append(Base64::id);
        enclist.append(Base64Url);
        enclist.append(Base32::id);
        enclist.append(HexEncode::id);
        enclist.append(Binary::id);
        enclist.append(Html::id);
        enclist.append(NumberToChar::id);
    } else if (typeName == DEFAULT_TYPE_HASHES) {
        enclist.append(Md5::id);
        enclist.append(Md4::id);
        enclist.append(Sha1::id);
#if QT_VERSION >= 0x050000
        enclist.append(Sha224::id);
        enclist.append(Sha256::id);
        enclist.append(Sha384::id);
        enclist.append(Sha512::id);
#endif
#if QT_VERSION >= 0x050100
        enclist.append(Sha3_224::id);
        enclist.append(Sha3_256::id);
        enclist.append(Sha3_384::id);
        enclist.append(Sha3_512::id);
#endif
        enclist.append(Crc32::id);

    } else if (typeName == DEFAULT_TYPE_CRYPTO) {
        enclist.append(Xor::id);
        enclist.append(Rotx::id);
        enclist.append(CiscoSecret7::id);
        enclist.append(Substitution::id);
        enclist.append(ByteRot::id);
        enclist.append(HMACTransform::id);
        enclist.append(TDS7_ENC::id);
    } else if (typeName == DEFAULT_TYPE_HACKING) {
        enclist.append(Hieroglyphy::id);
        enclist.append(RandomCase::id);
        enclist.append(OracleConcat::id);
        enclist.append(MysqlConcat::id);
        enclist.append(PostgresConcat::id);
        enclist.append(MSSqlConcat::id);
        enclist.append(JavaScriptConcat::id);
        enclist.append(MySqlConcatv2::id);
    } else if (typeName == DEFAULT_TYPE_PARSERS) {
        enclist.append(FixProtocol::id);
        enclist.append(Ntlmssp::id);
    } else if (typeName == DEFAULT_TYPE_MISC) {
        enclist.append(Reverse::id);
        enclist.append(Padding::id);
        enclist.append(Cut::id);
        enclist.append(Split::id);
        enclist.append(RegularExp::id);
        enclist.append(FindAndReplace::id);
        enclist.append(XmlQuery::id);
        enclist.append(NetworkMaskIPv4::id);
        enclist.append(NetworkMaskIPv6::id);
        enclist.append(Zlib::id);
        enclist.append(JsonValue::id);
    } else if (typeName == DEFAULT_TYPE_TYPES_CASTING) {
        enclist.append(IPTranslateIPv4::id);
        enclist.append(MicrosoftTimestamp::id);
        enclist.append(TimeStamp::id);
        enclist.append(BytesInteger::id);
        enclist.append(BytesToFloat::id);
    } else if (typeName == DEFAULT_TYPE_NUMBER) {
        enclist.append(BaseX::id);
        enclist.append(BinaryNum);
        enclist.append(OctalNum);
        enclist.append(HexaNum);
    }
    return enclist;
}

const QStringList BasePlugins::getTypesList()
{
    return QStringList() << DEFAULT_TYPE_ENCODER << DEFAULT_TYPE_CRYPTO << DEFAULT_TYPE_MISC << DEFAULT_TYPE_PARSERS
                         << DEFAULT_TYPE_HASHES << DEFAULT_TYPE_HACKING << DEFAULT_TYPE_TYPES_CASTING << DEFAULT_TYPE_NUMBER;
}

QWidget *BasePlugins::getConfGui(QWidget *)
{
    return nullptr;
}

QString BasePlugins::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

int BasePlugins::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString BasePlugins::pluginVersion() const
{
    return VERSION_STRING;
}

void BasePlugins::setCallBack(Pip3lineCallback * ncallback)
{
    callback = ncallback;
}

TransformAbstract *BasePlugins::getTransformFromFile(QString resFile)
{
    QFile source(resFile);
    if (source.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&source);
        return callback->getTransformFromXML(&reader);
    } else {
        callback->logError(source.errorString());
    }
    return nullptr;
}

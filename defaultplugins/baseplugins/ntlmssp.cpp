/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "ntlmssp.h"
#include <QBuffer>
#include <QMapIterator>
#include <QDateTime>
#include "confgui/ntlmsspwidget.h"
#include <QDebug>

const QString Ntlmssp::id = "NTLMSSP";
const int Ntlmssp::SecurityBufferSize = 8;
const int Ntlmssp::OSFooterSize = 8;

Ntlmssp::Ntlmssp()
{
    doBase64 = false;
    oemString = false;
    unicodeCodec = QTextCodec::codecForName("UTF-16");
    NTMLSSP_TYPE.insert(1,"NTLMSSP_NEGOCIATE");
    NTMLSSP_TYPE.insert(2,"NTLMSSP_CHALLENGE");
    NTMLSSP_TYPE.insert(3,"NTLMSSP_RESPONSE");
    FLAGS_VAL.insert(0x00000001,"Negotiate Unicode");
    FLAGS_VAL.insert(0x00000002,"Negotiate OEM");
    FLAGS_VAL.insert(0x00000004,"Request Target");
    FLAGS_VAL.insert(0x00000008,"Request 0x00000008 (should not have been set)");
    FLAGS_VAL.insert(0x00000010,"Negotiate Sign");
    FLAGS_VAL.insert(0x00000020,"Negotiate Seal");
    FLAGS_VAL.insert(0x00000040,"Negotiate Datagram Style");
    FLAGS_VAL.insert(0x00000080,"Negotiate LM Key");
    FLAGS_VAL.insert(0x00000100,"Negotiate Netware (should be not set)");
    FLAGS_VAL.insert(0x00000200,"Negotiate NTLM");
    FLAGS_VAL.insert(0x00000400,"Negotiate NT only (unused)");
    FLAGS_VAL.insert(0x00000800,"Negotiate Anonymous");
    FLAGS_VAL.insert(0x00001000,"Negotiate Domain Supplied");
    FLAGS_VAL.insert(0x00002000,"Negotiate Workstation Supplied");
    FLAGS_VAL.insert(0x00004000,"0x00004000 (should not have been set)");
    FLAGS_VAL.insert(0x00008000,"Negotiate Always Sign");
    FLAGS_VAL.insert(0x00010000,"Target Domain");
    FLAGS_VAL.insert(0x00020000,"Target Server Name");
    FLAGS_VAL.insert(0x00040000,"0x00040000 (should not have been set)");
    FLAGS_VAL.insert(0x00080000,"Negotiate NTLM2 Key");
    FLAGS_VAL.insert(0x00100000,"Negotiate Identify");
    FLAGS_VAL.insert(0x00200000,"0x00200000 (should not have been set)");
    FLAGS_VAL.insert(0x00400000,"Request Non-NT Session Key");
    FLAGS_VAL.insert(0x00800000,"Negotiate Target Info");
    FLAGS_VAL.insert(0x01000000,"0x01000000 (should not have been set)");
    FLAGS_VAL.insert(0x02000000,"Negotiate Version");
    FLAGS_VAL.insert(0x04000000,"0x04000000 (should not have been set)");
    FLAGS_VAL.insert(0x08000000,"0x08000000 (should not have been set)");
    FLAGS_VAL.insert(0x10000000,"0x10000000 (should not have been set)");
    FLAGS_VAL.insert(0x20000000,"Negotiate 128");
    FLAGS_VAL.insert(0x40000000,"Negotiate Key Exchange");
    FLAGS_VAL.insert(0x80000000,"Negotiate 56");

    TARGET_INFO.insert(0x0000,"End of Target info");
    TARGET_INFO.insert(0x0001,"Server Name");
    TARGET_INFO.insert(0x0002,"Domain Name");
    TARGET_INFO.insert(0x0003,"Fully-qualified DNS host name");
    TARGET_INFO.insert(0x0004,"DNS domain name");
    TARGET_INFO.insert(0x0005,"parent DNS domain");
    TARGET_INFO.insert(0x0006,"Flags");
    TARGET_INFO.insert(0x0007,"TimeStamp");
    TARGET_INFO.insert(0x0008,"Restrictions");
    TARGET_INFO.insert(0x0009,"Target Name");
    TARGET_INFO.insert(0x000A,"Channel bindings");

}

Ntlmssp::~Ntlmssp()
{
}

QString Ntlmssp::name() const
{
    return id;
}

QString Ntlmssp::description() const
{
    return tr("NTLM SSP packet parser");
}

void Ntlmssp::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    if (input.isEmpty())
        return;

    QBuffer buf;
    QString str;
    QByteArray initial;
    quint16 length = 0;
    quint16 maxLength = 0;
    bool effectivelyBase64 = doBase64;


    quint32 offset;
    if (input.startsWith("TlRMTVNTUA") && !effectivelyBase64 ) {
        emit warning(tr("Input seems to be base64 encoded, enabling base64 decoding"),id);
        effectivelyBase64 = true;
    }

    if (effectivelyBase64) {
        initial = QByteArray::fromBase64(input);
    } else {
        initial = input;
    }

    quint32 sizeLimit = (quint32)initial.size();

    buf.setBuffer(&initial);
    buf.open(QIODevice::ReadOnly);

    QByteArray temp = buf.read(SecurityBufferSize);
    QByteArray temp2;
    if (temp.size() == SecurityBufferSize ) {
        temp.chop(1);
        output.append("NTLMSSP ID: ").append(temp).append("\n");
    } else {
        emit error(tr("Invalid identifier (0-7)"),id);
        return;
    }
    if (!temp.contains("NTLMSSP")) {
        emit error(tr("Not a valid NTLMSSP id, let's try anyway."),id);
    }

    temp = buf.read(sizeof(quint32));

    if (temp.size() != sizeof(quint32)) {
        emit error(tr("Invalid type (8-11)"),id);
        return;
    }

    quint32 type = 0;
    memcpy(&type, temp.data(),sizeof(quint32));

    if (NTMLSSP_TYPE.contains(type)) {
        output.append("Message TYPE: ").append(NTMLSSP_TYPE.value(type)).append("\n");
    } else {
        emit error(tr("Unkown NTLMSSP message (8-11)"),id);
        output.append("NTLMSSP TYPE: Unkown ").append(QByteArray::number(type)).append("\n");
        return;
    }

    switch (type) {
    case 1: {
        quint32 flags = 0;

        if (!readFlags(buf,&flags)) {
            emit error(tr("Invalid flags (12-15)"),id);
            return;
        }

        output.append("Flags:\n").append(extractFlags(flags)).append("\n");

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid Worksation Domain buffer (16-23)"),id);
            return;
        }
        temp = initial.mid(offset,length);

        str = QString::fromUtf8(temp);

        if (str.isEmpty())
            output.append("Calling Worksation Domain: NULL").append("\n");
        else
            output.append("Calling Worksation Domain: ").append(str).append("\n");


        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid Worksation Name buffer (24-31)"),id);
            return;
        }
        temp = initial.mid(offset,length);

        str = QString::fromUtf8(temp);
        if (str.isEmpty())
            output.append("Calling Worksation Name: NULL").append("\n");
        else
            output.append("Calling Worksation Name: ").append(QString::fromUtf8(temp)).append("\n");

        output.append(extractOSVersion(buf));

    }
        break;
    case 2: {

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid Target name buffer (12-19)"),id);
            return;
        }

        temp2 = initial.mid(offset,length);

        quint32 flags = 0;

        if (!readFlags(buf, &flags)) {
            emit error(tr("Invalid flags (20-23)"),id);
            return;
        }

        output.append("Target Name: ");
        output.append(getString(temp2)).append("\n");

        output.append("Flags:\n").append(extractFlags(flags)).append("\n");

        temp = buf.read(SecurityBufferSize);

        if (temp.size() != SecurityBufferSize) {
            emit error(tr("Invalid challenge (24-31)"),id);
            return;
        }

        output.append("Challenge: ").append(temp.toHex()).append("\n");

        temp = buf.read(SecurityBufferSize);

        if (temp.size() != SecurityBufferSize) {
            emit error(tr("Invalid context (32-39)"),id);
            return;
        }

        output.append("Context: ").append(temp.toHex()).append("\n");

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid Target info (40-47)"),id);
            return;
        }

        temp2 = initial.mid(offset,length);
        if (temp2.isEmpty())
            str = "NULL";
        else {
            str = extractTargetInfo(temp2);
            str.prepend("\n");
        }
        output.append("Target Info List: ").append(str);

        output.append(extractOSVersion(buf));

    }
        break;
    case 3: {

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid LM (12-19)"),id);
            return;
        }

        temp = initial.mid(offset,length);
        output.append("LM/LMv2: ").append(temp.mid(8,16).toHex()).append("\n");
        output.append("LM challenge: ").append(temp.mid(0,8).toHex()).append("\n");

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid NTLM (20-27)"),id);
            return;
        }

        QByteArray ntlm = initial.mid(offset,length);

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid Target Name (28-35)"),id);
            return;
        }
        QByteArray targetName = initial.mid(offset,length);

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid User Name (36-43)"),id);
            return;
        }
        QByteArray userName = initial.mid(offset,length);

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid Workstation Name (44-51)"),id);
            return;
        }
        QByteArray workstationName = initial.mid(offset,length);

        if (!readSecurityBuffer(buf, &length, &maxLength, &offset, sizeLimit)) {
            emit error(tr("Invalid Session Key (52-59)"),id);
            return;
        }
        QByteArray sessionKey = initial.mid(offset,length);

        quint32 flags = 0;
        if (!readFlags(buf, &flags)) {
            emit error(tr("Invalid flags (20-23)"),id);
            return;
        }
        output.append("NTLM/NTLMv2: ");
        if (ntlm.isEmpty()) {
            output.append("Empty\n");
        } else {
            output.append(extractNTLM(ntlm)).append("\n");
        }

        output.append("TARGET NAME: ").append(getString(targetName)).append("\n");
        output.append("USER NAME: ").append(getString(userName)).append("\n");
        output.append("Workstation NAME: ").append(getString(workstationName)).append("\n");
        output.append("Session Key: ").append(sessionKey.toHex()).append("\n");
        output.append("Flags:\n").append(extractFlags(flags));

        output.append(extractOSVersion(buf));

    }
        break;
    default: {
            output.append("Unmanaged type: ").append(QByteArray::number(type)).append("\n");
        }
    }
}

bool Ntlmssp::isTwoWays()
{
    return false;
}

QWidget *Ntlmssp::requestGui(QWidget * parent)
{
    QWidget * widget = new(std::nothrow) NtlmsspWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for NtlmsspWidget X{");
    }
    return widget;
}

QString Ntlmssp::help() const
{
    QString help;
    help.append("<p>NTLMP SSP parser</p><p>Parse a NTLMP SSP message following Microsoft document [MS-NLMP].pdf</p><p>Beware, if the NEGOTIATE and CHALLENGES messages should be parsed fine, REPONSES messages parsing is still a bit flaky</p>");
    return help;
}

QHash<QString, QString> Ntlmssp::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLDECODE,QString::number(doBase64 ? 1 : 0));
    return properties;
}

bool Ntlmssp::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLDECODE).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLDECODE),id);
    } else {
        setDecodeBase64(val == 1);
    }

    return res;
}

bool Ntlmssp::decodeBase64() const
{
    return doBase64;
}

void Ntlmssp::setDecodeBase64(bool val)
{
    doBase64 = val;
    emit confUpdated();
}

QByteArray Ntlmssp::extractFlags(int flags)
{
    QByteArray res;

    QMapIterator<quint32, QString> i(FLAGS_VAL);
     while (i.hasNext()) {
         i.next();

         if ((flags & i.key()) != 0) {
             res.append("  ").append(i.value()).append("\n");
         }
     }
     if (!res.isEmpty()) {
         res.chop(1);
     }

     if ((flags & 0x00000001) == 0 && (flags & 0x00000002) == 0) {
         emit error("Invalid token (OEM and UNICODE flags set both to 0)",id);
     }

     return res;
}

QByteArray Ntlmssp::extractTargetInfo(QByteArray &data)
{
    QByteArray str;
    QByteArray temp;
    ushort length = 0;
    quint16 typeString = 0;
    quint32 flags = 0;

    QBuffer buf;
    buf.setBuffer(&data);
    buf.open(QIODevice::ReadOnly);

    temp = buf.read(2);
    if (temp.size() != 2) {
        emit error(tr("Target Info is truncated (type)"),id);
        return str;
    }

    while (temp.size() == 2) {
        memcpy(&typeString, temp.data(),2);
        temp = buf.read(2);
        if (temp.size() != 2) {
            emit error(tr("Target Info is truncated (length)"),id);
            return str;
        }
        memcpy(&length, temp.data(),2);
        if (typeString == 0) { // end
            QByteArray remaining = buf.readAll().toHex();
            str.append("  ").append(TARGET_INFO.value(typeString));
            if (!remaining.isEmpty()) {
               str.append(": ").append(remaining);
            }
            return str;
        }
        if (length >= buf.size()) {
            emit error(tr("Target Info Parser: Length(%1) larger than the buffer %2").arg(length).arg(buf.size()),id);
            return str;
        }
        temp = buf.read(length);
        if (temp.size() != length) {
            emit error(tr("Target Info is truncated (value)"),id);
            return str;
        }
        if (typeString == 0x0007) {
            if (length != 8) {
                emit error(tr("Incorrect Length for timestamp"),id);
                return str;
            }
            str.append("  ").append(TARGET_INFO.value(typeString)).append(": ").append(toTimeStamp(temp)).append("\n");
        } else if (typeString == 0x0006) {
            if (temp.size() != 4) {
                emit error(tr("Invalid flags (INFO)"),id);
                return str;
            }
            memcpy(&flags, temp.data(),4);

            str.append("  ").append(TARGET_INFO.value(typeString)).append(": ").append(temp.toHex()).append("\n");
        } else if (typeString == 0x0008 || typeString == 0x000A) {
            str.append("  ").append(TARGET_INFO.value(typeString)).append(": ").append(temp.toHex()).append("\n");
        } else if (TARGET_INFO.contains(typeString)) {
            str.append("  ").append(TARGET_INFO.value(typeString)).append(": ").append(getString(temp)).append("\n");
        } else {
            str.append("  ").append("Unknown").append(": ").append(temp.toHex()).append("\n");
        }

        temp = buf.read(2);
    }

    emit error(tr("Target Info is truncated (type)"),id);
    return str;
}

QByteArray Ntlmssp::extractOSVersion(QBuffer &input)
{
    quint32 revision = 0;
    quint8 major = 0;
    quint8 minor = 0;
    QByteArray str ="\nOS Info:\n";

    QByteArray temp = input.read(1);
    if (temp.size() != 1)
        return str;

    memcpy((void *)(&major),temp.data(),1);

    temp = input.read(1);
        if (temp.size() != 1)
            return str;

    memcpy((void *)(&minor),temp.data(),1);

    str.append("  Major Version ").append(QByteArray::number(major)).append("\n");
    str.append("  Minor version ").append(QByteArray::number(minor)).append("\n");
    temp = input.read(2);
    if (temp.size() != 2)
        return str;
    quint16 val;
    memcpy(&val, temp.data(),2);
    str.append("  Build ").append(QByteArray::number(val)).append("\n");

    temp = input.read(4);
    if (temp.size() != 4)
        return str;

    memcpy(&revision, temp.data(),4);
    str.append("  NTLM Current revision ").append(QByteArray::number(reverseBytes(revision))).append("\n");

    return str;
}

QByteArray Ntlmssp::extractNTLM(QByteArray &data)
{
    QByteArray str;
    QBuffer buf;
    buf.setBuffer(&data);
    buf.open(QIODevice::ReadOnly);

    QByteArray temp = buf.read(16);
    if (temp.size() != 16) {
        emit error(tr("HMAC is truncated (0-16)"),id);
        return str;
    }

    str.append("\n  HMAC: ").append(temp.toHex()).append("\n");

    temp = buf.read(4);
    if (temp.size() != 4) {
        emit error(tr("Header is truncated (17-20)"),id);
        return str;
    }
    temp = reverseBytes(temp);
    str.append("  Header: ").append(temp.toHex()).append("\n");

    temp = buf.read(4);
    if (temp.size() != 4) {
        emit error(tr("Reserved is truncated (21-24)"),id);
        return str;
    }
    temp = reverseBytes(temp);
    str.append("  Reserved: ").append(temp.toHex()).append("\n");

    temp = buf.read(8);
    if (temp.isEmpty()) { // this happens with certain type of message
        return str;
    }
    if (temp.size() != 8) {
        emit error(tr("Timestamp is truncated (25-32)"),id);
        return str;
    }
    str.append("  Timestamp: ").append(toTimeStamp(temp)).append("\n");

    temp = buf.read(8);
    if (temp.size() != 8) {
        emit error(tr("Client Challenge is truncated (33-40)"),id);
        return str;
    }
    str.append("  Client Challenge: ").append(temp.toHex()).append("\n");

    temp = buf.read(4);
    if (temp.size() != 4) {
        emit error(tr("Unknown is truncated (41-44)"),id);
        return str;
    }
    str.append("  Unknown: ").append(temp.toHex()).append("\n");

    temp = buf.readAll();
    str.append(extractTargetInfo(temp));

    return str;
}

QByteArray Ntlmssp::getString(const QByteArray &data)
{
    QByteArray str;

    if (oemString) { // the flag value is garanted to have been read already
        str = QString::fromUtf8(data).toUtf8();
    } else {
        str =  unicodeCodec->toUnicode(data).toUtf8();
    }

    if (str.isEmpty())
        str = "NULL";

    return str;
}

quint32 Ntlmssp::reverseBytes(quint32 val)
{
    return   (((val & 0xFF000000) >> 24) | \
              ((val & 0x00FF0000) >> 8) | \
              ((val & 0x0000FF00) << 8)  | \
              ((val & 0x000000FF) << 24));
}

QByteArray Ntlmssp::reverseBytes(const QByteArray &data)
{
    QByteArray ret;
    for (int i = data.size() - 1; i >= 0; i--) {
        ret.append(data.at(i));
    }
    return ret;
}

QByteArray Ntlmssp::toTimeStamp(const QByteArray &data)
{
    QByteArray ret;
    if (data.size() < 8) {
        emit error(tr("Invalid TimeStamp"),id);
        return ret;
    }

    QDateTime date;
    quint64 val1;
    quint64 timeStamp = 0;
    quint64 rest = 0;
#ifdef Q_CC_MSVC
    memcpy_s(&timeStamp,sizeof(quint64), data.data(),sizeof(quint64));
#else
    memcpy(&timeStamp, data.data(),sizeof(quint64));
#endif
    val1 = timeStamp / (10000);
    rest = timeStamp % 10000;
    date.setTimeSpec(Qt::UTC);
    date.setDate(QDate(1601,1,1));
    date = date.addMSecs(val1 > LONG_MAX ? LONG_MAX : (qint64) val1); // just in case there is a crazy value
    ret = date.toString("ddd d MMMM yyyy hh:mm:ss.zzz").toUtf8();
    ret.append("ms ").append(QByteArray::number(rest)).append(" ns UTC");
    return ret;
}

bool Ntlmssp::readSecurityBuffer(QBuffer &input, quint16 *length, quint16 *maxLength, quint32 *offset, quint32 maxsize)
{
    QByteArray data = input.read(SecurityBufferSize);

    if (data.size() != SecurityBufferSize) {
        emit error(tr("Security buffer is truncated"),id);
        return false;
    }
    memcpy(length, data.mid(0,sizeof(quint16)).data(),sizeof(quint16));
    memcpy(maxLength, data.mid(2,sizeof(quint16)).data(),sizeof(quint16));
    memcpy(offset, data.mid(4,sizeof(quint32)).data(),sizeof(quint32));

    if ((*offset) + (*length) > maxsize) {
        emit error(tr("Too large Security Buffer size"),id);
        return false;
    }
    return true;
}

bool Ntlmssp::readFlags(QBuffer &input, quint32 *flags)
{
    QByteArray data = input.read(sizeof(quint32));

    if (data.size() != sizeof(quint32)) {
        emit error(tr("Flag value is truncated"),id);
    }

    memcpy(flags, data.data(),sizeof(quint32));
    oemString = ((*flags) & 0x00000002) != 0;

    return true;
}

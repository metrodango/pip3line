/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "ciscosecret7.h"

const QString CiscoSecret7::id = "Cisco secret 7";
const unsigned int CiscoSecret7::MAXSEED = 52;

const QByteArray CiscoSecret7::ckey = "dsfd;kfoA,.iyewrkldJKDHSUBsgvca69834ncxv9873254k;fg87";


CiscoSecret7::CiscoSecret7()
{
    seed = 14;
}

QString CiscoSecret7::name() const
{
    return id;
}

QString CiscoSecret7::description() const
{
    return tr("Decrypt Cisco secret type 7");
}

void CiscoSecret7::transform(const QByteArray &input, QByteArray &output)
{
    if (wayValue == INBOUND) {
        QByteArray data;

        cipher(seed,input, data);
        output = data.toHex().prepend(QByteArray::number(seed));

    } else {
        QByteArray encrypted = input.toUpper();

        if (encrypted.isEmpty())
            return;

        if (encrypted.size() < 3) {
            emit error(tr("Invalid hash (far too small)"),id);
            return;
        }
        bool k = true;
        seed = static_cast<uint>(encrypted.mid(0,2).toInt(&k));
        if (!k || seed > MAXSEED) {
            emit error(tr("Invalid seed, it must be an unsigned integer <= %1").arg(MAXSEED),id);
            return;
        }

        QByteArray data = encrypted.mid(2);
        data = fromHex(data);
        cipher(seed,data,output);
    }
}

bool CiscoSecret7::isTwoWays()
{
    return true;
}

QString CiscoSecret7::help() const
{
    QString help;
    help.append("<p>Decryption of passwords in Cisco 7 format</p><p>Please send me any value that does not work properly.</p>");
    return help;
}

unsigned int CiscoSecret7::getSeed() const
{
    return seed;
}

bool CiscoSecret7::setSeed(unsigned int nseed)
{
    if (seed > 52) {
        emit error(tr("Valid values for a seed are supposed to be in [0-52]"),id);
        return false;
    }
    if (nseed != seed) {
        seed = nseed;
        emit confUpdated();
    }

    return true;
}

void CiscoSecret7::cipher(unsigned int seed, const QByteArray &data, QByteArray &output)
{
    for (int i = 0; i < data.size(); i ++) {
        output.append(data.at(i) ^ ckey.at(static_cast<int>(seed + static_cast<unsigned int>(i)) % ckey.size()));
    }
}

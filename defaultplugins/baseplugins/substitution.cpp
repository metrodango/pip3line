/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "substitution.h"
#include "confgui/substitutionwidget.h"
#include <QSet>
#include <QDebug>

const QString Substitution::id = "Substitution";

QStringList Substitution::knownTables = QStringList() << "Identity" << "Unisys Accessory Manager 5.0.00.075";

char Substitution::s_table_tcpa[256] = {
    '\x00', '\x01', '\x02', '\x03', '\x37', '\x2d', '\x2e', '\x2f', '\x16', '\x05', '\x25', '\x0B', '\x0C', '\x0D', '\x0E', '\x0F',
    '\x10', '\x11', '\x12', '\x13', '\x3C', '\x3D', '\x32', '\x26', '\x18', '\x19', '\x3F', '\x27', '\x1C', '\x1D', '\x1E', '\x1F',
    '\x40', '\x4F', '\x7F', '\x7B', '\x5B', '\x6c', '\x50', '\x7d', '\x4D', '\x5D', '\x5C', '\x4E', '\x6B', '\x60', '\x4B', '\x61',
    '\xF0', '\xf1', '\xF2', '\xF3', '\xF4', '\xF5', '\xF6', '\xF7', '\xF8', '\xF9', '\x7A', '\x5E', '\x4C', '\x7E', '\x6E', '\x6F',
    '\x7C', '\xC1', '\xC2', '\xC3', '\xC4', '\xC5', '\xC6', '\xC7', '\xC8', '\xC9', '\xD1', '\xD2', '\xD3', '\xD4', '\xD5', '\xD6',
    '\xD7', '\xD8', '\xD9', '\xE2', '\xE3', '\xE4', '\xE5', '\xE6', '\xE7', '\xE8', '\xE9', '\x4A', '\xE0', '\x5A', '\x5F', '\x6D',
    '\x79', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87', '\x88', '\x89', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96',
    '\x97', '\x98', '\x99', '\xA2', '\xA3', '\xA4', '\xA5', '\xA6', '\xA7', '\xA8', '\xA9', '\xC0', '\x6A', '\xD0', '\xA1', '\x07',
    '\x20', '\x21', '\x22', '\x23', '\x24', '\x15', '\x06', '\x17', '\x28', '\x29', '\x2A', '\x2B', '\x2C', '\x09', '\x0A', '\x1B',
    '\x30', '\x31', '\x1A', '\x33', '\x34', '\x35', '\x36', '\x08', '\x38', '\x39', '\x3A', '\x3B', '\x04', '\x14', '\x3E', '\xE1',
    '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', '\x48', '\x49', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57',
    '\x58', '\x59', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67', '\x68', '\x69', '\x70', '\x71', '\x72', '\x73', '\x74', '\x75',
    '\x76', '\x77', '\x78', '\x80', '\x8A', '\x8B', '\x8C', '\x8D', '\x8E', '\x8F', '\x90', '\x9A', '\x9B', '\x9C', '\x9D', '\x9E',
    '\x9F', '\xA0', '\xAA', '\xAB', '\xAc', '\xad', '\xae', '\xaf', '\xb0', '\xb1', '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7',
    '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd', '\xbe', '\xbf', '\xca', '\xcb', '\xcc', '\xcd', '\xce', '\xcf', '\xda', '\xdb',
    '\xdc', '\xdd', '\xde', '\xdf', '\xea', '\xeb', '\xec', '\xed', '\xee', '\xef', '\xfa', '\xfb', '\xfc', '\xfd', '\xfe', '\xff'
                    };

Substitution::Substitution()
{
    for (quint16 i = 0; i < SubstitutionTables::S_ARRAY_SIZE; i++) {
        s_table_e[i] = (char)i;
    }

    createDecryptTable();
}

Substitution::~Substitution()
{

}

QString Substitution::name() const
{
    return id;

}

QString Substitution::description() const
{
    return tr("Basic Char substitution transformation (also known as permutation)");
}

void Substitution::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    if (!isPermutationValid(s_table_e)) {
        emit error(tr("Invalid permutation : duplicates found in the table"),id);
    }

    if (input.isEmpty())
        return;

    if (wayValue == INBOUND) {
        for (int i = 0; i < input.size(); i++) {
            output.append((char)s_table_e[(quint8)input.at(i)]);
        }
    } else {
        for (int i = 0; i < input.size(); i++) {
            output.append((char)s_table_d[(quint8)input.at(i)]);
        }
    }

}

bool Substitution::isTwoWays()
{
    return true;
}

QHash<QString, QString> Substitution::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLKEY,QString(s_table_e.toBase64()));

    return properties;
}

bool Substitution::setConfiguration(QHash<QString, QString> propertiesList)
{

    setSTable(QByteArray::fromBase64(propertiesList.value(XMLKEY).toUtf8()));
    return TransformAbstract::setConfiguration(propertiesList);
}

QWidget *Substitution::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) SubstitutionWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for SubstitutionWidget X{");
    }
    return widget;
}

QString Substitution::inboundString() const
{
    return tr("Encrypt");
}

QString Substitution::outboundString() const
{
    return tr("Decrypt");
}

QByteArray Substitution::getSTable()
{
    return s_table_e;
}

void Substitution::setSTable(QByteArray sTable)
{
    if (sTable.size() < SubstitutionTables::S_ARRAY_SIZE) {
        sTable.append(QByteArray(SubstitutionTables::S_ARRAY_SIZE - sTable.size(),'\x00'));
        emit error(tr("Permutation table too short (%1 intead of 256). Completed with zeros.").arg(sTable.size()),id);
    } else if (sTable.size() > SubstitutionTables::S_ARRAY_SIZE) {
        sTable = sTable.mid(0,SubstitutionTables::S_ARRAY_SIZE);
        emit error(tr("Permutation table too long (%1 instead of 256). Truncated.").arg(sTable.size()),id);
    }
    if (s_table_e != sTable) {
        s_table_e = sTable;

        createDecryptTable();
        emit confUpdated();
    }
}

bool Substitution::isPermutationValid(QByteArray sTable)
{
    QSet<char> verf;
    for (int i = 0; i < sTable.size(); i++) {
        if (verf.contains(sTable.at(i))) {
            return false;
        }
        else
            verf.insert(sTable.at(i));
    }
    return true;
}

QByteArray Substitution::getPredeterminedTable(QString name)
{
    QByteArray ret;

    if (knownTables.contains(name)) {
        int l = knownTables.indexOf(name);
        switch(l) {
        case 0:
            for (quint16 i = 0; i < SubstitutionTables::S_ARRAY_SIZE; i++) {
                ret.append((char)i);
            }
            break;
        case 1:
            ret.append(s_table_tcpa,SubstitutionTables::S_ARRAY_SIZE);
            return ret;
            break;
        default:
            return s_table_e;
        }
    }

    return ret;
}

QString Substitution::help() const
{
    QString help;
    help.append("<p>Basic substitution algorithm over the entire char table. i.e. permutation over 256 characters</p><p>Some old softwares are still using this as end-to-end encryption, such as Unisys with the TCPA Telnet-like protocol.</p><p>An example is given for a specific Unisys client version.<br> It is however possible that different versions use a different permutation vector, or that this vector value is linked to the customer licence, or configuration.(untested)<br>Although it appears that the value remains constant accross different user accounts for the same platform.</p>");
    return help;
}

void Substitution::createDecryptTable()
{
    for (quint16 i = 0; i < SubstitutionTables::S_ARRAY_SIZE; i++) {
        s_table_d[(quint8)s_table_e[i]] = i;
    }
}

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "transformabstract.h"
#include <QTextStream>
#include <QWidget>
#include <QDebug>

const QByteArray TransformAbstract::HEXCHAR("abcdefABCDEF1234567890");

TransformAbstract::TransformAbstract() {
    confGui = nullptr;
    wayValue = INBOUND;
    qDebug() << "Created " << this;
}

TransformAbstract::~TransformAbstract() {

    delete confGui;
    confGui = nullptr;

    qDebug() << "Destroyed " << this;
}

QByteArray TransformAbstract::transform(const QByteArray &input)
{
    QByteArray ret;
    transform(input,ret);
    return ret;
}

void TransformAbstract::logError(const QString message, const QString source)
{
    emit error(message, source);
}

void TransformAbstract::logWarning(const QString message, const QString source)
{
    emit warning(message, source);
}

bool TransformAbstract::isTwoWays()
{
    return false;
}

QWidget *TransformAbstract::getGui(QWidget * parent)
{
    if (confGui == nullptr) {
        confGui = requestGui(parent);
        if (confGui != nullptr) {
            connect(confGui, &QWidget::destroyed, this, &TransformAbstract::onGuiDelete, Qt::UniqueConnection);
        }
    }
    return confGui;
}

QWidget *TransformAbstract::requestGui(QWidget * /* parent */)
{
    return nullptr;
}

void TransformAbstract::onGuiDelete()
{
    confGui = nullptr;
}

QString TransformAbstract::inboundString() const
{
    return tr("Encode");
}

QString TransformAbstract::outboundString() const
{
    return tr("Decode");
}

QString TransformAbstract::help() const
{
    return tr("No help available.<br> If you want to add help for your transformation, re-implement the help() function to return your own string. HTML is authorised.");
}

QString TransformAbstract::credits() const
{
    QString credits;
    credits.append(tr("<p>Released as open source by Gabriel Caudrelier </p><p>Developped by Gabriel Caudrelier - gabriel.caudrelier@gmail.com</p><p>Released under AGPL see LICENSE file for more information</p> "));
    return credits;
}

QHash<QString, QString> TransformAbstract::getConfiguration()
{
    QHash<QString, QString> propertiesList;
    propertiesList.insert(PROP_WAY, QString::number((int)wayValue));
    propertiesList.insert(PROP_NAME, name());
    return propertiesList;
}

bool TransformAbstract::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool ok = true;
    int val = propertiesList.value(PROP_WAY).toInt(&ok);
    if (!ok || (val != INBOUND && val != OUTBOUND)) {
        emit error(tr("Invalid value for %1").arg(PROP_WAY),name());
        ok = false;
    }
    else {
        wayValue = (Way) val;
    }

    return ok;
}

void TransformAbstract::setWay(TransformAbstract::Way nway)
{
    if (wayValue != nway) {
        wayValue = nway;
        Q_EMIT confUpdated();
    }
}

TransformAbstract::Way TransformAbstract::way()
{
    return wayValue;
}

QByteArray TransformAbstract::fromHex(QByteArray in)
{

    QString invalid;
    QString HEXCHAR("abcdefABCDEF0123456789");
    for (int i = 0; i < in.size(); i++){
        if (!HEXCHAR.contains(in.at(i))) {
            if (!invalid.contains(in.at(i)))
                invalid.append(in.at(i));
        }
    }

    if (!invalid.isEmpty()) {
        emit error(tr("Invalid character(s) found in the hexadecimal stream: '%1', they will be skipped").arg(invalid),name());
    }

    in.replace(invalid,"");

    if (in.size()%2 != 0) {
        in.chop(1);
        emit error(tr("Invalid size for a hexadecimal stream, skipping the last byte."),name());
    }

    return QByteArray::fromHex(in);
}

QString TransformAbstract::saveChar(const char c) const
{
    QByteArray val(1,c);
    return QString::fromUtf8(val.toHex());
}

bool TransformAbstract::loadChar(const QString &val, char *c)
{
    QByteArray tmp = fromHex(val.toUtf8());
    if (tmp.size() == 0) {
        emit error(tr("Empty value for char"), name());
        return false;
    } else if (tmp.size() > 1) {
        emit warning(tr("Multiple values for char, only the first one will be considered"), name());
    }
    (*c) = tmp.at(0);

    return true;
}

bool TransformAbstract::isPrintable(const qint32 c) {
    return (c > 0x1F && c < 0x7F);
}

QByteArray TransformAbstract::toPrintableString(const QByteArray &val, bool strict)
{
    QByteArray ret;
    for (int i = 0; i < val.size(); i++) {
        char c = val.at(i);
        if (isPrintable((quint32)c)) {
            ret.append(c);
        } else if (strict) {
            ret.append("\\x").append(QByteArray(1,c).toHex());
        } else {
            switch (c) {
                case '\n':
                    ret.append("\\n");
                    break;
                case '\r':
                    ret.append("\\r");
                    break;
                default:
                ret.append("\\x").append(QByteArray(1,c).toHex());
            }
        }
    }

    return ret;
}

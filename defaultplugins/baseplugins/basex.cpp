/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "basex.h"
#include "confgui/basexwidget.h"

const QString BaseX::id = "BaseX";

BaseX::BaseX(int nbase)
{
    base = nbase;
    uppercase = true;
}

BaseX::~BaseX()
{
}

QString BaseX::name() const {
    return id;
}

QString BaseX::description() const {
    return tr("Translate numbers to a different numeral base");
}

bool BaseX::isTwoWays() {
    return true;
}

void BaseX::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    bool ok;
    if (wayValue == INBOUND) {
        qlonglong num = input.toLongLong(&ok);
        if (!ok) {
            emit error(tr("Invalid number (remember that the number is handled internally as an int64, so anything bigger will produce an error)"),id);
            return;
        }

        output = QByteArray::number(num, base);
        if (uppercase)
            output = output.toUpper();
    } else {
        qlonglong num = input.toLongLong(&ok, base);
        if (!ok) {
            emit error(tr("Invalid number (remember that the number is handled internally as an int64, so anything bigger will produce an error)"),id);
            return;
        }
        output = QByteArray::number(num);
    }
}

QHash<QString, QString> BaseX::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLBASE,QString::number(base));
    properties.insert(XMLUPPERCASE,QString::number(uppercase ? 1 : 0));

    return properties;
}

bool BaseX::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;
    int val = propertiesList.value(XMLBASE).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLBASE),id);
    } else {
        res = setBase(val) && res;
    }

    val = propertiesList.value(XMLUPPERCASE).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLUPPERCASE),id);
    } else {
        setUppercase(val == 1);
    }

    return res;
}

QWidget *BaseX::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) BaseXWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for BaseXWidget X{");
    }
    return widget;
}

QString BaseX::help() const
{
    QString help;
    help.append("<p>Translate numbers to a different numeral base</p><p>Internally the numbers are handled with a 64bits signed integer, so any numbers that cannot fit will produce an error.</p>");
    return help;
}

int BaseX::getBase() const
{
    return base;
}

bool BaseX::getUppercase() const
{
    return uppercase;
}

bool BaseX::setBase(int val)
{
    if (base < MINBASE || base > MAXBASE) {
        emit error(tr("Base value outside valid range [%1-%2]").arg(MINBASE).arg(MAXBASE),id);
        return false;
    }
    if (base != val) {
        base = val;
        emit confUpdated();
    }
    return true;

}

void BaseX::setUppercase(bool val)
{
    if (uppercase != val) {
        uppercase = val;
        emit confUpdated();
    }
}


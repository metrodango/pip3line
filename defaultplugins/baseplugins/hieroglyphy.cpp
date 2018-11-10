/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hieroglyphy.h"
#include "confgui/hieroglyphywidget.h"
#include <QDebug>

const QString Hieroglyphy::id = "Hieroglyphy";
const QString Hieroglyphy::XMLBTOAINUSE = "btoaInUse";

Hieroglyphy::Hieroglyphy()
{
    btoaInUse = true;
    init();
}

Hieroglyphy::~Hieroglyphy()
{
}

QString Hieroglyphy::name() const
{
    return id;
}

QString Hieroglyphy::description() const
{
    return tr("Convert Javascript into hieroglyhpy");
}

void Hieroglyphy::transform(const QByteArray &input, QByteArray &output)
{
    if (input.size() < 1)
        return;

    QString init = QString::fromUtf8(input);
    QString final;


    if (wayValue == OUTBOUND)
        final = hString(init);
    else
        final = hScript(init);

    output = final.toUtf8();
}

bool Hieroglyphy::isTwoWays()
{
    return true;
}

QHash<QString, QString> Hieroglyphy::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLBTOAINUSE,QString::number(static_cast<int>(btoaInUse)));

    return properties;
}

bool Hieroglyphy::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLBTOAINUSE).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLBTOAINUSE),id);
    } else {
        setUseBtoa(val == 1);
    }

    return res;
}

QWidget *Hieroglyphy::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) HieroglyphyWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for HieroglyphyWidget X{");
    }
    return widget;
}

QString Hieroglyphy::inboundString() const
{  
    return tr("Convert script");
}

QString Hieroglyphy::outboundString() const
{
    return tr("Convert string");
}

QString Hieroglyphy::help() const
{
    QString helpS;
    helpS = tr("<p>Convert a JavaScript String or script to a non-alphanumeric sequences.</p><p>The infamous <span style=\" color:#ff5757;\">alert(1);</span> works fine but more complex scripts can be converted.</p>");
    helpS.append("<p>The transformation has two modes: <ul><li>one for strings only</li><li>one for scripts (i.e. executable string)</li></ul></p>");
    helpS.append("<p>Compared to the original algorithm (see Credits) multi-bytes unicode characters (unicode value > 255) conversion should be working fine.");
    helpS.append("<br>Although converting just one of them produces a massive sequence, so I would recommend avoiding them.</p>");
    helpS.append("<br>As explained from the original blog post, this algorithm will only produce \"browser\" compatible sequences, due to the fact that we use the <span style=\" color:#ff5757;\">location</span> property to generate the 'h' , 't' and '/' characters.<br>");
    return helpS;
}

QString Hieroglyphy::credits() const
{
    QString creditsS = TransformAbstract::credits();

    creditsS.append("<p>The conversion algorithm is losely inspired from the one found in Patricio Palladino blog <a href='http://patriciopalladino.com/blog/2012/08/09/non-alphanumeric-javascript.html'>post</a><br>");
    return creditsS;
}

void Hieroglyphy::setUseBtoa(bool val)
{
    if (btoaInUse != val) {
        btoaInUse = val;
        init();
        emit confUpdated();
    }
}

bool Hieroglyphy::isBtoaInUse() const
{
    return btoaInUse;
}

void Hieroglyphy::init()
{
    numbers.clear();
    numbers << "+[]" << "+!![]" << "!+[]+!![]" << "!+[]+!![]+!![]";
    numbers << "!+[]+!![]+!![]+!![]";
    numbers << "!+[]+!![]+!![]+!![]+!![]";
    numbers << "!+[]+!![]+!![]+!![]+!![]+!![]";
    numbers << "!+[]+!![]+!![]+!![]+!![]+!![]+!![]";
    numbers << "!+[]+!![]+!![]+!![]+!![]+!![]+!![]+!![]";
    numbers << "!+[]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+!![]";

    characters.clear();
    characters.insert('0', QString("(%1+[])").arg(numbers.at(0)));
    characters.insert('1', QString("(%1+[])").arg(numbers.at(1)));
    characters.insert('2', QString("(%1+[])").arg(numbers.at(2)));
    characters.insert('3', QString("(%1+[])").arg(numbers.at(3)));
    characters.insert('4', QString("(%1+[])").arg(numbers.at(4)));
    characters.insert('5', QString("(%1+[])").arg(numbers.at(5)));
    characters.insert('6', QString("(%1+[])").arg(numbers.at(6)));
    characters.insert('7', QString("(%1+[])").arg(numbers.at(7)));
    characters.insert('8', QString("(%1+[])").arg(numbers.at(8)));
    characters.insert('9', QString("(%1+[])").arg(numbers.at(9)));

    Sobject_Object = "[]+{}";
    SNaN = "+{}+[]";
    Strue = "!![]+[]";
    Sfalse = "![]+[]";
    Sundefined = "[][[]]+[]";

    characters.insert(' ', QString("(%1)[%2]").arg(Sobject_Object).arg(numbers.at(7)));
    characters.insert('[', QString("(%1)[%2]").arg(Sobject_Object).arg(numbers.at(0)));
    characters.insert(']', QString("(%1)[%2+%3]").arg(Sobject_Object).arg(characters.value('1')).arg(characters.value('4')));
    characters.insert('a', QString("(%1)[%2]").arg(SNaN).arg(numbers.at(1)));
    characters.insert('b', QString("(%1)[%2]").arg(Sobject_Object).arg(numbers.at(2)));
    characters.insert('c', QString("(%1)[%2]").arg(Sobject_Object).arg(numbers.at(5)));
    characters.insert('d', QString("(%1)[%2]").arg(Sundefined).arg(numbers.at(2)));
    characters.insert('e', QString("(%1)[%2]").arg(Sundefined).arg(numbers.at(3)));
    characters.insert('f', QString("(%1)[%2]").arg(Sundefined).arg(numbers.at(4)));
    characters.insert('i', QString("(%1)[%2]").arg(Sundefined).arg(numbers.at(5)));
    characters.insert('j', QString("(%1)[%2]").arg(Sobject_Object).arg(numbers.at(3)));
    characters.insert('l', QString("(%1)[%2]").arg(Sfalse).arg(numbers.at(2)));
    characters.insert('n', QString("(%1)[%2]").arg(Sundefined).arg(numbers.at(1)));
    characters.insert('o', QString("(%1)[%2]").arg(Sobject_Object).arg(numbers.at(1)));
    characters.insert('r', QString("(%1)[%2]").arg(Strue).arg(numbers.at(1)));
    characters.insert('s', QString("(%1)[%2]").arg(Sfalse).arg(numbers.at(3)));
    characters.insert('t', QString("(%1)[%2]").arg(Strue).arg(numbers.at(0)));
    characters.insert('u', QString("(%1)[%2]").arg(Sundefined).arg(numbers.at(0)));
    characters.insert('N', QString("(%1)[%2]").arg(SNaN).arg(numbers.at(0)));
    characters.insert('O', QString("(%1)[%2]").arg(Sobject_Object).arg(numbers.at(8)));

    SInfinity = QString("+(%1+%2+%3+%4+%5+%6)+[]").arg(numbers.at(1)).arg(characters.value('e')).arg(characters.value('1')).arg(characters.value('0')).arg(characters.value('0')).arg(characters.value('0'));

    characters.insert('y', QString("(%1)[%2]").arg(SInfinity).arg(numbers.at(7)));
    characters.insert('I', QString("(%1)[%2]").arg(SInfinity).arg(numbers.at(0)));

    S1e100 = QString("+(%1+%2+%3+%4+%5)+[]").arg(numbers.value(1)).arg(characters.value('e')).arg(characters.value('1')).arg(characters.value('0')).arg(characters.value('0'));

    characters.insert('+', QString("(%1)[%2]").arg(S1e100).arg(numbers.at(2)));

    functionConstructor = QString("[][%1][%2]").arg(hString("sort")).arg(hString("constructor"));
    locationString = QString("[]+").append(hScript("return location"));

    characters.insert('h', QString("(%1)[%2]").arg(locationString).arg(numbers.at(0)));
    characters.insert('p', QString("(%1)[%2]").arg(locationString).arg(numbers.at(3)));
    characters.insert('/', QString("(%1)[%2]").arg(locationString).arg(numbers.at(6)));

    unescapeString = hScript("return unescape");
    escapeString = hScript("return escape");

    characters.insert('%', QString("%1(%2)[%3]").arg(escapeString).arg(characters.value('[')).arg(numbers.at(0)));
    characters.insert('x', QString("%1(%2+%3)").arg(unescapeString).arg(characters.value('%')).arg(hString(toHex('x'))));
    characters.insert('\\', QString("%1(%2+%3)").arg(unescapeString).arg(characters.value('%')).arg(hString(toHex('\\'))));

    if (btoaInUse) {
        btoaInit();
    }
    qDebug() << "Base characters currently implemented " << characters.size() ;
    QList<char> list = characters.keys();
    std::sort(list.begin(), list.end());

    qDebug() << list;

    fromCharCodeS = hScript("return String.fromCharCode");
}

void Hieroglyphy::btoaInit()
{
    btoaFunction = hScript("return btoa");

    // Extracting from btoa("object Object")
    characters.insert('W', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(numbers.at(0)));
    characters.insert('m', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(numbers.at(5)));
    characters.insert('V', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(numbers.at(6)));
    characters.insert('C', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(numbers.at(9)));
    characters.insert('B', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(hNumber(10)));
    characters.insert('P', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(hNumber(11)));
    characters.insert('Y', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(hNumber(12)));
    characters.insert('p', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(hNumber(14)));
    characters.insert('R', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sobject_Object).arg(hNumber(18)));
    // Extracting from btoa("NaN")
    characters.insert('T', QString("%1(%2)[%3]").arg(btoaFunction).arg(SNaN).arg(numbers.at(0)));
    characters.insert('F', QString("%1(%2)[%3]").arg(btoaFunction).arg(SNaN).arg(numbers.at(2)));
    // Extracting from btoa("true")
    characters.insert('H', QString("%1(%2)[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(1)));
    characters.insert('J', QString("%1(%2)[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(2)));
    characters.insert('Z', QString("%1(%2)[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(4)));
    characters.insert('Q', QString("%1(%2)[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(5)));
    characters.insert('=', QString("%1(%2)[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(6)));
    // extracting from btoa("false")
    characters.insert('U', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sfalse).arg(numbers.at(6)));
    // extracting from btoa("undefined")
    characters.insert('k', QString("%1(%2)[%3]").arg(btoaFunction).arg(Sundefined).arg(numbers.at(3)));
    // extracting from btoa("Infinity")
    characters.insert('S', QString("%1(%2)[%3]").arg(btoaFunction).arg(SInfinity).arg(numbers.at(0)));

    // extracting from double btoa(btoa(true))
    characters.insert('E', QString("%1(%1(%2))[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(1)));
    characters.insert('K', QString("%1(%1(%2))[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(3)));
    characters.insert('M', QString("%1(%1(%2))[%3]").arg(btoaFunction).arg(Strue).arg(numbers.at(4)));
}

QString Hieroglyphy::hChar(char c)
{
    if (characters.contains(c)) {
        return characters.value(c);
    }

    characters.insert(c, QString("%1(%2+%3)").arg(unescapeString).arg(characters['%']).arg(hString(toHex(c))));

    return characters.value(c);
}

QString Hieroglyphy::hUnicode(QChar u)
{
    return QString("%1(%2)").arg(fromCharCodeS).arg(hNumber(u.unicode()));
}

QString Hieroglyphy::hNumber(int num)
{
    if (num < 10) {
        return  numbers[num];
    }

    return QString("+(%1)").arg(hString(QString::number(num,10)));
}

QString Hieroglyphy::hString(QString str)
{
    QString final;
    for (int i = 0; i < str.size(); i++) {
        final.append((i > 0) ? "+" : "");
        QChar ch = str.at(i);
        if (ch.unicode() < 256) {
            final.append(hChar(ch.toLatin1()));
        } else {
            final.append(hUnicode(ch));
        }
    }

    return final;
}

QString Hieroglyphy::hScript(QString scr)
{
    return QString("%1(%2)()").arg(functionConstructor).arg(hString(scr));
}

QString Hieroglyphy::toHex(char c)
{
    QByteArray ba(1,c);
    return QString::fromUtf8(ba.toHex());
}

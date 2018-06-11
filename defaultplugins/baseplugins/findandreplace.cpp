#include "findandreplace.h"
#include "confgui/findandreplacewidget.h"
#include <QDebug>

const QString FindAndReplace::id = "FindAndReplace";

FindAndReplace::FindAndReplace()
{

}

FindAndReplace::~FindAndReplace()
{

}

QString FindAndReplace::name() const
{
    return id;
}

QString FindAndReplace::description() const
{
    return tr("Search for a specific hexadecimal sequence and replace it");
}

void FindAndReplace::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    if (input.isEmpty())
        return;

    output = input;
    QByteArray searchi = QByteArray::fromHex(searchExpr.toUtf8());
    QByteArray replacewithi = QByteArray::fromHex(replaceExpr.toUtf8());

    output.replace(searchi, replacewithi);
}

bool FindAndReplace::isTwoWays()
{
    return false;
}

QHash<QString, QString> FindAndReplace::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLREGEXP,searchExpr);
    properties.insert(XMLREPLACEWITH, replaceExpr);
    return properties;
}

bool FindAndReplace::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    setSearchExpr(propertiesList.value(XMLREGEXP));
    setReplaceExpr(propertiesList.value(XMLREPLACEWITH));
    return res;
}

QWidget *FindAndReplace::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) FindAndReplaceWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for FindAndReplaceWidget X{");
    }
    return widget;
}

QString FindAndReplace::help() const
{
    QString help;
    help.append("<p>Search for the specified hexadecimal sequence and replace it with the (second) specified one</p>");
    help.append("<p>Fairly trivial to use</p>");
    return help;
}

QString FindAndReplace::getSearchExpr() const
{
    return searchExpr;
}

void FindAndReplace::setSearchExpr(const QString &value)
{
    if (searchExpr != value) {
        searchExpr = value;
        QByteArray in = searchExpr.toUtf8();
        searchItem = QByteArray::fromHex(in);

        if (in != searchItem.toHex()) {
            searchExpr = QString::fromUtf8(searchItem.toHex());
            emit error(tr("Invalid hexadecimal values in search expression, all invalid characters will be ignored. %1").arg(QString::fromUtf8(searchItem.toHex())),id);
        }
        qDebug() << "search" << QString::fromUtf8(searchItem.toHex());
        emit confUpdated();
    }
}

QString FindAndReplace::getReplaceExpr() const
{
    return replaceExpr;
}

void FindAndReplace::setReplaceExpr(const QString &value)
{
    if (replaceExpr != value) {
        replaceExpr = value;
        QByteArray in = replaceExpr.toUtf8();
        ReplaceItem = QByteArray::fromHex(in);

        if (in != ReplaceItem.toHex()) {
            replaceExpr = QString::fromUtf8(ReplaceItem.toHex());
            emit error(tr("Invalid hexadecimal values in replace expression, all invalid characters will be ignored."),id);
        }

        emit confUpdated();
    }
}

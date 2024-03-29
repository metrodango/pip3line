/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "randomcase.h"
#include "confgui/randomcasewidget.h"
#include <QTime>
#include <QDebug>

RandomCase::RandomCase()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    rand = QRandomGenerator::securelySeeded();
#else
    qsrand(static_cast<uint>(QTime::currentTime().msec())); // non cryptographic quality, boo
#endif
}

const QString RandomCase::id = "Random Case";

QString RandomCase::name() const {
    return id;
}

QString RandomCase::description() const {
    return tr("Randomize the characters cases");
}

bool RandomCase::isTwoWays() {
    return false;
}

QWidget *RandomCase::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) RandomCaseWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for RandomCaseWidget X{");
    }
    return widget;
}

void RandomCase::reRandomize()
{
    emit confUpdated();
}

void RandomCase::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    QByteArray temp;
    for (int i = 0; i < input.size(); i++) {
        temp.append(input.at(i));
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        quint32 rval = rand.generate();
#else
        int rval = qrand();
#endif
        output.append((rval % 2 == 0) ? temp.toUpper() : temp.toLower());
        temp.clear();
    }
}

QString RandomCase::help() const
{
    QString help;
    help.append("<p>Randomize the characters case of the input (UTF-8)</p><p>i.e. rAndomIze thE cHaRActErS cAsE oF thE Input (UTf-8)</p>");
    return help;
}

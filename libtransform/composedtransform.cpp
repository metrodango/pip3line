/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "composedtransform.h"

const QString ComposedTransform::id = "Composed";

ComposedTransform::ComposedTransform(TransformChain transformChain) :
    chain(transformChain)
{
    twoWaysFlag = true;
    for (int i = 0; i < chain.size(); i++) {
        twoWaysFlag = chain.at(i)->isTwoWays() && twoWaysFlag;
        connect(chain.at(i), &TransformAbstract::error, this , &ComposedTransform::logError);
        connect(chain.at(i), &TransformAbstract::warning, this , &ComposedTransform::logWarning);
        defaultWays.append(chain.at(i)->way());
    }
}

ComposedTransform::~ComposedTransform()
{
    while (!chain.isEmpty())
        delete chain.takeLast();
}

QString ComposedTransform::name() const
{
    return chain.getName();
}

QString ComposedTransform::description() const
{
    return chain.getDescription().isEmpty() ? tr("Composed Transform") : chain.getDescription();
}

void ComposedTransform::transform(const QByteArray &input, QByteArray &output)
{
    QByteArray inter = input;

    if (!twoWaysFlag)
        wayValue = INBOUND;

    int chainSize = chain.size();
    int parity = chainSize % 2;
    if (wayValue == INBOUND) {
        for (int i = 0; i < chainSize; i++) {
            chain.at(i)->setWay(defaultWays.at(i));
            if (i % 2 == 0) {
                output.clear();
                chain.at(i)->transform(inter,output);
            }
            else {
                inter.clear();
                chain.at(i)->transform(output,inter);
            }
        }

        if (parity == 0)
            output = inter;

    } else {
        for (int i = chainSize - 1; i >= 0; i--) {
            chain.at(i)->setWay(defaultWays.at(i) == INBOUND ? OUTBOUND : INBOUND);
            if (i % 2 != parity)
                chain.at(i)->transform(inter,output);
            else
                chain.at(i)->transform(output,inter);
        }

        if (parity == 0)
            output = inter;
    }
}

bool ComposedTransform::isTwoWays()
{
    return twoWaysFlag;
}

QWidget *ComposedTransform::getGui(QWidget * /* parent */)
{
    return 0;
}

QString ComposedTransform::help() const
{
    QString help;
    help.append("<p>").append(name()).append("</p>");
    if (!chain.getHelp().isEmpty())
        help.append(chain.getHelp());
    help.append("<p>This transformation is composed with multiple other ones.</p><p>In INBOUND order: <ul>");
    for (int i = 0; i < chain.size(); i++) {
        help.append("<li>").append(chain.at(i)->name()).append("</li>");
    }
    help.append("</ul></p>");
    return help;
}

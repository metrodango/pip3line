/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "transformabstract.h"
#include "transformchain.h"

TransformChain::TransformChain()
{
    preferredFormat = TEXTFORMAT;
    preferredOutputType = ONELINE;
}

TransformChain::TransformChain(const TransformChain &other) : QList<TransformAbstract *>(other)
{
    name = other.name;
    description = other.description;
    help = other.help;
    preferredFormat = other.preferredFormat;
    preferredOutputType = other.preferredOutputType;
}

TransformChain &TransformChain::operator =(const TransformChain &other)
{
    QList<TransformAbstract *>::operator =(other);
    name = other.name;
    description = other.description;
    help = other.help;
    preferredFormat = other.preferredFormat;
    preferredOutputType = other.preferredOutputType;
    return *this;
}

TransformChain::~TransformChain()
{
}

void TransformChain::setName(const QString &nname)
{
    name = nname;
}

QString TransformChain::getName() const
{
    return name;
}

void TransformChain::setDescription(const QString &desc)
{
    description = desc;
}

QString TransformChain::getDescription() const
{
    return description;
}

void TransformChain::setHelp(const QString &helpString)
{
    help = helpString;
}

QString TransformChain::getHelp() const
{
    return help;
}

void TransformChain::setFormat(const OutputFormat &val)
{
    preferredFormat = val;
}

OutputFormat TransformChain::getFormat() const
{
    return preferredFormat;
}
Pip3lineConst::OutputType TransformChain::getPreferredOutputType() const
{
    return preferredOutputType;
}

void TransformChain::clearTransforms()
{
    for (int i = 0; i < size(); i++) {
        delete at(i);
    }
    clear();
}

void TransformChain::setPreferredOutputType(const Pip3lineConst::OutputType &value)
{
    preferredOutputType = value;
}


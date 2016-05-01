/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMCHAIN_H
#define TRANSFORMCHAIN_H

#include "libtransform_global.h"
#include <QList>

#include "commonstrings.h"


class TransformAbstract;

class LIBTRANSFORMSHARED_EXPORT TransformChain : public QList<TransformAbstract *>
{
    public:
        explicit TransformChain();
        TransformChain(TransformChain const &other);
        TransformChain& operator = (TransformChain const &other);
        ~TransformChain();
        void setName(const QString &name);
        QString getName() const;
        void setDescription(const QString &desc);
        QString getDescription() const;
        void setHelp(const QString &help);
        QString getHelp() const;
        void setFormat(const Pip3lineConst::OutputFormat &val);
        Pip3lineConst::OutputFormat getFormat() const;
        void setPreferredOutputType(const Pip3lineConst::OutputType &value);
        Pip3lineConst::OutputType getPreferredOutputType() const;
        void clearTransforms();
private:

        QString name;
        QString description;
        QString help;
        Pip3lineConst::OutputFormat preferredFormat;
        Pip3lineConst::OutputType preferredOutputType;
};

#endif // TRANSFORMCHAIN_H

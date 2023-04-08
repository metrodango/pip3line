/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef COMPOSEDTRANSFORM_H
#define COMPOSEDTRANSFORM_H

#include "transformabstract.h"
#include "transformchain.h"

class LIBTRANSFORMSHARED_EXPORT ComposedTransform : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit ComposedTransform(TransformChain transformChain);
        ~ComposedTransform();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * getGui(QWidget * parent);
        QString help() const;
        static const QString id;
   private:
        Q_DISABLE_COPY(ComposedTransform)
        TransformChain chain;
        bool twoWaysFlag;
        QList<TransformAbstract::Way> defaultWays;
};

#endif // COMPOSEDTRANSFORM_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef FIXPROTOCOL_H
#define FIXPROTOCOL_H

#include <transformabstract.h>
#include <QXmlQuery>

class FixProtocol : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit FixProtocol();
        ~FixProtocol() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
    private:
        QByteArray translateField(QByteArray val);
        QXmlQuery query;
};

#endif // FIXPROTOCOL_H

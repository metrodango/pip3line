/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef MD5_H
#define MD5_H

#include "transformabstract.h"

class Md5 : public TransformAbstract
{
    Q_OBJECT
    
public:
    explicit Md5() {}
    ~Md5() override {}
    QString name() const override;
    QString description() const override;
    void transform(const QByteArray &input, QByteArray &output) override;
    bool isTwoWays() override;
    static const QString id;
    QString help() const override;
};

#endif // MD5_H

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef CISCOSECRET7_H
#define CISCOSECRET7_H

#include <transformabstract.h>

class CiscoSecret7 : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit CiscoSecret7();
        ~CiscoSecret7() {}
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
        unsigned int getSeed() const;
        bool setSeed(unsigned int seed);
    private:
        unsigned int seed;
        void cipher(unsigned int seed, const QByteArray &data, QByteArray &output);
        static const QByteArray ckey;
        static const unsigned int MAXSEED;
};

#endif // CISCOSECRET7_H

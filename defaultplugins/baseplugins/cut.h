/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef CUT_H
#define CUT_H

#include "transformabstract.h"

class Cut : public TransformAbstract
{
    Q_OBJECT

    public:
        explicit Cut();
        ~Cut() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        int getFromPos();
        int getLength();
        bool doCutEverything();
        bool setFromPos(int val);
        bool setLength(int val);
        void setCutEverything(bool val);
        bool isClassicCut() const;
        void setClassicCut(bool value);
        bool isLineByLine() const;
        void setLineByLine(bool value);

    private:
        int from;
        int length;
        bool everything;
        bool classicCut;
        bool lineByLine;
};

#endif // CUT_H

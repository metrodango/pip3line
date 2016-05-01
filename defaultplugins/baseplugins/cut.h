/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CUT_H
#define CUT_H

#include "transformabstract.h"

class Cut : public TransformAbstract
{
    Q_OBJECT

    public:
        explicit Cut();
        ~Cut();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

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

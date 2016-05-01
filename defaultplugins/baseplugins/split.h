/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SPLIT_H
#define SPLIT_H

#include "transformabstract.h"

class Split : public TransformAbstract
{
    Q_OBJECT
    
    public:
        static const int MAXGROUPVALUE = 1000;
        explicit Split();
        ~Split();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        char getSeparator() const;
        int getSelectedGroup() const;
        bool doWeTakeAllGroup() const;
        bool isTrimmingBlank() const;
        bool isProcessingLineByLine() const;
        void setSeparator(char val);
        bool setSelectedGroup(int val);
        void setTakeAllGroup(bool val);
        void setTrimBlank(bool val);
        void setProcessLineByLine(bool val);
    private:
        void appendingToOutput(QByteArray &out, QByteArray &val);
        char separator;
        int group;
        bool allGroup;
        bool trimBlank;
        bool processLineByLine;
};

#endif // SPLIT_H

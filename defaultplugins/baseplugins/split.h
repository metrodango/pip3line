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
        ~Split() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

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

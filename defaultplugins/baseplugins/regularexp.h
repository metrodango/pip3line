/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef REGULAREXP_H
#define REGULAREXP_H

#include "transformabstract.h"

class RegularExp : public TransformAbstract
{
    Q_OBJECT
    
    public:
        enum Actions {EXTRACT = 0, REPLACE};
        explicit RegularExp();
        ~RegularExp() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;

        bool doWeProcessLineByLine();
        Actions getActionType();
        bool isUsingGreedyQuantifier();
        QString getRegularExpression();
        bool isCaseInsensitive();
        int getSelectedGroupIndex();
        bool isTakingAllGroup();
        QString getReplacementString();

        void setProcessLineByLine(bool val);
        void setActionType(Actions val);
        void setUsingGreedyQuantifier(bool val);
        void setExpression(QString exp);
        void setCaseInsensitive(bool val);
        bool setSelectedGroup(int val);
        void setAllGroups(bool val);
        void setReplacementString(QString val);
    private:
        void assignValues();
        bool processLineByLine;
        Actions actionType;
        bool useGreedyQuantifier;
        QString expression;
        bool caseInsensitive;
        int selectedGroup;
        bool allGroups;
        QString replacement;
};

#endif // REGULAREXP_H

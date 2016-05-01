/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
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
        ~RegularExp();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

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

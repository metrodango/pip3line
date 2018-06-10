#ifndef FILTERENGINE_H
#define FILTERENGINE_H

#include <QObject>
#include <QSharedPointer>
#include <QStack>
#include "filteritem.h"

class Packet;

class BasicToken
{
    public:
        enum TokenType {
            LPAR = 0,
            RPAR = 1,
            NOTOP = 2,
            BINARYOP =3,
            PROPOSITION = 4,
            INVALID = 5
        };
        explicit BasicToken(TokenType type = BasicToken::INVALID);
        virtual ~BasicToken();
        virtual bool evaluate(QSharedPointer<Packet> packet);
        TokenType getType() const;
        void setType(const TokenType &value);
        QSharedPointer<BasicToken> getParent() const;
        void setParent(QSharedPointer<BasicToken> value);
        virtual QString toStr();
    protected:
        TokenType type;
        QSharedPointer<BasicToken> parent;
};

class BinaryOperator : public BasicToken
{
    public:
        enum BinaryOps {
            AND = 0,
            OR = 1,
            XOR = 2
        };
        explicit BinaryOperator(BinaryOperator::BinaryOps op);
        ~BinaryOperator();
        BinaryOps getOp() const;
        void setOp(const BinaryOps &value);
        void setLoperand(QSharedPointer<BasicToken> value);
        QSharedPointer<BasicToken> getLoperand() const;
        void setRoperand(QSharedPointer<BasicToken> value);
        QSharedPointer<BasicToken> getRoperand() const;
        bool evaluate(QSharedPointer<Packet> packet);
        QString toStr();
    private:
        BinaryOps op;
        QSharedPointer<BasicToken> loperand;
        QSharedPointer<BasicToken> roperand;
};

class NotOperator : public BasicToken
{
    public:
        explicit NotOperator();
        ~NotOperator();
        bool evaluate(QSharedPointer<Packet> packet);
        void setOperand(QSharedPointer<BasicToken> value);
        QSharedPointer<BasicToken> getUoperand() const;
    private:
        QSharedPointer<BasicToken> uoperand;
};

class Operand : public BasicToken
{
    public:
        explicit Operand(QSharedPointer<FilterItem> fitem);
        ~Operand();
        bool evaluate(QSharedPointer<Packet> packet);
        QSharedPointer<FilterItem> getFitem() const;
        QString toStr();
    private:
        QSharedPointer<FilterItem> fitem;
};

class Parenthesis : public BasicToken
{
    public:
        enum ParenthesisType {
            Left,
            Right
        };
        explicit Parenthesis(Parenthesis::ParenthesisType ptype);
        ~Parenthesis();
        QSharedPointer<BasicToken> getChild() const;
        void setChild(QSharedPointer<BasicToken> value);
    private:
        QSharedPointer<BasicToken> child;
};

class FilterEngine : public QObject
{
        Q_OBJECT
    public:
        explicit FilterEngine(QObject *parent = nullptr);
        bool assertExpr(QString expression);
        bool evaluate(QSharedPointer<Packet> packet);
        QString getErrorStr() const;
        FilterItemsList getItems() const;
        void setItems(const FilterItemsList &value);
        bool getFilteringEnabled() const;
        void setFilteringEnabled(bool value);
        QString getExpressionStr() const;
    signals:
        void updated();
    private:
        QSharedPointer<BasicToken> buildTree(QList<QSharedPointer<BasicToken>> tokens);
        bool consolidateStacks(QStack<QSharedPointer<BasicToken> > &operands, QStack<QSharedPointer<BasicToken> > &operators);
        void debugprintstrlist(QStringList list);
        void debugPrintTree(QSharedPointer<BasicToken> rootToken);
        QJsonObject debugTreetoJson(QSharedPointer<BasicToken> token, int level);
        int calculateLength(QSharedPointer<BasicToken> token, int current);
        QSharedPointer<BasicToken> createToken(const QString &tokenStr);
        QString errorStr;
        QSharedPointer<BasicToken> rootToken;
        FilterItemsList items;
        bool filteringEnabled;
        QString expressionStr;
        static const QString NotOperatorStr;
        static const QString LeftPar;
        static const QString RightPar;
        static const QString AndOp;
        static const QString OrOp;
        static const QString XorOp;

};

#endif // FILTERENGINE_H

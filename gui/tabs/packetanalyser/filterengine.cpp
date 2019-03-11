#include "filterengine.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegExp>
#include <QStack>

BasicToken::BasicToken(BasicToken::TokenType type) :
    type(type)
{
   // qDebug() << "created" << this;
}

BasicToken::~BasicToken()
{
    //qDebug() << "destroyed" << this;
}

bool BasicToken::evaluate(QSharedPointer<Packet> )
{
    return false; // default
}

BasicToken::TokenType BasicToken::getType() const
{
    return type;
}

void BasicToken::setType(const TokenType &value)
{
    type = value;
}

QSharedPointer<BasicToken> BasicToken::getParent() const
{
    return parent;
}

void BasicToken::setParent(QSharedPointer<BasicToken> value)
{
    parent = value;
}

QString BasicToken::toStr()
{
    QString ret;
    if (type == LPAR) {
        ret = "(";
    } else if (type == RPAR) {
        ret = ")";
    } else if (type == NOTOP) {
        ret = "NOT";
    } else if (type == BINARYOP) {
        ret = "BIN";
    } else if (type == PROPOSITION) {
        ret = "PROPOSITION";
    } else {
        ret = "T_T";
    }

    return ret;
}

BinaryOperator::BinaryOperator(BinaryOperator::BinaryOps op) :
    BasicToken(BINARYOP),
    op(op)
{

}

BinaryOperator::~BinaryOperator()
{

}

BinaryOperator::BinaryOps BinaryOperator::getOp() const
{
    return op;
}

void BinaryOperator::setOp(const BinaryOps &value)
{
    op = value;
}

void BinaryOperator::setRoperand(QSharedPointer<BasicToken> value)
{
    roperand = value;
}

void BinaryOperator::setLoperand(QSharedPointer<BasicToken> value)
{
    loperand = value;
}

bool BinaryOperator::evaluate(QSharedPointer<Packet> packet)
{
    bool ret = false;
    bool lr = loperand->evaluate(packet);
    if (op == AND) {
        if (lr) {
            ret = roperand->evaluate(packet);
        } // short circuit, ignoring right operand

    } else if (op == OR) {
        if (lr) { // short circuit, ignoring right operand
            ret = true;
        } else {
            ret = roperand->evaluate(packet);
        }
    } else if (op == XOR) {
        ret = lr ^ roperand->evaluate(packet);
    }
    return ret;
}

QString BinaryOperator::toStr()
{
    QString ret;
    if (op == AND) {
        ret = "AND";
    } else if (op == OR) {
        ret = "OR";
    } else if (op == XOR) {
        ret = "XOR";
    } else {
        ret = "T_T";
    }

    return ret;
}

QSharedPointer<BasicToken> BinaryOperator::getLoperand() const
{
    return loperand;
}

QSharedPointer<BasicToken> BinaryOperator::getRoperand() const
{
    return roperand;
}

NotOperator::NotOperator() :
    BasicToken(NOTOP)
{

}

NotOperator::~NotOperator()
{

}

bool NotOperator::evaluate(QSharedPointer<Packet> packet)
{
    return !uoperand->evaluate(packet);
}

void NotOperator::setOperand(QSharedPointer<BasicToken> value)
{
    uoperand = value;
}

QSharedPointer<BasicToken> NotOperator::getUoperand() const
{
    return uoperand;
}

Operand::Operand(QSharedPointer<FilterItem> fitem) :
    BasicToken(PROPOSITION),
    fitem(fitem)
{

}

Operand::~Operand()
{

}

bool Operand::evaluate(QSharedPointer<Packet> packet)
{
    return fitem->selectable(packet);
}

QSharedPointer<FilterItem> Operand::getFitem() const
{
    return fitem;
}

QString Operand::toStr()
{
    return fitem->getName();
}

Parenthesis::Parenthesis(Parenthesis::ParenthesisType ptype)
{
    if (ptype == Left) {
        this->type = BasicToken::LPAR;
    } else {
        this->type = BasicToken::RPAR;
    }
}

Parenthesis::~Parenthesis()
{

}

QSharedPointer<BasicToken> Parenthesis::getChild() const
{
    return child;
}

void Parenthesis::setChild(QSharedPointer<BasicToken> value)
{
    child = value;
}

const QString FilterEngine::NotOperatorStr = "NOT";
const QString FilterEngine::LeftPar = "(";
const QString FilterEngine::RightPar = ")";
const QString FilterEngine::AndOp = "AND";
const QString FilterEngine::OrOp = "OR";
const QString FilterEngine::XorOp = "XOR";

FilterEngine::FilterEngine(QObject *parent) : QObject(parent)
{
    filteringEnabled = false;
}

bool FilterEngine::assertExpr(QString expression)
{
    bool ret = false;
    QStringList strTokens;
    QSharedPointer<BasicToken> tok;
    QList<QSharedPointer<BasicToken> > tokenList;

    rootToken.clear();

    expression = expression.trimmed();

    if (expression.isEmpty()) { // nothing to assert, building the tree with ANDs (not foot)
        if (items.size() == 0) { // nothing to see here
            return true;
        }

        rootToken = QSharedPointer<BasicToken>(new(std::nothrow) Operand(items.at(0)));
        for (int i = 1; i < items.size(); i++) {
            tok = QSharedPointer<BasicToken>(new(std::nothrow) Operand(items.at(i)));
            QSharedPointer<BinaryOperator> bo = QSharedPointer<BinaryOperator>(new(std::nothrow) BinaryOperator(BinaryOperator::AND));
            bo->setLoperand(rootToken);
            bo->setRoperand(tok);
            rootToken = bo;
        }
        expressionStr.clear();
        emit updated();
        return true;
    }

    // spliting by space
    strTokens = expression.split(" ",QString::SkipEmptyParts);
    //debugprintstrlist(strTokens);
    // splitting by parenthesis
    QStringList temp;
    for (int i = 0 ; i < strTokens.size(); i++) {
        QString str = strTokens.at(i);
        int last = 0;
        for (int j = 0; j < str.size(); j++) {
            if (str.at(j) == LeftPar || str.at(j) == RightPar) {
                QString st1 = str.mid(last, j - last);
                if (!st1.isEmpty()) {
                    temp.append(st1);
                }

                temp.append(str.at(j));
                last = j + 1;
            }
        }

        // add the rest, if any
        if (str.size() - last > 0)
            temp.append(str.mid(last));
    }



    strTokens = temp;
    //debugprintstrlist(strTokens);
    temp.clear();
    // Lexer
    for (int i = 0; i < strTokens.size(); i++) {
        QString str = strTokens.at(i);
        tok = createToken(str);
        if (!tok.isNull()) {
            tokenList.append(tok);
        } else {
            errorStr = QString("Invalid token found: %1").arg(str);
            return ret;
        }
    }

   // qDebug() << "Lexer token count" << tokenList.size();
    if (tokenList.size() == 0) {
        qCritical() << tr("[FilterEngine::assert] tokenlist is empty T_T");
        return false;
    }
    // Parser

    if (tokenList.size() == 1) { // no tree to build here ...
        tok = tokenList.at(0);
        // in this case the only tokens allowed are propositions
        if (tok->getType() == BasicToken::PROPOSITION)
            rootToken = tokenList.at(0);
        else {
            errorStr = QString("Invalid expression");
            goto syntax_error;
        }
    } else {
        // toherwise build the tree
        rootToken = buildTree(tokenList);
        if (rootToken.isNull())
            goto syntax_error;
    }

    // if we are here everything went fine ... hopfully
    expressionStr = expression;
    ret = true;
    emit updated();

syntax_error:
#ifndef QT_NO_DEBUG
    debugPrintTree(tok);
#endif
    return ret;
}

QSharedPointer<BasicToken> FilterEngine::buildTree(QList<QSharedPointer<BasicToken> > tokens)
{
    QSharedPointer<BasicToken> final;
    QSharedPointer<BasicToken> next;
    QSharedPointer<BasicToken> current;
    QSharedPointer<BasicToken> prev;
    BasicToken::TokenType curType;
    BasicToken::TokenType nextType = BasicToken::INVALID;

    QStack<QSharedPointer<BasicToken> > operands;
    QStack<QSharedPointer<BasicToken> > operators;

    errorStr.clear();

    while (!tokens.isEmpty()) {
         current = tokens.takeFirst();
         curType = current->getType();
         if (curType == BasicToken::LPAR) {
             // just push the LPAR onto the operand stack
             operands.push(current);
         } else if (curType == BasicToken::NOTOP) {
             // need an operand
             if (tokens.isEmpty())  {
                 errorStr = tr("NOT needs an operand");
                 goto buildTree2_end;
             }

             next = tokens.first();
             nextType = next->getType();
             if (nextType == BasicToken::PROPOSITION) { // we can build a branch here
                 // getting the proposition
                 next = tokens.takeFirst();
                 QSharedPointer<NotOperator> notop = current.staticCast<NotOperator>();
                 // link the not and the prop
                 notop->setOperand(next);
                 next->setParent(current);
                 // push onto the operand stack
                 operands.push(current);
             } else if (nextType == BasicToken::NOTOP) {
                 // two not's cancel each other (king of pun)
                 next = tokens.takeFirst();
             } else if (nextType == BasicToken::LPAR) {
                 // just push the not onto the operator stack
                 operators.push(current);
             } else {
                 errorStr =  tr("Invalid token after NOT");
                 goto buildTree2_end;
             }

         } else if (curType == BasicToken::BINARYOP) {
             // need a left operand
             if (operands.isEmpty()) {
                 errorStr =  tr("%1 needs a left operand").arg(current->toStr());
                 goto buildTree2_end;
             }
             // need a right operand
             if (tokens.isEmpty()) {
                 errorStr =  tr("%1 needs a right operand").arg(current->toStr());
                 goto buildTree2_end;
             }

             // the top stack operand cannot be LPAR of RPAR (RPAR is never added to the operand stack)
             if (operands.top()->getType() == BasicToken::LPAR)  {
                 errorStr =  tr("\"%1\" cannot follow \"%2\" ")
                            .arg(current->toStr())
                            .arg(operands.top()->toStr());
                 goto buildTree2_end;
             }

             next = tokens.first();
             nextType = next->getType();
             if (nextType == BasicToken::PROPOSITION) {// we can build a branch here
                 // getting the proposition
                 next = tokens.takeFirst();
                 QSharedPointer<BinaryOperator> bo = current.staticCast<BinaryOperator>();
                 prev = operands.pop();
                 bo->setLoperand(prev);
                 prev->setParent(current);
                 bo->setRoperand(next);
                 next->setParent(current);
                 // push onto the operand stack
                 operands.push(current);
             } else if (nextType == BasicToken::NOTOP || nextType == BasicToken::LPAR) {
                 // just push the not onto the operator stack
                 operators.push(current);
             } else {
                 errorStr = tr("\"%1\" cannot follow \"%2\" ")
                            .arg(next->toStr())
                            .arg(current->toStr());
                 // invalid right operand
                 goto buildTree2_end;
             }
         } else if (curType == BasicToken::PROPOSITION) {
             // it would only happens for the very first token
             operands.push(current);
         } else if (curType == BasicToken::RPAR) {
             // need an operand here ..
             if (operands.isEmpty()) {
                 errorStr = tr("\"%1\" incomplete").arg(current->toStr());
                 goto buildTree2_end;
             }

             // top operand can't be LPAR
             if (operands.top()->getType() == BasicToken::LPAR)  {
                 errorStr = tr("Empty parenthesis");
                 goto buildTree2_end;
             }

             // take the top operand (we don't care abobut RPAR anymore)
             current = operands.pop();

             if (operands.isEmpty() || operands.top()->getType() != BasicToken::LPAR) {
                 // we are missing a LPAR
                 errorStr = tr("Missing \"(\"");
                 goto buildTree2_end;
             }

             operands.pop(); // don't care about this LPAR anymore

             // we need to know of the next operand is LPAR
             if (operands.isEmpty()) {
                 next.clear();
             } else {
                next = operands.top();
             }
             // putting the operand back onto the operand stack
             operands.push(current);
             // if not LPAR or at the top then consolidate the tree
             if (!next.isNull() && next->getType() != BasicToken::LPAR) {
                 // consolidating the tree up to the next LPAR, or the top
                 if (!consolidateStacks(operands, operators)) {
                     goto buildTree2_end;
                 }
             }

         } else {
             errorStr = tr("Invalid token \"%1\"").arg(current->toStr());
             goto buildTree2_end;
         }
    }

    // consolidating the stacks
    if (!consolidateStacks(operands, operators)) {
        goto buildTree2_end;
    }

    // at the end there has to be one and only one operand
    // and the operators stack should be empty
    if (operators.size() != 0 || operands.size() != 1)  {
        if (errorStr.isEmpty())
            errorStr = tr("Invalid expression");
        goto buildTree2_end;
    }

    final = operands.pop();

buildTree2_end:
    return final;
}

bool FilterEngine::consolidateStacks(QStack<QSharedPointer<BasicToken> > &operands, QStack<QSharedPointer<BasicToken> > &operators)
{
    // consolidate the tree up to the last LPAR

    QSharedPointer<BasicToken> current;
    QSharedPointer<BasicToken> prev;
    BasicToken::TokenType curType;

    while (!operators.isEmpty()) {

        current = operators.pop();
        curType = current->getType();

        if (curType == BasicToken::NOTOP) {
            // need an operand
            if (operands.isEmpty())  {
                errorStr = tr("NOT needs an operand");
                return false;
            }

            QSharedPointer<NotOperator> notop = current.staticCast<NotOperator>();

            prev = operands.pop();
            if (prev->getType() == BasicToken::LPAR) {
                errorStr = tr("Missing )");
                return false;
            }
            notop->setOperand(prev);
            prev->setParent(notop);
        } else if (curType == BasicToken::BINARYOP) {
            // need two operands here
            if (operands.size() < 2)  {
                errorStr = tr("%1 needs two operands").arg(current->toStr());
                return false;
            }

            QSharedPointer<BinaryOperator> bo = current.staticCast<BinaryOperator>();

            // getting and setting the right operand
            prev = operands.pop();
            if (prev->getType() == BasicToken::LPAR) {
                errorStr = tr("Missing )");
                return false;
            }
            bo->setRoperand(prev);
            prev->setParent(bo);

            // getting and setting the left operand
            prev = operands.pop();
            if (prev->getType() == BasicToken::LPAR) {
                errorStr = tr("Missing )");
                return false;
            }
            bo->setLoperand(prev);
            prev->setParent(bo);
        } else {
            qCritical() << tr("invalid operator in the operator stack T_T");
            return false;
        }
        if (operands.isEmpty()) {
            // if no operand, just push the operator on the operand stack
            operands.push(current);
        } else {
            // need to check if the next operand is LPAR after pushing the operator on the operand stack
            QSharedPointer<BasicToken> next = operands.top();
            operands.push(current);
            // if LPAR then return (not considered an error here, but maybe later)
            if (next->getType() == BasicToken::LPAR) {
                break;
            }
        }
    }

    return true;
}

bool FilterEngine::evaluate(QSharedPointer<Packet> packet)
{

    bool ret = true;

    // returning true if filter disabled
    if (filteringEnabled) {
        // returning true if root token is null
        if (!rootToken.isNull()) {
            ret = rootToken->evaluate(packet);;
        }
    }
    return ret;
}

QString FilterEngine::getErrorStr() const
{
    return errorStr;
}

FilterItemsList FilterEngine::getItems() const
{
    return items;
}

void FilterEngine::setItems(const FilterItemsList &value)
{
    items = value;
    emit updated();
}

void FilterEngine::debugprintstrlist(QStringList list)
{
    QString res =  "['";
    if (list.size() > 0) {
        for (int i = 0; i < list.size(); i++) {
            res.append(list.at(i)).append("','");
        }
        res.chop(2);
    }
    res.append("]");
}

void FilterEngine::debugPrintTree(QSharedPointer<BasicToken> token)
{
    if (token.isNull()) {
        qCritical() << "[debugPrintTree] root token is null T_T";
        return;
    }
    if (!token->getParent().isNull()) {
        // search for the top element
        QSharedPointer<BasicToken> parent = token->getParent();
        while (!parent.isNull()) {
            token = parent;
            parent = token->getParent();
        }
    }

    // calculate depth of the tree
   // int length = calculateLength(token, 0);
   // qDebug() << tr("Tree depth: %1").arg(length);

    QJsonObject obj = debugTreetoJson(token, 0);
    QJsonDocument doc;
    doc.setObject(obj);

    QByteArray output = doc.toJson();
    QTextStream cout(stdout);
    cout << QString::fromUtf8(output);
}

QJsonObject FilterEngine::debugTreetoJson(QSharedPointer<BasicToken> token, int level)
{

    QJsonObject obj;

    if (level > 100) {
        qCritical() << tr("[debugTreetoStr] more than a 100 , really ?? T_T");
        return obj;
    }

    if (token->getType() == BasicToken::PROPOSITION) {
        QSharedPointer<Operand> oper = token.staticCast<Operand>();
        obj.insert("item", oper->getFitem()->getName());
    } else if (token->getType() == BasicToken::BINARYOP) {

        QSharedPointer<BinaryOperator> bo = token.staticCast<BinaryOperator>();
        obj.insert("OP", bo->toStr());

        if (bo->getLoperand().isNull()) {
            obj.insert("Left", "NULL");
        } else {
            obj.insert("Left", debugTreetoJson(bo->getLoperand(), level + 1));
        }

        if (bo->getRoperand().isNull()) {
            obj.insert("Right", "NULL");
        } else {
            obj.insert("Right", debugTreetoJson(bo->getRoperand(), level + 1));
        }
    } else if (token->getType() == BasicToken::NOTOP) {
         QSharedPointer<NotOperator> bo = token.staticCast<NotOperator>();
        if (bo->getUoperand().isNull()) {
            obj.insert("Not", "NULL");
        } else {
            obj.insert("Not", debugTreetoJson(bo->getUoperand(), level + 1));
        }
    } else if (token->getType() == BasicToken::RPAR || token->getType() == BasicToken::LPAR) {
        QSharedPointer<Parenthesis> bo = token.staticCast<Parenthesis>();
        QString pt = (token->getType() == BasicToken::LPAR ? "(" : ")");
        if (bo->getChild().isNull()) {
            obj.insert(pt, "NULL");
        } else {
            obj.insert(pt, debugTreetoJson(bo->getChild(), level + 1));
        }
    } else if (token->getType() == BasicToken::INVALID) {
        obj.insert("INVALID", "INVALID");
    }

    return obj;
}

int FilterEngine::calculateLength(QSharedPointer<BasicToken> token, int current)
{
    if (current > 100) {
        qCritical() << tr("[debugPrintTree] more than a 100 , really ?? T_T");
        return -1;
    }
    if (token->getType() != BasicToken::PROPOSITION) {
        if (token->getType() == BasicToken::BINARYOP) {
            current++;
            QSharedPointer<BinaryOperator> bo = token.staticCast<BinaryOperator>();
            int ll = -1;
            int lr = -1;
            if (bo->getLoperand().isNull()) {
                qDebug() << tr("[debugPrintTree] Invalid tree, LOPERAND is null");
            } else {
                ll = calculateLength(bo->getLoperand(), current);
            }

            if (bo->getRoperand().isNull()) {
                qDebug() << tr("[debugPrintTree] Invalid tree, ROPERAND is null");
            } else {
                lr = calculateLength(bo->getRoperand(),current);
            }
            if (lr < 0 && ll < 0) {
                current--;
            } else if (lr < 0) {
                current = ll;
            } else if (ll < 0) {
                current = lr;
            } else {
                current = qMax(ll,lr);
            }
            return current;
        } else if (token->getType() == BasicToken::NOTOP) {
            current++;
            QSharedPointer<NotOperator> bo = token.staticCast<NotOperator>();
            if (bo->getUoperand().isNull()) {
                current--;
                qDebug() << tr("[debugPrintTree] Invalid tree, UOPERAND is null");
                return current; // we don't error here
            }
            return calculateLength(bo->getUoperand(),current);
        } else if (token->getType() == BasicToken::RPAR || token->getType() == BasicToken::LPAR) {
            current++;
            if (token->getType() == BasicToken::RPAR) {
                qDebug() << tr("[debugPrintTree] Invalid tree, RPAR found");
            } else {
                 qDebug() << tr("[debugPrintTree] Invalid tree, LPAR found");
            }
            QSharedPointer<Parenthesis> bo = token.staticCast<Parenthesis>();
            if (bo->getChild().isNull()) {
                current--;
                qDebug() << tr("[debugPrintTree] Invalid tree, P CHILD is null");
                return current;
            }
            return calculateLength(bo->getChild(),current);
        } else if (token->getType() == BasicToken::INVALID) {
            qDebug() << tr("[debugPrintTree] Invalid tree, INVALID found at %").arg(current);
            return current;
        }
    }

    return current;
}



QSharedPointer<BasicToken> FilterEngine::createToken(const QString &tokenStr)
{
    QSharedPointer<BasicToken> ret;
    QString upper = tokenStr.toUpper();
    if (upper == NotOperatorStr) {
        ret = QSharedPointer<BasicToken>(new(std::nothrow) NotOperator());
    } else if (upper == LeftPar) {
        ret = QSharedPointer<BasicToken>(new(std::nothrow) Parenthesis(Parenthesis::Left));
    } else if (upper == RightPar) {
        ret = QSharedPointer<BasicToken>(new(std::nothrow) Parenthesis(Parenthesis::Right));
    } else if (upper == AndOp) {
        ret = QSharedPointer<BasicToken>(new(std::nothrow) BinaryOperator(BinaryOperator::AND));
    } else if (upper == OrOp) {
        ret = QSharedPointer<BasicToken>(new(std::nothrow) BinaryOperator(BinaryOperator::OR));
    } else if (upper == XorOp) {
        ret = QSharedPointer<BasicToken>(new(std::nothrow) BinaryOperator(BinaryOperator::XOR));
    } else if (items.containsFilter(tokenStr)) {
        ret = QSharedPointer<BasicToken>(new(std::nothrow) Operand(items.getFilter(tokenStr)));
    }

    return ret;
}

QString FilterEngine::getExpressionStr() const
{
    return expressionStr;
}

bool FilterEngine::getFilteringEnabled() const
{
    return filteringEnabled;
}

void FilterEngine::setFilteringEnabled(bool value)
{
    if (filteringEnabled != value) {
        filteringEnabled = value;
        emit updated();
    }
}

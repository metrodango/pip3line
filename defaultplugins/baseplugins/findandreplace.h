#ifndef FINDANDREPLACE_H
#define FINDANDREPLACE_H

#include "transformabstract.h"

class FindAndReplace : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit FindAndReplace();
        ~FindAndReplace();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;
        QString getSearchExpr() const;
        void setSearchExpr(const QString &value);

        QString getReplaceExpr() const;
        void setReplaceExpr(const QString &value);

    private:
        QByteArray searchItem;
        QByteArray ReplaceItem;
        QString searchExpr;
        QString replaceExpr;
};

#endif // FINDANDREPLACE_H

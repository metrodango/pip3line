#ifndef FINDANDREPLACE_H
#define FINDANDREPLACE_H

#include "transformabstract.h"

class FindAndReplace : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit FindAndReplace();
        ~FindAndReplace() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        static const QString id;
        QString help() const override;
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

/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef SUBSTITUTION_H
#define SUBSTITUTION_H

#include <transformabstract.h>
#include <QVector>

class Substitution : public TransformAbstract
{
        Q_OBJECT
    public:
        static QStringList knownTables;
        explicit Substitution();
        ~Substitution() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        QWidget * requestGui(QWidget * parent) override;
        QString inboundString() const override;
        QString outboundString() const override;
        QByteArray getSTable();
        void setSTable(QByteArray sTable);
        bool isPermutationValid(QByteArray sTable);
        QByteArray getPredeterminedTable(QString name);
        QString help() const override;

    private:
        static char s_table_tcpa[256];
        void createDecryptTable();
        QByteArray s_table_e;
        QByteArray s_table_d;
};

#endif // SUBSTITUTION_H

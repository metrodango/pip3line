/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
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
        ~Substitution();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        QString inboundString() const;
        QString outboundString() const;
        QByteArray getSTable();
        void setSTable(QByteArray sTable);
        bool isPermutationValid(QByteArray sTable);
        QByteArray getPredeterminedTable(QString name);
        QString help() const;

    private:
        static char s_table_tcpa[256];
        void createDecryptTable();
        QByteArray s_table_e;
        QByteArray s_table_d;
};

#endif // SUBSTITUTION_H

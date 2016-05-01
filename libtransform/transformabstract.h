/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMABSTRACT_H
#define TRANSFORMABSTRACT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QHash>
#include "commonstrings.h"
#include "libtransform_global.h"
using namespace Pip3lineConst;

class QWidget;

class LIBTRANSFORMSHARED_EXPORT TransformAbstract : public QObject
{
    Q_OBJECT
    public:
        // enumeration to define the direction of the transformation
        // note: not all transformations are bi-directional
        enum Way {INBOUND, OUTBOUND};
        TransformAbstract();
        virtual ~TransformAbstract();
        // mandatory methods to implement
        virtual QString name() const = 0;
        virtual QString description() const = 0;
        virtual void transform(const QByteArray &input, QByteArray &output) = 0;

        // mandatory configuration methods used for cloning objects within the gui
        // if your transforms have internal properties, you need to advertise and
        // load them here
        virtual QHash<QString, QString> getConfiguration();
        virtual bool setConfiguration(QHash<QString, QString> propertiesList);

        // methods only used for gui coolness (optionals)
        virtual bool isTwoWays();
        QWidget * getGui(QWidget * parent); // beware this one is not virtual anymore use requestGui() instead
        virtual QString inboundString() const;
        virtual QString outboundString() const;
        // Optional Help string. Will be accessible from the Gui
        virtual QString help() const;
        // Optional Credits. Will be accessible from the Gui
        virtual QString credits() const;
        // Base functions setting the current direction of the transformation process
        void setWay(Way nway);
        TransformAbstract::Way way();

        // Convenience function which just calls "void transform(const QByteArray &input, QByteArray &output)"
        QByteArray transform(const QByteArray &input);
        // static utility functions
        static QByteArray toPrintableString(const QByteArray &val, bool strict = true);
        static bool isPrintable(const qint32 c);

    protected:
        // Utilities functions used for conversion
        QByteArray fromHex(QByteArray in);
        QString saveChar(const char c) const;
        bool loadChar(const QString &val, char *c);
    protected Q_SLOTS:
        // Convenience functions used to manage errors and warnings internally
        virtual void logError(const QString message, const QString source);
        virtual void logWarning(const QString message, const QString source);

    Q_SIGNALS:
        void confUpdated(); // to be emitted when the internal configuration has been modified, so that the gui get notified
        void error(const QString, const QString); // signal for an error message (message, source)
        void warning(const QString, const QString); // signal for warning message (message, source)

    protected:
        virtual QWidget * requestGui(QWidget * parent);
        Way wayValue; // store the "direction" of the Transformation when needed

    private Q_SLOTS:
        void onGuiDelete();
    private:
        Q_DISABLE_COPY(TransformAbstract)
        static const QByteArray HEXCHAR;
        QWidget *confGui; // this is the configuration widget, if needed. if the pointer is null, the gui will just ignore the setting tab.
};

#endif // TRANSFORMABSTRACT_H

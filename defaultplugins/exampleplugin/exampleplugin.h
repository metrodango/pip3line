/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include "exampleplugin_global.h"
#include <transformfactoryplugininterface.h>

class Pip3lineCallback;
class QWidget;

// Nothing extravagant here, just the usual stuff: implementing all the virtual methods of the interface
// Note: the plugin has to inherits from QObject and the interface, IN THAT ORDER.

class EXAMPLEPLUGINSHARED_EXPORT ExamplePlugin : public QObject, public TransformFactoryPluginInterface {
        Q_OBJECT // mandatory when inheriting from QObject
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "exampleplugin.json") // mandatory for QT5. the json file is included automatically in the plugin during compilation.
#endif
        Q_INTERFACES(TransformFactoryPluginInterface) // mandatory when implementing a plugin
    public:
        explicit ExamplePlugin();
        // This function set a Pip3lineCallback object for the plugin to interact with the upper layer
        // It can be seen as an initialisation call, as it is called just after the plugin is instanciated
        void setCallBack(Pip3lineCallback *);
        ~ExamplePlugin();
        // The plugin name helps differentiate plugins and should describe them. If two plugins return the same
        // string here, only the first one will be loaded
        QString pluginName() const;
        // This should return a Transformation object described by the given name (with default configuration)
        // Must return null on error or if the name is not known
        TransformAbstract * getTransform(QString name);
        // Return the list of Transformation name for the given type (needed to build the combobox in the gui)
        const QStringList getTransformList(QString typeName);
        // Should return a gui object that interfaces itself with the plugin, or null
        QWidget * getConfGui(QWidget * parent);
        // Should return the list of type names used by this plugin
        // There are some default type names, but you can specify any name really, with a word of caution:
        // if the there is no Transformation fitting under a type, the type will not be appear in the list.
        const QStringList getTypesList();

        // Used for debugging, you can return random data here it won't change a thing.
        // Ideally returns the macro value QT_VERSION_STR
        QString compiledWithQTversion() const;
        // If the number returned here is different from the one of the calling program, the plugin will be ignored.
        // the number is given by the macro LIB_TRANSFORM_VERSION (see cpp file)
        int getLibTransformVersion() const;
        // Return the version number of the plugin as a string
        // Will be disclosed when loading the plugin, does not have any effect otherwise
        QString pluginVersion() const;
};

#endif // EXAMPLEPLUGIN_H

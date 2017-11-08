/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "exampleplugin.h"
#include "identity.h"
#include <QWidget>
#include <pip3linecallback.h>
#include <QDebug>
#include "../../version.h"

ExamplePlugin::ExamplePlugin()
{
    // nothing new
}

void ExamplePlugin::setCallBack(Pip3lineCallback *)
{
    // Set the callback object, to interact with the higher layers
    // This function is called once right after creating the object
    // It is a good idea to perform your initialization(s) here as you
    // can use the callback object to log any errors/warnings
}

ExamplePlugin::~ExamplePlugin()
{
    // nothing new
    qDebug() << "Destroying " << this;
}

QString ExamplePlugin::pluginName() const
{
    // This name must be unique, i.e. not be in used by another plugin
    // if there is a duplicate, the calling programme will only load the first instance and ignore the rest
    return "Example plugin";
}

TransformAbstract *ExamplePlugin::getTransform(QString name)
{
    // This is where the Transform object are being created
    // The programme will pass the name of the plugin it is looking for

    if (Identity::id == name) {
        return new Identity();
    }
    // you should not delete the object, the main program will delete it itself
    return 0; // return null on error, or if the name is not known
}

const QStringList ExamplePlugin::getTransformList(QString typeName)
{
// This function is used to register/load the Transforms list name into the main programme
// It takes a type name as parameter so that the calling programme can sort the Transforms by category

    QStringList ret;
    if (typeName == DEFAULT_TYPE_MISC) {
        ret.append(Identity::id);
    }
    return ret;
}

QWidget *ExamplePlugin::getConfGui(QWidget * /* parent */)
{
    // You can return a gui QWidget oject to manage your plugin via the gui
    // Return nullptr otherwise
    // you don't have to delete your object, the main program will take care of this.
    // or you can delete it yourself, whatever suits you, it should be guaranted
    // to work in both cases due to Qt magic
    return nullptr;
}

// This function returns a list of transformation types name used by this plugin
// The list is arbitrary, so you can create your own types
// the default ones are defined in libtransform commonstrings.h, for example:
// DEFAULT_TYPE_ENCODER "Encoders"
// DEFAULT_TYPE_MISC "Misc."
// DEFAULT_TYPE_CRYPTO "Crypto"
// DEFAULT_TYPE_HACKING "Hacking"
// DEFAULT_TYPE_HASHES "Hashes"
//
// The types' names are limited in length to avoid breaking the gui,
// anything above TransformMgmt::MAX_TYPE_NAME_LENGTH will be truncated
const QStringList ExamplePlugin::getTypesList()
{
    return QStringList() << DEFAULT_TYPE_MISC;
}

// This function is used to detect if this plugin was compiled with a different version
// of libtransform than the main programme, so it should just return LIB_TRANSFORM_VERSION.
int ExamplePlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

// No restriction here, you can return whatever you want
QString ExamplePlugin::pluginVersion() const
{
    return VERSION_STRING;
}

// this function is used for debugging purposes.
// should ideally return QT_VERSION_STR
QString ExamplePlugin::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}


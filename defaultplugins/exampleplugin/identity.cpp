/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "identity.h"
#include <QWidget>

const QString Identity::id = "Identity";

Identity::Identity()
{
    // constructor, nothing new
}

Identity::~Identity()
{
    // destructor, nothing new
}

QString Identity::name() const {
    // return the identifier of your transformation
    // it MUST be unique from the ones already loaded
    // In case of duplicate the application will keep the first one and ignore the rest
    return id;
}

QString Identity::description() const {
    // short description of what your transformation does
    return tr("Identity transformation (not really useful)");
}

bool Identity::isTwoWays() {
    // Function used to tell the Gui if
    // it needs to display the encode/decode buttons
    return true;
}

void Identity::transform(const QByteArray &input, QByteArray &output) {
    // And here is the main part, where you actually do the dirty work
    // this method is NOT protected against concurrent thread call, so be carefull if you
    // use resources that are shared
    output = input;
}

// Mandatory methods, they are used to clone Transform objects within the gui

QHash<QString, QString> Identity::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    // add here whatever property you want to save
    // You can use the predefined tag names from
    // xmlxommons.h
    // but that's optional
    // the properties names (keys of the hash table) must match this regexp : "^[a-zA-Z_][-a-zA-Z0-9_\\.]{1,100}$"
    // otherwise the property is ignored

    return properties;
}

bool Identity::setConfiguration(QHash<QString, QString> propertiesList)
{
    // used to configure this class from a list of properties
    // should return true if the configuration succeded
    // false if there has been any error
    return TransformAbstract::setConfiguration(propertiesList);
}

// optional
QWidget *Identity::requestGui(QWidget *parent)
{
    //Ideally this should be something like:
     return new QWidget(parent);
    // where QWidget is replaced with your own configuration widget linked with the transform object

    // you don't need to delete this object yourself, either the upperlayer gui or the
    // abstract destructor will take care of it
    // you also don't need to worry about duplicate as the Abstract object is taking care of it
}


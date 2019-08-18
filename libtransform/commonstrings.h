/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef XMLCOMMON_H
#define XMLCOMMON_H
#include <QString>
#include "libtransform_global.h"

#define LIB_TRANSFORM_VERSION 18

namespace Pip3lineConst {
    // Default types
    EXPORTING const QString DEFAULT_TYPE_ENCODER;
    EXPORTING const QString DEFAULT_TYPE_MISC;
    EXPORTING const QString DEFAULT_TYPE_CRYPTO;
    EXPORTING const QString DEFAULT_TYPE_HACKING;
    EXPORTING const QString DEFAULT_TYPE_HASHES;
    EXPORTING const QString DEFAULT_TYPE_TYPES_CASTING;
    EXPORTING const QString DEFAULT_TYPE_PARSERS;
    EXPORTING const QString DEFAULT_TYPE_NUMBER;
    EXPORTING const QString DEFAULT_TYPE_USER;

    // Default XML tags names
    EXPORTING const QString XMLPIP3LINECONF;
    EXPORTING const QString XMLDESCRIPTION;
    EXPORTING const QString XMLHELP;
    EXPORTING const QString XMLVERSIONMAJOR;
    EXPORTING const QString XMLORDER;
    EXPORTING const QString PROP_NAME;
    EXPORTING const QString PROP_WAY;
    EXPORTING const QString XMLTRANSFORM;
    EXPORTING const QString XMLVARIANT;
    EXPORTING const QString XMLPADDINGCHAR;
    EXPORTING const QString XMLINCLUDEPADDING;
    EXPORTING const QString XMLPADDINGTYPE;
    EXPORTING const QString XMLKEY;
    EXPORTING const QString XMLFROMHEX;
    EXPORTING const QString XMLPERCENTCHAR;
    EXPORTING const QString XMLINCLUDE;
    EXPORTING const QString XMLEXCLUDE;
    EXPORTING const QString XMLCHOOSENTARGET;
    EXPORTING const QString XMLBLOCKSIZE;
    EXPORTING const QString XMLNOBLOCK;
    EXPORTING const QString XMLUSEENTITYNAMES;
    EXPORTING const QString XMLUSEHEXA;
    EXPORTING const QString XMLENCODEALL;
    EXPORTING const QString XMLISESCAPED;
    EXPORTING const QString XMLCODECNAME;
    EXPORTING const QString XMLCHAR62;
    EXPORTING const QString XMLCHAR63;
    EXPORTING const QString XMLFROM;
    EXPORTING const QString XMLLENGTH;
    EXPORTING const QString XMLEVERYTHING;
    EXPORTING const QString XMLSEPARATOR;
    EXPORTING const QString XMLGROUP;
    EXPORTING const QString XMLACTIONTYPE;
    EXPORTING const QString XMLGREEDYQUANT;
    EXPORTING const QString XMLREGEXP;
    EXPORTING const QString XMLCASEINSENSITIVE;
    EXPORTING const QString XMLALLGROUPS;
    EXPORTING const QString XMLREPLACEWITH;
    EXPORTING const QString XMLBASE;
    EXPORTING const QString XMLHASHNAME;
    EXPORTING const QString XMLUPPERCASE;
    EXPORTING const QString XMLPROCESSLINEBYLINE;
    EXPORTING const QString XMLTYPE;
    EXPORTING const QString XMLLITTLEENDIAN;
    EXPORTING const QString XMLQUERYSTRING;
    EXPORTING const QString XMLLEVEL;
    EXPORTING const QString XMLFORMAT;
    EXPORTING const QString XMLOUTPUTTYPE;
    EXPORTING const QString XMLAPPENDTOINPUT;
    EXPORTING const QString XMLDECODE;
    EXPORTING const QString XMLOFFSET;
    EXPORTING const QString XMLCLEAN;
    EXPORTING const QString XMLAUTORELOAD;
    EXPORTING const QString XMLTZ;
    EXPORTING const QString XMLVALUE;

    EXPORTING const QString USER_DIRECTORY;
    EXPORTING const int BLOCK_MAX_SIZE;

    enum LOGLEVEL { PLSTATUS = 0, LWARNING = 1, LERROR = 2};
    struct Message {
            QString message;
            QString source;
            LOGLEVEL level;
    };

    enum OutputFormat {TEXTFORMAT = 0, HEXAFORMAT = 1};
    enum OutputType {ONELINE = 0, MULTILINES = 1};

    typedef QList<Message> Messages;
}

#endif // XMLCOMMON_H

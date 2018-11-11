/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/


#ifndef GUICONFIGURATION_H
#define GUICONFIGURATION_H

#include <QColor>
#include <QFont>
#include <QStringList>
#include <QFontDatabase>
#include <QHostAddress>
#include <QJsonDocument>
#include <QBitArray>

#define GEN_BLOCK_SIZE 4096

namespace GuiConst
{
    extern const char* MEM_ALLOC_ERROR_STR;
    extern const QString UNDEFINED_TEXT;
    extern const QString SETTINGS_SERVER_PORT;
    extern const QString SETTINGS_SERVER_IP;
    extern const QString SETTINGS_SERVER_PIPE_NAME;
    extern const QString SETTINGS_SERVER_DECODE;
    extern const QString SETTINGS_SERVER_ENCODE;
    extern const QString SETTINGS_SERVER_SEPARATOR;
    extern const QString SETTINGS_PIPE_NAME;
    extern const QString SETTINGS_QUICKVIEWS;
    extern const QString SETTINGS_FILTER_BLACKLIST;
    extern const QString SETTINGS_MARKINGS_COLORS;
    extern const QString SETTINGS_EXPORT_IMPORT_FUNC;
    extern const QString SETTINGS_OFFSET_BASE;
    extern const QString SETTINGS_GLOBAL_PROXY_IP;
    extern const QString SETTINGS_GLOBAL_PROXY_PORT;
    extern const QString SETTINGS_AUTO_COPY_TRANSFORM;
    extern const QString SETTINGS_AUTO_UPDATE;
    extern const QString SETTINGS_GENERAL_GROUP;
    extern const QString SETTINGS_MINIMIZE_TO_TRAY;
    extern const QString SETTINGS_MASS_PROCESSING_GROUP;
    extern const QString SETTINGS_DEFAULT_TAB;
    extern const QString SETTINGS_IGNORE_SSL_ERRORS;
    extern const QString SETTINGS_ENABLE_NETWORK_PROXY;
    extern const QString SETTINGS_DEFAULT_SAVELOAD_FLAGS;
    extern const QString SETTINGS_AUTO_SAVE_ENABLE;
    extern const QString SETTINGS_AUTO_SAVE_SINGLE_FILENAME;
    extern const QString SETTINGS_AUTO_SAVE_FILENAME;
    extern const QString SETTINGS_AUTO_SAVE_ON_EXIT;
    extern const QString SETTINGS_AUTO_SAVE_TIMER_ENABLE;
    extern const QString SETTINGS_AUTO_SAVE_TIMER_INTERVAL;
    extern const QString SETTINGS_AUTO_RESTORE_ON_STARTUP;
    extern const QString SETTINGS_REGULAR_FONT;
    extern const QString SETTINGS_FUZZING_EXPORT_MAX_SIZE;
    extern const QString SETTINGS_FUZZING_EXPORT_FORMAT;
    extern const QString SETTINGS_EQUALITY_PACKETS_BACKGROUND;
    extern const QString SETTINGS_EQUALITY_PACKETS_FOREGROUND;
    extern const QString SETTINGS_JSON_TYPE_COLOR;
    extern const QString SETTINGS_JSON_KEY_COLOR;
    extern const QString SETTINGS_JSON_VALUE_COLOR;
    extern const QString SETTINGS_SEARCH_BG_COLOR;
    extern const QString SETTINGS_SEARCH_FG_COLOR;

    extern const QString DEFAULT_STATE_FILE;
    extern const QString STATE_PIP3LINE_DOC;
    extern const QString STATE_MAIN_WINDOW;
    extern const QString STATE_DIALOGS_RUNNING;
    extern const QString STATE_YES;
    extern const QString STATE_NO;
    extern const QString STATE_WIDGET_GEOM;
    extern const QString STATE_WIDGET_STATE;
    extern const QString STATE_GLOBAL_CONF;
    extern const QString STATE_MAIN_WINDOW_STATE;
    extern const QString STATE_SETTINGS_DIALOG;
    extern const QString STATE_ANALYSE_DIALOG;
    extern const QString STATE_REGEXPHELP_DIALOG;
    extern const QString STATE_QUICKVIEW_DIALOG;
    extern const QString STATE_QUICKVIEW_CONF;
    extern const QString STATE_QUICKVIEW_ITEM_COUNT;
    extern const QString STATE_QUICKVIEW_ITEM;
    extern const QString STATE_CONF;
    extern const QString STATE_COMPARISON_DIALOG;
    extern const QString STATE_UI_CONF;
    extern const QString STATE_MARKING_COLOR;
    extern const QString STATE_DEBUG_DIALOG;
    extern const QString STATE_DIALOG_GEOM;
    extern const QString STATE_DIALOG_ISVISIBLE;
    extern const QString STATE_INPUT_CONTENT;
    extern const QString STATE_OUTPUT_CONTENT;
    extern const QString STATE_MAINTABS;
    extern const QString STATE_MAINTABS_LOGGER;
    extern const QString STATE_CURRENT_INDEX;
    extern const QString STATE_IS_FOLDED;
    extern const QString STATE_SCROLL_INDEX;
    extern const QString STATE_SYNTAX;
    extern const QString STATE_RANDOM_SOURCE_CURRENT_OFFSET;
    extern const QString STATE_CHUNK_SIZE;
    extern const QString STATE_LARGE_FILE_NAME;
    extern const QString STATE_FILE_NAME;
    extern const QString STATE_SEARCH_DATA;
    extern const QString STATE_GOTOOFFSET_DATA;
    extern const QString STATE_MESSAGE_PANEL;
    extern const QString STATE_MESSAGE_PANEL_VISIBLE;
    extern const QString STATE_SIZE;
    extern const QString STATE_TABS_ARRAY;
    extern const QString STATE_WINDOWED_TABS_ARRAY;
    extern const QString STATE_WINDOWED ;
    extern const QString STATE_TYPE;
    extern const QString STATE_ENCODING;
    extern const QString STATE_TRANSFORMGUI_ARRAY;
    extern const QString STATE_TAB_NAME;
    extern const QString STATE_DATA;
    extern const QString STATE_BYTESOURCE;
    extern const QString STATE_BLOCKSOURCE;
    extern const QString STATE_IP_ADDRESS;
    extern const QString STATE_PORT;
    extern const QString STATE_TAB;
    extern const QString STATE_TABVIEWLIST;
    extern const QString STATE_TABVIEW;
    extern const QString STATE_PRETAB_TYPE;
    extern const QString STATE_NAME;
    extern const QString STATE_READONLY;
    extern const QString STATE_MARKINGS;
    extern const QString STATE_USERMARKINGS;
    extern const QString STATE_MARKING;
    extern const QString STATE_HEX_HISTORY;
    extern const QString STATE_HEX_HISTORY_ITEM;
    extern const QString STATE_HEX_HISTORY_POINTER;
    extern const QString STATE_HEX_HISTORY_OFFSET;
    extern const QString STATE_HEX_HISTORY_ACTION;
    extern const QString STATE_HEX_HISTORY_BEFORE;
    extern const QString STATE_HEX_HISTORY_AFTER;
    extern const QString STATE_BYTE_DESC;
    extern const QString STATE_BYTE_LOWER_VAL;
    extern const QString STATE_BYTE_UPPER_VAL;
    extern const QString STATE_BYTE_FG_COLOR;
    extern const QString STATE_BYTE_BG_COLOR;
    extern const QString STATE_TRANSFORM_CONF;
    extern const QString STATE_CAPABILITIES;
    extern const quint64 STATE_SAVE_REQUEST;
    extern const quint64 STATE_LOADSAVE_DIALOG_POS;
    extern const quint64 STATE_LOADSAVE_DATA;
    extern const quint64 STATE_LOADSAVE_SEARCH_RESULTS;
    extern const quint64 STATE_LOADSAVE_QUICKVIEW_CONF;
    extern const quint64 STATE_LOADSAVE_HISTORY;
    extern const quint64 STATE_LOADSAVE_MARKINGS;
    extern const quint64 STATE_LOADSAVE_COMPARISON;
    extern const quint64 STATE_LOADSAVE_GLOBAL_CONF;
    extern const quint64 STATE_LOADSAVE_LOAD_ALL;
    extern const quint64 STATE_LOADSAVE_SAVE_ALL;
    extern const QString STATE_LOGGER_INDEX;
    extern const QString STATE_STATIC_MARKINGS;
    extern const QString STATE_TIMESTAMP;
    extern const QString STATE_MICROSEC;
    extern const QString STATE_ORIGINAL_DATA;
    extern const QString STATE_DIRECTION;
    extern const QString STATE_COMMENT;
    extern const QString STATE_SOURCE_STRING;
    extern const QString STATE_FOREGROUNG_COLOR;
    extern const QString STATE_BACKGROUNG_COLOR;
    extern const QString STATE_ADDITIONAL_FIELDS_LIST;
    extern const QString STATE_ADDITIONAL_FIELD;
    extern const QString STATE_COLUMNS_CONF;
    extern const QString STATE_COLUMN;
    extern const QString STATE_MODEL_INDEX;
    extern const QString STATE_IS_HIDDEN;
    extern const QString STATE_VIEW_INDEX;
    extern const QString STATE_WIDTH;
    extern const QString STATE_PACKET_LIST;
    extern const QString STATE_PACKET;
    extern const QString STATE_ORCHESTRATOR;
    extern const QString STATE_FLAGS;
    extern const QString STATE_INBOUNDTRANSFORM;
    extern const QString STATE_OUTBOUNDTRANSFORM;
    extern const QString STATE_INJECTED_PACKET;
    extern const QString STATE_TRACK_PACKETS;
    extern const QString STATE_TRACK_CHANGES;
    extern const QString STATE_AUTO_MERGE;
    extern const QString STATE_SOURCEID;
    extern const QString STATE_MAX_PAYLOAD_DISPLAY_SIZE;
    extern const QString STATE_PACKET_PROXY_FILTERS_ENABLE;
    extern const QString STATE_PACKET_PROXY_FILTERS_LIST;
    extern const QString STATE_PACKET_PROXY_EQUALITY_ENABLE;
    extern const QString STATE_PACKET_PROXY_EQUALITY_TARGET_COLUMNS;
    extern const QString STATE_PACKET_PROXY_SORTING_TARGET_COLUMN;
    extern const QString STATE_PACKET_PROXY_SORTING_ORDER;
    extern const QString STATE_FIELD_SEPARATOR;
    extern const QString STATE_FILTER_SORT;
    extern const QString STATE_FILTER_ITEMS;
    extern const QString STATE_FILTER_ITEM;
    extern const QString STATE_FILTER_ID;
    extern const QString STATE_FILTER_EXPR;
    extern const QString STATE_OFFSET;
    extern const QString STATE_REVERSE_SELECTION;
    extern const QString STATE_MASK;
    extern const QString STATE_AFTER;
    extern const QString STATE_BEFORE;
    extern const QString STATE_TIMEZONE;
    extern const QString STATE_REGEXP_PATTERN;
    extern const QString STATE_REGEXP_CASE_SENSITIVE;
    extern const QString STATE_REGEXP_PATTERN_SYNTAX;
    extern const QString STATE_CID_LIST;
    extern const QString STATE_SERVER_TYPE;
    extern const QString STATE_CLIENT_TYPE;
    extern const QString STATE_KEY;
    extern const QString STATE_SHMID;
    extern const QString STATE_JSON_STATE;

    extern const bool DEFAULT_AUTO_SAVE_ENABLED;
    extern const bool DEFAULT_AUTO_RESTORE_ENABLED;
    extern const bool DEFAULT_AUTO_SAVE_ON_EXIT;
    extern const bool DEFAULT_AUTO_SAVE_TIMER_ENABLE;
    extern const int DEFAULT_AUTO_SAVE_TIMER_INTERVAL;
    extern const int MAX_AUTO_SAVE_TIMER_INTERVAL;
    extern const int MIN_AUTO_SAVE_TIMER_INTERVAL;

    extern const QString STATE_ACTION_RESTORE_STR;
    extern const QString STATE_ACTION_SAVE_STR;

    extern const QString TCP_SERVER;

    extern const QString UPDATE_URL;
    extern const QString RELEASES_URL;
    extern const QString NO_TRANSFORM;
    extern const QString MODIFIED_STR;

    extern const int DEFAULT_PORT;
    extern const char DEFAULT_BLOCK_SEPARATOR;
    extern const bool DEFAULT_SERVER_ENCODE;
    extern const bool DEFAULT_SERVER_DECODE;

    extern const QString LOGS_STR;
    extern const QString CHOOSE_STR;
    extern const QString START_STR;
    extern const QString STOP_STR;
    extern const QString TEXT_TEXT;
    extern const QString JSON_TEXT;
    extern const QString ORIGINAL_DATA_STR;

    extern const QString DEFAULT_PIPE_MASS ;
    extern const QString DEFAULT_PIPE;
    extern const QString PIPE_SERVER;
    extern const QString PIPE_SOCKET;

    extern const bool DEFAULT_AUTO_COPY_TEXT;
    extern const bool DEFAULT_IGNORE_SSL;
    extern const bool DEFAULT_PROXY_ENABLE;
    extern const QString DEFAULT_GLOBAL_PROXY_IP;
    extern const quint16 DEFAULT_GLOBAL_PROXY_PORT;
    extern const int DEFAULT_OFFSET_BASE;

    extern const QString UTF8_STRING_ACTION;
    extern const QString NEW_BYTE_ACTION;
    extern const QString SEND_TO_NEW_TAB_ACTION;

    extern const QString LITTLE_ENDIAN_STRING;
    extern const QString BIG_ENDIAN_STRING;
    extern const QString OCTAL_STRING;
    extern const QString DECIMAL_STRING;
    extern const QString HEXADECIMAL_STRING;
    extern const QString ABSOLUTE_LITTLE_ENDIAN_STRING;
    extern const QString ABSOLUTE_BIG_ENDIAN_STRING;
    extern const QString RELATIVE_LITTLE_ENDIAN_STRING;
    extern const QString RELATIVE_BIG_ENDIAN_STRING;

    extern const uint DEFAULT_MAX_TAB_COUNT;

    extern const int MAX_DELETED_TABS_KEPT;

    extern const quint16 DEFAULT_INCOMING_SERVER_PORT;
    extern const QHostAddress DEFAULT_INCOMING_SERVER_ADDRESS;

    enum TAB_TYPES {
        TRANSFORM_TAB_TYPE = 0,  // default
        GENERIC_TAB_TYPE = 1,
        RANDOM_ACCESS_TAB_TYPE = 2
    };

    enum AVAILABLE_PRETABS {
        INVALID_PRETAB = -1,
        TRANSFORM_PRETAB = 0,
        LARGE_FILE_PRETAB = 1,
        CURRENTMEM_PRETAB = 2,
        HEXAEDITOR_PRETAB = 3,
        PACKET_ANALYSER_PRETAB = 5
    };

    enum FileOperations {IMPORT_OPERATION, EXPORT_OPERATION, UNKNOWN_OPERATION};
    enum FileFormat {PCAP_FORMAT, JSON_FORMAT, XML_FORMAT, PLAIN_FORMAT, INVALID_FORMAT};

    extern const QString TRANSFORM_TAB_STRING;
    extern const QString LARGE_FILE_TAB_STRING;
    extern const QString CURRENTMEM_TAB_STRING;
    extern const QString BASEHEX_TAB_STRING;
    extern const QString INTERCEP_TAB_STRING;
    extern const QString PACKET_ANALYSER_TAB_STRING;
    extern const QString NEW_TAB_STRING;
    extern const QStringList AVAILABLE_TAB_STRINGS;

    extern const QString COPYOFFSET;
    extern const QString SAVELISTTOFILE;
    extern const QString COPYLIST;

    extern const QString PKCS12_EXT_STRING;
    extern const QString SSL_PEM_EXT_STRING;
    extern const QString SSL_DER_EXT_STRING;

    extern const int DEFAULT_UDP_TIMEOUT_MS;

    extern const quint64 DEFAULT_FUZZING_EXPORT_MAX_SIZE;
    extern const QJsonDocument::JsonFormat DEFAULT_FUZZING_EXPORT_FORMAT;

    extern const QString HEXCHAR;
    extern const QString WILDCARD_HEXCHAR;
    QByteArray extractSearchHexa(QString searchRequest, QBitArray &mask);
    extern QString GLOBAL_LAST_PATH;
    QString convertSizetoBytesString(quint64 size);
    int calculateStringWidthWithGlobalFont(QString value);

    extern const int DEFAULT_HEXVIEW_TEXTCOLUMN_WIDTH;
    extern const int DEFAULT_HEXVIEW_HEXCOLUMN_WIDTH;
    extern const int DEFAULT_HEXVIEW_ROWS_HEIGHT;
}

namespace GlobalsValues {
    extern int TEXTCOLUMNWIDTH;
    extern int HEXCOLUMNWIDTH;
    extern int ROWSHEIGHT;
    extern QFont GLOBAL_REGULAR_FONT;
    extern QColor EqualsPacketsBackground;
    extern QColor EqualsPacketsForeground;
    extern QColor JSON_KEY_COLOR;
    extern QColor JSON_VALUE_COLOR;
    extern QColor JSON_TYPE_COLOR;
    extern QColor MARKINGS_FG_COLOR;
    extern QColor SEARCH_BG_COLOR;
    extern QColor SEARCH_FG_COLOR;
}

namespace GuiStyles {
    extern const QString LineEditError;
    extern const QString LineEditOk;
    extern const QString ComboBoxError;
    extern const QString PushButtonReadonly;
    extern const QString LineEditWarning;
    extern const QString LineEditMessage;
    extern const QColor DEFAULT_EQUAL_PACKETS_BACKGROUND;
    extern const QColor DEFAULT_EQUAL_PACKETS_FOREGROUND;
    extern const QColor DEFAULT_MARKING_COLOR;
    extern const QColor DEFAULT_MARKING_COLOR_DATA;
    extern const QColor DEFAULT_MARKING_COLOR_SIZE;
    extern const QColor DEFAULT_MARKING_COLOR_TYPE;
    extern const QFont DEFAULT_REGULAR_FONT;
    extern const QColor DEFAULT_JSON_KEY_COLOR;
    extern const QColor DEFAULT_JSON_VALUE_COLOR;
    extern const QColor DEFAULT_JSON_TYPE_COLOR;
    extern const QColor DEFAULT_MARKING_FG_COLOR;
    extern const QColor DEFAULT_SEARCH_BG_COLOR;
    extern const QColor DEFAULT_SEARCH_FG_COLOR;
}

#endif // GUICONST_H

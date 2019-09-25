#include "guiconst.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFontMetrics>

namespace GuiConst
{
    const char* MEM_ALLOC_ERROR_STR = "Cannot allocate memory";
    const QString UNDEFINED_TEXT = "Undefined";
    const QString SETTINGS_SERVER_PORT = "DefaultServerPort";
    const QString SETTINGS_SERVER_IP = "DefaultServerIP";
    const QString SETTINGS_SERVER_PIPE_NAME = "DefaultServerPipe";
    const QString SETTINGS_SERVER_DECODE = "DefaultServerDecode";
    const QString SETTINGS_SERVER_ENCODE = "DefaultServerEncode";
    const QString SETTINGS_SERVER_SEPARATOR = "DefaultServerSeparator";
    const QString SETTINGS_PIPE_NAME="PipeName";
    const QString SETTINGS_QUICKVIEWS = "QuickView";
    const QString SETTINGS_FILTER_BLACKLIST = "FilterBlacklist";
    const QString SETTINGS_MARKINGS_COLORS = "MarkingsColors";
    const QString SETTINGS_EXPORT_IMPORT_FUNC = "ExportImportFunctions";
    const QString SETTINGS_OFFSET_BASE = "OffsetBase";
    const QString SETTINGS_GLOBAL_PROXY_IP = "GlobalProxyIP";
    const QString SETTINGS_GLOBAL_PROXY_PORT = "GlobalProxyPort";
    const QString SETTINGS_AUTO_COPY_TRANSFORM = "AutoCopyTransform";
    const QString SETTINGS_AUTO_UPDATE = "AutoUpdate";
    const QString SETTINGS_GENERAL_GROUP = "General";
    const QString SETTINGS_MINIMIZE_TO_TRAY = "MinimizeToTray";
    const QString SETTINGS_MASS_PROCESSING_GROUP = "MassProcessing";
    const QString SETTINGS_DEFAULT_TAB = "DefaultTab";
    const QString SETTINGS_IGNORE_SSL_ERRORS = "IgnoreSSLErrors";
    const QString SETTINGS_ENABLE_NETWORK_PROXY = "EnableNetworkProxy";
    const QString SETTINGS_DEFAULT_SAVELOAD_FLAGS = "DefaultSaveLoadFlags";
    const QString SETTINGS_AUTO_SAVE_ENABLE = "AutoSaveEnabled";
    const QString SETTINGS_AUTO_SAVE_SINGLE_FILENAME = "AutoSaveSingleFileName";
    const QString SETTINGS_AUTO_SAVE_FILENAME = "autostate.sav";
    const QString SETTINGS_AUTO_SAVE_ON_EXIT = "AutoSaveOnExit";
    const QString SETTINGS_AUTO_SAVE_TIMER_ENABLE = "AutoSaveTimerEnable";
    const QString SETTINGS_AUTO_SAVE_TIMER_INTERVAL = "AutoSaveTimerInterval";
    const QString SETTINGS_AUTO_RESTORE_ON_STARTUP = "AutoRestoreOnStartup";
    const QString SETTINGS_REGULAR_FONT = "RegularFont";
    const QString SETTINGS_FUZZING_EXPORT_MAX_SIZE = "FuzzingExportMaxSize";
    const QString SETTINGS_FUZZING_EXPORT_FORMAT = "FuzzingExportFormat";
    const QString SETTINGS_EQUALITY_PACKETS_BACKGROUND = "EqualityPacketBackground";
    const QString SETTINGS_EQUALITY_PACKETS_FOREGROUND = "EqualityPacketForeground";
    const QString SETTINGS_JSON_TYPE_COLOR = "JsonTypeColor";
    const QString SETTINGS_JSON_KEY_COLOR = "JsonKeyColor";
    const QString SETTINGS_JSON_VALUE_COLOR = "JsonValueColor";
    const QString SETTINGS_SEARCH_BG_COLOR = "SearchBgColor";
    const QString SETTINGS_SEARCH_FG_COLOR = "SearchFgColor";
    const QString DEFAULT_STATE_FILE = "savedstate.conf";
    const QString STATE_PIP3LINE_DOC = "Pip3lineState";
    const QString STATE_MAIN_WINDOW = "MainWindow";
    const QString STATE_DIALOGS_RUNNING = "DialogsRunning";
    const QString STATE_YES = "Yes";
    const QString STATE_NO = "No";
    const QString STATE_WIDGET_GEOM = "Geometry";
    const QString STATE_WIDGET_STATE = "State";
    const QString STATE_GLOBAL_CONF = "GlobalConf";
    const QString STATE_MAIN_WINDOW_STATE = "MainWindowState";
    const QString STATE_SETTINGS_DIALOG = "SettingsDialog";
    const QString STATE_ANALYSE_DIALOG = "AnalyseDialog";
    const QString STATE_REGEXPHELP_DIALOG = "RegExpHelpDialog";
    const QString STATE_QUICKVIEW_DIALOG = "QuickView";
    const QString STATE_QUICKVIEW_CONF = "QuickViewConf";
    const QString STATE_QUICKVIEW_ITEM_COUNT = "QuickViewItemCount";
    const QString STATE_QUICKVIEW_ITEM = "QuickViewItem";
    const QString STATE_CONF = "Conf";
    const QString STATE_COMPARISON_DIALOG = "ComparisonDialog";
    const QString STATE_UI_CONF = "UIConf";
    const QString STATE_MARKING_COLOR = "MarkingColor";
    const QString STATE_DEBUG_DIALOG = "DebugDialog";
    const QString STATE_DIALOG_GEOM = "DialogGeom";
    const QString STATE_DIALOG_ISVISIBLE = "IsVisible";
    const QString STATE_INPUT_CONTENT = "InputContent";
    const QString STATE_OUTPUT_CONTENT = "OutputContent";
    const QString STATE_MAINTABS = "MainTabs";
    const QString STATE_MAINTABS_LOGGER = "MainTabsLogger";
    const QString STATE_CURRENT_INDEX = "CurrentIndex";
    const QString STATE_IS_FOLDED = "IsFolded";
    const QString STATE_SCROLL_INDEX = "ScrollIndex";
    const QString STATE_SYNTAX = "Syntax";
    const QString STATE_RANDOM_SOURCE_CURRENT_OFFSET = "CurrentStartingOffset";
    const QString STATE_CHUNK_SIZE = "ChunkSize";
    const QString STATE_LARGE_FILE_NAME = "LargeFileName";
    const QString STATE_FILE_NAME = "FileName";
    const QString STATE_SEARCH_DATA = "SearchData";
    const QString STATE_GOTOOFFSET_DATA = "GotoOffsetData";
    const QString STATE_MESSAGE_PANEL = "Messages";
    const QString STATE_MESSAGE_PANEL_VISIBLE = "MessPanelVisible";
    const QString STATE_SIZE = "Size";
    const QString STATE_TABS_ARRAY = "Tabs";
    const QString STATE_WINDOWED_TABS_ARRAY = "WindowTabs";
    const QString STATE_WINDOWED = "Windowed";
    const QString STATE_TYPE = "Type";
    const QString STATE_ENCODING = "Encoding";
    const QString STATE_TRANSFORMGUI_ARRAY = "TransformGuiList";
    const QString STATE_TAB_NAME = "TabName";
    const QString STATE_DATA = "Data";
    const QString STATE_BYTESOURCE = "ByteSource";
    const QString STATE_BLOCKSOURCE = "BlockSource";
    const QString STATE_IP_ADDRESS = "IP";
    const QString STATE_PORT = "Port";
    const QString STATE_TAB = "Tab";
    const QString STATE_TABVIEWLIST = "TabViewList";
    const QString STATE_TABVIEW = "TabView";
    const QString STATE_PRETAB_TYPE = "PreTab";
    const QString STATE_NAME = "Name";
    const QString STATE_READONLY = "Readonly";
    const QString STATE_MARKINGS = "Markings";
    const QString STATE_USERMARKINGS = "UserMarkings";
    const QString STATE_MARKING = "Marking";
    const QString STATE_HEX_HISTORY = "HexHistory";
    const QString STATE_HEX_HISTORY_ITEM = "HexHistoryItem";
    const QString STATE_HEX_HISTORY_POINTER = "HexHistoryPointer";
    const QString STATE_HEX_HISTORY_OFFSET = "HexHistoryOffset";
    const QString STATE_HEX_HISTORY_ACTION = "HexHistoryAction";
    const QString STATE_HEX_HISTORY_BEFORE = "HexHistoryBefore";
    const QString STATE_HEX_HISTORY_AFTER = "HexHistoryAfter";
    const QString STATE_BYTE_DESC = "ByteRangeDesc";
    const QString STATE_BYTE_LOWER_VAL = "ByteRangeLowerVal";
    const QString STATE_BYTE_UPPER_VAL = "ByteRangeUpperVal";
    const QString STATE_BYTE_FG_COLOR = "ByteRangeForegroundColor";
    const QString STATE_BYTE_BG_COLOR = "ByteRangeBackgroundColor";
    const QString STATE_TRANSFORM_CONF = "TransformConf";
    const QString STATE_CAPABILITIES = "Caps";
    const quint64 STATE_SAVE_REQUEST = 0x1;
    const quint64 STATE_LOADSAVE_DIALOG_POS = 0x2;
    const quint64 STATE_LOADSAVE_DATA = 0x4;
    const quint64 STATE_LOADSAVE_SEARCH_RESULTS = 0x8;
    const quint64 STATE_LOADSAVE_QUICKVIEW_CONF = 0x10;
    const quint64 STATE_LOADSAVE_HISTORY = 0x20;
    const quint64 STATE_LOADSAVE_MARKINGS = 0x40;
    const quint64 STATE_LOADSAVE_COMPARISON = 0x80;
    const quint64 STATE_LOADSAVE_GLOBAL_CONF = 0x100;
    const quint64 STATE_LOADSAVE_LOAD_ALL = 0xFFFFFFFFFFFFFFFE;
    const quint64 STATE_LOADSAVE_SAVE_ALL = 0xFFFFFFFFFFFFFFFF;
    const QString STATE_LOGGER_INDEX = "LoggerTabIndex";
    const QString STATE_STATIC_MARKINGS = "StaticMarkings";
    const QString STATE_TIMESTAMP = "TimeStamp";
    const QString STATE_MICROSEC = "Microseconds";
    const QString STATE_ORIGINAL_DATA = "OriginalData";
    const QString STATE_DIRECTION = "Direction";
    const QString STATE_COMMENT = "Comment";
    const QString STATE_SOURCE_STRING = "SourceDescription";
    const QString STATE_FOREGROUNG_COLOR = "BackgroundColor";
    const QString STATE_BACKGROUNG_COLOR = "ForegroundColor";
    const QString STATE_ADDITIONAL_FIELDS_LIST = "AdditionalFields";
    const QString STATE_ADDITIONAL_FIELD = "AdditionalField";
    const QString STATE_COLUMNS_CONF = "ColumnsConf";
    const QString STATE_COLUMN = "Column";
    const QString STATE_MODEL_INDEX = "ModelIndex";
    const QString STATE_IS_HIDDEN = "IsHidden";
    const QString STATE_VIEW_INDEX = "ViewIndex";
    const QString STATE_WIDTH = "Width";
    const QString STATE_PACKET_LIST = "PacketsList";
    const QString STATE_PACKET = "Packet";
    const QString STATE_ORCHESTRATOR = "Orchestrator";
    const QString STATE_FLAGS = "Flags";
    const QString STATE_INBOUNDTRANSFORM = "InboundTransform";
    const QString STATE_OUTBOUNDTRANSFORM = "OutboundTransform";
    const QString STATE_INJECTED_PACKET = "Injected";
    const QString STATE_TRACK_PACKETS = "TrackLast";
    const QString STATE_TRACK_CHANGES = "TrackChanges";
    const QString STATE_AUTO_MERGE = "AutoMerge";
    const QString STATE_SOURCEID = "SourceID";
    const QString STATE_MAX_PAYLOAD_DISPLAY_SIZE = "MaxPayloadDisplaySize";
    const QString STATE_PACKET_PROXY_FILTERS_ENABLE = "FiltersEnabled";
    const QString STATE_PACKET_PROXY_FILTERS_LIST = "FiltersList";
    const QString STATE_PACKET_PROXY_EQUALITY_ENABLE = "EqualityEnabled";
    const QString STATE_PACKET_PROXY_EQUALITY_TARGET_COLUMNS = "EqualityColumns";
    const QString STATE_PACKET_PROXY_SORTING_TARGET_COLUMN = "SortingColumn";
    const QString STATE_PACKET_PROXY_SORTING_ORDER = "SortingOrder";
    const QString STATE_FIELD_SEPARATOR = ":";
    const QString STATE_FILTER_SORT = "FilterSortInfo";
    const QString STATE_FILTER_ITEMS = "FilterItems";
    const QString STATE_FILTER_ITEM = "FilterItem";
    const QString STATE_FILTER_EXPR = "FilterExpr";
    const QString STATE_FILTER_ID = "FitlerId";
    const QString STATE_OFFSET = "Offset";
    const QString STATE_REVERSE_SELECTION = "ReverseSelection";
    const QString STATE_MASK = "Mask";
    const QString STATE_AFTER = "After";
    const QString STATE_BEFORE = "Before";
    const QString STATE_TIMEZONE = "TimeZone";
    const QString STATE_REGEXP_PATTERN = "RegExpPattern";
    const QString STATE_REGEXP_CASE_SENSITIVE = "RegExpCaseSensitive";
    const QString STATE_REGEXP_PATTERN_SYNTAX = "RegExpPatternSyntax";
    const QString STATE_CID_LIST = "CIDsList";
    const QString STATE_SERVER_TYPE = "ServerType";
    const QString STATE_CLIENT_TYPE = "ClientType";
    const QString STATE_KEY = "Key";
    const QString STATE_SHMID = "Shmid";
    const QString STATE_JSON_STATE = "JsonState";
    const QString STATE_DELAY = "Delay";

    const bool DEFAULT_AUTO_SAVE_ENABLED = true;
    const bool DEFAULT_AUTO_RESTORE_ENABLED = true;
    const bool DEFAULT_AUTO_SAVE_ON_EXIT = true;
    const bool DEFAULT_AUTO_SAVE_TIMER_ENABLE = true;
    const int DEFAULT_AUTO_SAVE_TIMER_INTERVAL = 5;
    const int MAX_AUTO_SAVE_TIMER_INTERVAL = 120;
    const int MIN_AUTO_SAVE_TIMER_INTERVAL = 1;

    const QString STATE_ACTION_RESTORE_STR = "restoring";
    const QString STATE_ACTION_SAVE_STR = "saving";

    const QString TCP_SERVER = QObject::tr("Tcp server");

    const QString UPDATE_URL = "https://raw.githubusercontent.com/metrodango/pip3line/master/gui/release.txt";
    const QString RELEASES_URL = "https://github.com/metrodango/pip3line/releases";
    const QString NO_TRANSFORM = "No transform";
    const QString MODIFIED_STR = "Modified";

    const int DEFAULT_PORT = 45632;
    const char DEFAULT_BLOCK_SEPARATOR = '\n';
    const bool DEFAULT_SERVER_ENCODE = false;
    const bool DEFAULT_SERVER_DECODE = false;

    const QString LOGS_STR = QObject::tr("Logs");
    const QString CHOOSE_STR = QObject::tr("Choose");
    const QString START_STR = QObject::tr("Start");
    const QString STOP_STR = QObject::tr("Stop");
    const QString TEXT_TEXT = QObject::tr("Text");
    const QString JSON_TEXT = QObject::tr("Json");
    const QString ORIGINAL_DATA_STR = QObject::tr("Original");

#if defined(Q_OS_WIN32)
    const QString DEFAULT_PIPE_MASS = "pip3lineMass";
    const QString DEFAULT_PIPE = "pip3line";
    const QString PIPE_SERVER = QObject::tr("Named Pipe Server");
    const QString PIPE_SOCKET = QObject::tr("Named Pipe");
#else
    const QString DEFAULT_PIPE_MASS = "/tmp/pip3lineMass";
    const QString DEFAULT_PIPE = "/tmp/pip3line";
    const QString PIPE_SERVER = QObject::tr("Unix Local Socket Server");
    const QString PIPE_SOCKET = QObject::tr("Unix Local Socket");
#endif

    const bool DEFAULT_AUTO_COPY_TEXT = false;
    const bool DEFAULT_IGNORE_SSL = true;
    const bool DEFAULT_PROXY_ENABLE = false;
    const QString DEFAULT_GLOBAL_PROXY_IP = "127.0.0.1";
    const quint16 DEFAULT_GLOBAL_PROXY_PORT = 8080;
    const int DEFAULT_OFFSET_BASE = 16;

    const QString UTF8_STRING_ACTION = QObject::tr("UTF-8");
    const QString NEW_BYTE_ACTION = QObject::tr("New Byte(s)");
    const QString SEND_TO_NEW_TAB_ACTION = QObject::tr("New tab");

    const QString LITTLE_ENDIAN_STRING = QObject::tr("Little endian");
    const QString BIG_ENDIAN_STRING = QObject::tr("Big endian");
    const QString OCTAL_STRING = QObject::tr("Octal");
    const QString DECIMAL_STRING = QObject::tr("Decimal");
    const QString HEXADECIMAL_STRING = QObject::tr("Hexadecimal");
    const QString ABSOLUTE_LITTLE_ENDIAN_STRING = QObject::tr("Absolute LE");
    const QString ABSOLUTE_BIG_ENDIAN_STRING = QObject::tr("Absolute BE");
    const QString RELATIVE_LITTLE_ENDIAN_STRING = QObject::tr("Relative LE");
    const QString RELATIVE_BIG_ENDIAN_STRING = QObject::tr("Relative BE");

    const uint DEFAULT_MAX_TAB_COUNT = 50;

    const int MAX_DELETED_TABS_KEPT = 30;

    const QString TRANSFORM_TAB_STRING = QObject::tr("Transform Editor");
    const QString LARGE_FILE_TAB_STRING = QObject::tr("Large File Editor");
    const QString CURRENTMEM_TAB_STRING = QObject::tr("Current memory");
    const QString BASEHEX_TAB_STRING = QObject::tr("Hexeditor");
    const QString INTERCEP_TAB_STRING = QObject::tr("Interceptor");
    const QString PACKET_ANALYSER_TAB_STRING = QObject::tr("Packet analyser");
    const QString NEW_TAB_STRING = QObject::tr("New tab");
    const QStringList AVAILABLE_TAB_STRINGS = QStringList() << TRANSFORM_TAB_STRING
                                                             << LARGE_FILE_TAB_STRING
                                                             << CURRENTMEM_TAB_STRING
                                                             << BASEHEX_TAB_STRING
                                                             << INTERCEP_TAB_STRING
                                                             << PACKET_ANALYSER_TAB_STRING;

    const QString COPYOFFSET = QObject::tr("Copy Offset as");
    const QString SAVELISTTOFILE = QObject::tr("Save List to file as");
    const QString COPYLIST = QObject::tr("Copy list as");

    const QString PKCS12_EXT_STRING = "p12";
    const QString SSL_PEM_EXT_STRING = "pem";
    const QString SSL_DER_EXT_STRING = "der";

    const int DEFAULT_UDP_TIMEOUT_MS = 60000;

    const quint16 DEFAULT_INCOMING_SERVER_PORT = 40000;
    const QHostAddress DEFAULT_INCOMING_SERVER_ADDRESS = QHostAddress::LocalHost;

    const quint64 DEFAULT_FUZZING_EXPORT_MAX_SIZE = 10 * 1024 * 1024; // 10 MB way more than enough
    const QJsonDocument::JsonFormat DEFAULT_FUZZING_EXPORT_FORMAT = QJsonDocument::Indented;

    const QString HEXCHAR = "abcdef1234567890ABCDEF";
    const QString WILDCARD_HEXCHAR = "*xX";

    QByteArray extractSearchHexa(QString searchRequest, QBitArray &mask)
    {
        QByteArray searchItem;
        mask.resize(searchRequest.size()/2);
        int i = 0;
        while (i < searchRequest.size() -1 ) { // this is fine, we know that searchRequest.size() > 0
                if (HEXCHAR.contains(searchRequest.at(i))
                        && HEXCHAR.contains(searchRequest.at(i+1))) { // check if valid hexa value
                    mask.setBit(i/2,true);
                    searchItem.append(
                                QByteArray::fromHex(
                                    QString(searchRequest.at(i)).append(searchRequest.at(i+1)).toUtf8()
                                    )
                                );
                    i += 2;
                    continue;
                } else if (WILDCARD_HEXCHAR.contains(searchRequest.at(i))
                           && WILDCARD_HEXCHAR.contains(searchRequest.at(i+1))){ // check if this is a mask character
                    searchItem.append('\0');
                    mask.setBit(i/2, false);
                    i += 2;
                    continue;
                } // otherwise discard the first character
            i++;
        }
        return searchItem;
    }

    QString GLOBAL_LAST_PATH;

    QString convertSizetoBytesString(quint64 size)
    {
        QString ret;
        if (size < 1000)
            ret = QString("%1 B").arg(size);
        else if (size < 1000000)
            ret = QString("%1 KiB").arg(static_cast<double>(size)/static_cast<double>(1000),0,'f',2);
        else if (size < 1000000000)
            ret = QString("%1 MiB").arg(static_cast<double>(size)/static_cast<double>(1000000),0,'f',2);

        return ret;
    }


    const int DEFAULT_HEXVIEW_TEXTCOLUMN_WIDTH = 131; // arbitratry number, dont' ask
    const int DEFAULT_HEXVIEW_HEXCOLUMN_WIDTH = 28; // arbitratry number, dont' ask
    const int DEFAULT_HEXVIEW_ROWS_HEIGHT = 20;

    int calculateStringWidthWithGlobalFont(QString value)
    {
        QFontMetrics fm(GlobalsValues::GLOBAL_REGULAR_FONT);
//        qDebug() << QObject::tr("width") << fm.width(value);
        return fm.width(value);
    }
}

namespace GuiStyles {
     const QString LineEditError = "QLineEdit { background-color: #FFB1B2; }";
     const QString LineEditOk = "QLineEdit { background-color: #b3e7ff; }";
     const QString ComboBoxError = "QComboBox { color : red; }";
     const QString PushButtonReadonly = "QPushButton { color : #FF0000; }";
     const QString LineEditWarning = "";
     const QString LineEditMessage = "";
     const QColor DEFAULT_EQUAL_PACKETS_BACKGROUND = QColor(255,255,128);
     const QColor DEFAULT_EQUAL_PACKETS_FOREGROUND = QColor(0,0,0);
     const QColor DEFAULT_MARKING_COLOR = QColor(255,182,117);
     const QColor DEFAULT_MARKING_COLOR_DATA = QColor(11449599);
     const QColor DEFAULT_MARKING_COLOR_SIZE = QColor(16755616);
     const QColor DEFAULT_MARKING_COLOR_TYPE = QColor(9043881);
#if QT_VERSION >= 0x050200
#if defined(Q_OS_LINUX)
     const QFont DEFAULT_REGULAR_FONT = QFont("Courier",10);
#elif defined(Q_OS_OSX)
     const QFont DEFAULT_REGULAR_FONT = QFont("Menlo",10);
#else
     const QFont DEFAULT_REGULAR_FONT = QFont("Courier New",10);
#endif
#else
     const QFont DEFAULT_REGULAR_FONT = QFont("Courier New",10);
#endif
     const QColor DEFAULT_JSON_KEY_COLOR = QColor(236,164,18);
     const QColor DEFAULT_JSON_VALUE_COLOR = QColor(5, 184, 187);
     const QColor DEFAULT_JSON_TYPE_COLOR = QColor(169, 0, 236);
     const QColor DEFAULT_MARKING_FG_COLOR = Qt::black;
     const QColor DEFAULT_SEARCH_BG_COLOR = QColor(211, 234, 0);
     const QColor DEFAULT_SEARCH_FG_COLOR = Qt::black;
}

namespace GlobalsValues {
    int TEXTCOLUMNWIDTH = GuiConst::DEFAULT_HEXVIEW_TEXTCOLUMN_WIDTH;
    int HEXCOLUMNWIDTH = GuiConst::DEFAULT_HEXVIEW_HEXCOLUMN_WIDTH;
    int ROWSHEIGHT = GuiConst::DEFAULT_HEXVIEW_ROWS_HEIGHT;
    QFont GLOBAL_REGULAR_FONT;
    QColor EqualsPacketsBackground = GuiStyles::DEFAULT_EQUAL_PACKETS_BACKGROUND;
    QColor EqualsPacketsForeground = GuiStyles::DEFAULT_EQUAL_PACKETS_FOREGROUND;
    QColor JSON_KEY_COLOR = GuiStyles::DEFAULT_JSON_KEY_COLOR;
    QColor JSON_VALUE_COLOR = GuiStyles::DEFAULT_JSON_VALUE_COLOR;
    QColor JSON_TYPE_COLOR = GuiStyles::DEFAULT_JSON_TYPE_COLOR;
    QColor MARKINGS_FG_COLOR = GuiStyles::DEFAULT_MARKING_FG_COLOR;
    QColor SEARCH_BG_COLOR = GuiStyles::DEFAULT_SEARCH_BG_COLOR;
    QColor SEARCH_FG_COLOR = GuiStyles::DEFAULT_SEARCH_FG_COLOR;
}

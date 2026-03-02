QT += gui qml quick widgets xml concurrent network quickcontrols2 multimedia printsupport svg charts pdf webenginequick core5compat

DESTDIR = $$PWD/../../binary
TARGET = Scrite

CONFIG += c++17

VERSION = 2.0.21
DEFINES += SCRITE_VERSION=\\\"$$VERSION\\\"
# DEFINES += SCRITE_VERSION_TYPE=\\\"beta\\\"
DEFINES += SCRITE_VERSION_TYPE=\\\"\\\"

#QT += testlib

CONFIG += qmltypes
QML_IMPORT_NAME = io.scrite.components
QML_IMPORT_MAJOR_VERSION = 1

DEFINES += SCRITE_QML_URI=\\\"$$QML_IMPORT_NAME\\\"

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
    CONFIG += qtquickcompiler
}

exists($$DESTDIR/../../profilingtools/timeprofiler.cpp) {
    INCLUDEPATH += $$DESTDIR/../../profilingtools
    HEADERS += $$DESTDIR/../../profilingtools/timeprofiler.h
    SOURCES += $$DESTDIR/../../profilingtools/timeprofiler.cpp
}

exists($$DESTDIR/../../profilingtools/callgraph.cpp) {
    INCLUDEPATH += $$DESTDIR/../../profilingtools
    HEADERS += $$DESTDIR/../../profilingtools/callgraph.h
    SOURCES += $$DESTDIR/../../profilingtools/callgraph.cpp
}

exists($$DESTDIR/../../apikeys) {
    INCLUDEPATH += $$DESTDIR/../../apikeys
}

include(./src/crashpad/crashpad.pri)
include(../../thirdparty/dynamic/sonnet/sonnet.pri)
include(../../thirdparty/dynamic/phtranslator/phtranslator.pri)
include(../../thirdparty/static/quazip/quazip.pri)
include(../../thirdparty/static/simplecrypt/simplecrypt.pri)
include(../../thirdparty/static/poly2tri/poly2tri.pri)

INCLUDEPATH += \
        ../../ \
        ./src \
        ./src/core \
        ./src/network \
        ./src/importers \
        ./src/exporters \
        ./src/printing \
        ./src/quick \
        ./src/quick/objects \
        ./src/quick/items \
        ./src/utils \
        ./src/document \
        ./src/interfaces \
        ./src/reports \
        ./src/restapikey 

HEADERS += \
        src/core/userguidesearchindex.h \
        src/core/utils.h \
        src/core/systemrequirements.h \
        src/core/languageengine.h \
        src/core/actionmanager.h \
        src/core/application.h \
        src/core/printerobject.h \
        src/core/scrite.h \
        src/core/filemodificationtracker.h \
        src/core/qobjectlistmodel.h \
        src/core/filelocker.h \
        src/core/enumerationmodel.h \
        src/core/qobjectproperty.h \
        src/core/localstorage.h \
        src/core/appwindow.h \
        src/core/pdfexportablegraphicsscene.h \
        src/core/peerapplookup.h \
        src/core/autoupdate.h \
        src/core/valueindexlookup.h \
        src/importers/fountainimporter.h \
        src/importers/finaldraftimporter.h \
        src/importers/openfromlibrary.h \
        src/importers/htmlimporter.h \
        src/exporters/finaldraftexporter.h \
        src/exporters/structureexporter_p.h \
        src/exporters/structureexporter.h \
        src/exporters/textexporter.h \
        src/exporters/fountainexporter.h \
        src/exporters/htmlexporter.h \
        src/exporters/pdfexporter.h \
        src/exporters/characterrelationshipsgraphexporter_p.h \
        src/exporters/characterrelationshipsgraphexporter.h \
        src/exporters/odtexporter.h \
        src/network/restapicall.h \
        src/network/user.h \
        src/network/networkstatus.h \
        src/network/networkaccessmanager.h \
        src/printing/qtextdocumentpagedprinter.h \
        src/quick/objects/filemanager.h \
        src/quick/objects/spellcheckservice.h \
        src/quick/objects/itempositionmapper.h \
        src/quick/objects/textdocument.h \
        src/quick/objects/announcement.h \
        src/quick/objects/notification.h \
        src/quick/objects/diacritichandler.h \
        src/quick/objects/textlimiter.h \
        src/quick/objects/searchengine.h \
        src/quick/objects/eventfilter.h \
        src/quick/objects/modelaggregator.h \
        src/quick/objects/colorimageprovider.h \
        src/quick/objects/polygontesselator.h \
        src/quick/objects/completionmodel.h \
        src/quick/objects/basicfileinfo.h \
        src/quick/objects/flickscrollspeedcontrol.h \
        src/quick/objects/delayedproperty.h \
        src/quick/objects/trackobject.h \
        src/quick/objects/contextmenuevent.h \
        src/quick/objects/deltadocument.h \
        src/quick/objects/tabsequencemanager.h \
        src/quick/objects/notificationmanager.h \
        src/quick/objects/syntaxhighlighter.h \
        src/quick/objects/basicfileiconprovider.h \
        src/quick/objects/errorreport.h \
        src/quick/objects/progressreport.h \
        src/quick/objects/propertyalias.h \
        src/quick/objects/batchchange.h \
        src/quick/objects/resetonchange.h \
        src/quick/objects/focustracker.h \
        src/quick/objects/standardpaths.h \
        src/quick/objects/aggregation.h \
        src/quick/items/simpletabbaritem.h \
        src/quick/items/textshapeitem.h \
        src/quick/items/textdocumentitem.h \
        src/quick/items/timelinecursoritem.h \
        src/quick/items/qimageitem.h \
        src/quick/items/ruleritem.h \
        src/quick/items/abstractshapeitem.h \
        src/quick/items/gridbackgrounditem.h \
        src/quick/items/boundingboxevaluator.h \
        src/quick/items/painterpathitem.h \
        src/utils/timeprofiler.h \
        src/utils/booleanresult.h \
        src/utils/garbagecollector.h \
        src/utils/fountain.h \
        src/utils/hourglass.h \
        src/utils/genericarraymodel.h \
        src/utils/graphlayout.h \
        src/utils/urlattributes.h \
        src/utils/qobjectfactory.h \
        src/utils/execlatertimer.h \
        src/utils/qobjectserializer.h \
        src/utils/callgraph.h \
        src/utils/modifiable.h \
        src/document/formatting.h \
        src/document/scritedocument.h \
        src/document/documentfilesystem.h \
        src/document/structure.h \
        src/document/form.h \
        src/document/screenplaytextdocument.h \
        src/document/screenplaypaginatorworker.h \
        src/document/characterrelationshipgraph.h \
        src/document/notebookmodel.h \
        src/document/scritedocumentvault.h \
        src/document/scritefilelistmodel.h \
        src/document/scritefileinfo.h \
        src/document/screenplaytreeadapter.h \
        src/document/screenplaytextdocumentoffsets.h \
        src/document/undoredo.h \
        src/document/notes.h \
        src/document/screenplaypaginator.h \
        src/document/attachments.h \
        src/document/screenplayadapter.h \
        src/document/screenplay.h \
        src/document/scene.h \
        src/interfaces/abstracttextdocumentexporter.h \
        src/interfaces/abstractreportgenerator.h \
        src/interfaces/abstractexporter.h \
        src/interfaces/abstractimporter.h \
        src/interfaces/abstractdeviceio.h \
        src/interfaces/abstractscreenplaysubsetreport.h \
        src/reports/characterscreenplayreport.h \
        src/reports/statisticsreport_p.h \
        src/reports/screenplaysubsetreport.h \
        # src/reports/locationscreenplayreport.h \
        src/reports/locationreport.h \
        src/reports/characterreport.h \
        src/reports/scenecharactermatrixreport.h \
        src/reports/twocolumnreport.h \
        src/reports/notebookreport.h \
        src/reports/statisticsreport.h 

SOURCES += \
	main.cpp \
	src/core/filemodificationtracker.cpp \
	src/core/languageengine.cpp \
	src/core/utils.cpp \
	src/core/qobjectproperty.cpp \
	src/core/autoupdate.cpp \
	src/core/pdfexportablegraphicsscene.cpp \
	src/core/localstorage.cpp \
	src/core/qobjectlistmodel.cpp \
	src/core/filelocker.cpp \
	src/core/appwindow.cpp \
	src/core/enumerationmodel.cpp \
	src/core/application_build_timestamp.cpp \
	src/core/userguidesearchindex.cpp \
	src/core/valueindexlookup.cpp \
	src/core/peerapplookup.cpp \
	src/core/systemrequirements.cpp \
	src/core/application.cpp \
	src/core/actionmanager.cpp \
	src/core/scrite.cpp \
	src/importers/finaldraftimporter.cpp \
	src/importers/fountainimporter.cpp \
	src/importers/openfromlibrary.cpp \
	src/importers/htmlimporter.cpp \
	src/exporters/htmlexporter.cpp \
	src/exporters/structureexporter.cpp \
	src/exporters/odtexporter.cpp \
	src/exporters/textexporter.cpp \
	src/exporters/pdfexporter.cpp \
	src/exporters/characterrelationshipsgraphexporter_p.cpp \
	src/exporters/structureexporter_p.cpp \
	src/exporters/characterrelationshipsgraphexporter.cpp \
	src/exporters/fountainexporter.cpp \
	src/exporters/finaldraftexporter.cpp \
	src/network/networkstatus.cpp \
	src/network/user.cpp \
	src/network/restapicall.cpp \
	src/network/networkaccessmanager.cpp \
	src/printing/qtextdocumentpagedprinter.cpp \
	src/quick/objects/colorimageprovider.cpp \
	src/quick/objects/tabsequencemanager.cpp \
	src/quick/objects/basicfileiconprovider.cpp \
	src/quick/objects/focustracker.cpp \
	src/quick/objects/modelaggregator.cpp \
	src/quick/objects/syntaxhighlighter.cpp \
	src/quick/objects/notificationmanager.cpp \
	src/quick/objects/flickscrollspeedcontrol.cpp \
	src/quick/objects/notification.cpp \
	src/quick/objects/resetonchange.cpp \
	src/quick/objects/filemanager.cpp \
	src/quick/objects/eventfilter.cpp \
	src/quick/objects/announcement.cpp \
	src/quick/objects/batchchange.cpp \
	src/quick/objects/aggregation.cpp \
	src/quick/objects/trackobject.cpp \
	src/quick/objects/basicfileinfo.cpp \
	src/quick/objects/textlimiter.cpp \
	src/quick/objects/completionmodel.cpp \
	src/quick/objects/diacritichandler.cpp \
	src/quick/objects/contextmenuevent.cpp \
	src/quick/objects/propertyalias.cpp \
	src/quick/objects/errorreport.cpp \
    src/quick/objects/progressreport.cpp \
	src/quick/objects/standardpaths.cpp \
	src/quick/objects/polygontesselator.cpp \
	src/quick/objects/itempositionmapper.cpp \
	src/quick/objects/deltadocument.cpp \
	src/quick/objects/delayedproperty.cpp \
	src/quick/objects/searchengine.cpp \
	src/quick/objects/spellcheckservice.cpp \
	src/quick/objects/textdocument.cpp \
	src/quick/items/qimageitem.cpp \
	src/quick/items/gridbackgrounditem.cpp \
	src/quick/items/painterpathitem.cpp \
	src/quick/items/simpletabbaritem.cpp \
	src/quick/items/textshapeitem.cpp \
	src/quick/items/timelinecursoritem.cpp \
	src/quick/items/textdocumentitem.cpp \
	src/quick/items/abstractshapeitem.cpp \
	src/quick/items/boundingboxevaluator.cpp \
	src/quick/items/ruleritem.cpp \
	src/utils/execlatertimer.cpp \
	src/utils/genericarraymodel.cpp \
	src/utils/urlattributes.cpp \
	src/utils/graphlayout.cpp \
	src/utils/garbagecollector.cpp \
	src/utils/fountain.cpp \
	src/utils/qobjectserializer.cpp \
	src/document/scritedocument.cpp \
	src/document/screenplay.cpp \
	src/document/form.cpp \
	src/document/scene.cpp \
	src/document/documentfilesystem.cpp \
	src/document/screenplaypaginatorworker.cpp \
	src/document/structure.cpp \
	src/document/screenplaytextdocument.cpp \
	src/document/undoredo.cpp \
	src/document/screenplaypaginator.cpp \
	src/document/screenplaytreeadapter.cpp \
	src/document/screenplayadapter.cpp \
	src/document/scritedocumentvault.cpp \
	src/document/attachments.cpp \
	src/document/notes.cpp \
	src/document/notebookmodel.cpp \
	src/document/scritefilelistmodel.cpp \
	src/document/scritefileinfo.cpp \
	src/document/screenplaytextdocumentoffsets.cpp \
	src/document/formatting.cpp \
	src/document/characterrelationshipgraph.cpp \
	src/interfaces/abstracttextdocumentexporter.cpp \
	src/interfaces/abstractdeviceio.cpp \
	src/interfaces/abstractexporter.cpp \
	src/interfaces/abstractscreenplaysubsetreport.cpp \
	src/interfaces/abstractimporter.cpp \
	src/interfaces/abstractreportgenerator.cpp \
	src/reports/statisticsreport.cpp \
	src/reports/scenecharactermatrixreport.cpp \
	src/reports/characterreport.cpp \
	src/reports/screenplaysubsetreport.cpp \
	src/reports/characterscreenplayreport.cpp \
	src/reports/twocolumnreport.cpp \
	src/reports/notebookreport.cpp \
	# src/reports/locationscreenplayreport.cpp \
	src/reports/locationreport.cpp \
	src/reports/statisticsreport_p.cpp 

RESOURCES += \
    ui.qrc \
    misc/misc.qrc \
    images/images.qrc \
    icons/icons.qrc \
    fonts/Bengali/bengali_font.qrc \
    fonts/English/english_font.qrc \
    fonts/Gujarati/gujarati_font.qrc \
    fonts/Hindi/hindi_font.qrc \
    fonts/Kannada/kannada_font.qrc \
    fonts/Malayalam/malayalam_font.qrc \
    fonts/Marathi/marathi_font.qrc \
    fonts/Oriya/oriya_font.qrc \
    fonts/Punjabi/punjabi_font.qrc \
    fonts/Rubik/rubik_font.qrc \
    fonts/Sanskrit/sanskrit_font.qrc \
    fonts/Tamil/tamil_font.qrc \
    fonts/Telugu/telugu_font.qrc 

# https://doc.qt.io/qt-5/qtwebengine-deploying.html#javascript-files-in-qt-resource-files
QTQUICK_COMPILER_SKIPPED_RESOURCES += misc/misc.qrc

macx {
    ICON = appicon.icns
    QMAKE_INFO_PLIST = Info.plist
    VERSION_INFO = "2.0.21-macos"
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

    HEADERS += src/core/platformtransliterator_macos.h
    OBJECTIVE_SOURCES += src/core/platformtransliterator_macos.mm

    LIBS += -framework Carbon

    # QMAKE_LIBS_OPENGL = -framework OpenGL
    # LIBS -= -framework AGL
    # LIBS += -framework Carbon -framework OpenGL
    # CONFIG+=sdk_no_version_check
}

win32 {
    contains(QT_ARCH, i386) {
        VERSION_INFO = "2.0.21-windows-x86"
    } else {
        VERSION_INFO = "2.0.21-windows-x64"
    }

    RC_ICONS = appicon.ico

    HEADERS += src/core/platformtransliterator_windows.h
    SOURCES += src/core/platformtransliterator_windows.cpp

    LIBS += User32.lib
}

linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += ibus-1.0

    CONFIG+=use_gold_linker
    VERSION_INFO = "2.0.21-linux"

    HEADERS += src/core/platformtransliterator_linux.h
    SOURCES += src/core/platformtransliterator_linux.cpp
}


# The following lines ensure that timestamp of application_build_timestamp.cpp is
# modified to current time stamp before every build. This ensures that build
# timestamp is always accurate whenever we initialte a build of Scrite.
win32 {
    CODE_PATH=$$shell_path($$PWD)
    QMAKE_POST_LINK = 'call COPY /B $$CODE_PATH\\src\\core\\application_build_timestamp.cpp+,,$$CODE_PATH\\src\\core\\application_build_timestamp.cpp'
} else {
    QMAKE_POST_LINK = '/bin/bash -c "touch $$PWD/src/core/application_build_timestamp.cpp"'
}


QT += gui qml quick widgets xml concurrent network quickcontrols2 multimedia printsupport svg charts pdf webengine webenginewidgets
CONFIG += qtquickcompiler
DESTDIR = $$PWD/../Release/
TARGET = Scrite

CONFIG += c++17
DEFINES += PHTRANSLATE_STATICLIB

VERSION = 1.9.9
DEFINES += SCRITE_VERSION=\\\"$$VERSION\\\"
DEFINES += SCRITE_VERSION_TYPE=\\\"beta\\\"

#QT += testlib

CONFIG += qmltypes
QML_IMPORT_NAME = io.scrite.components
QML_IMPORT_MAJOR_VERSION = 1

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
    CONFIG += qtquickcompiler
}

INCLUDEPATH += ../apikeys ../profilingtools . \
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
    3rdparty/phtranslator/LanguageCodes.h \
    3rdparty/phtranslator/PhTranslateLib.h \
    3rdparty/phtranslator/PhTranslator.h \
    3rdparty/phtranslator/stdafx.h \
    3rdparty/phtranslator/targetver.h \
    3rdparty/poly2tri/common/shapes.h \
    3rdparty/poly2tri/common/utils.h \
    3rdparty/poly2tri/poly2tri.h \
    3rdparty/poly2tri/sweep/advancing_front.h \
    3rdparty/poly2tri/sweep/cdt.h \
    3rdparty/poly2tri/sweep/sweep.h \
    3rdparty/poly2tri/sweep/sweep_context.h \
    src/core/actionmanager.h \
    src/core/appwindow.h \
    src/core/enumerationmodel.h \
    src/core/filelocker.h \
    src/core/filemodificationtracker.h \
    src/core/languageengine.h \
    src/core/localstorage.h \
    src/core/systemrequirements.h \
    src/core/utils.h \
    src/core/valueindexlookup.h \
    src/document/screenplaypaginator.h \
    src/document/screenplaypaginatorworker.h \
    src/document/screenplaytreeadapter.h \
    src/network/networkstatus.h \
    src/core/pdfexportablegraphicsscene.h \
    src/core/peerapplookup.h \
    src/core/printerobject.h \
    src/core/qobjectlistmodel.h \
    src/core/qobjectproperty.h \
    src/core/scrite.h \
    src/document/attachments.h \
    src/document/characterrelationshipgraph.h \
    src/document/form.h \
    src/document/notebookmodel.h \
    src/document/notes.h \
    src/document/screenplaytextdocumentoffsets.h \
    src/document/scritedocumentvault.h \
    src/document/scritefileinfo.h \
    src/document/scritefilelistmodel.h \
    src/exporters/characterrelationshipsgraphexporter.h \
    src/exporters/characterrelationshipsgraphexporter_p.h \
    src/exporters/structureexporter_p.h \
    src/importers/openfromlibrary.h \
    src/network/networkaccessmanager.h \
    src/network/restapicall.h \
    src/network/user.h \
    src/printing/qtextdocumentpagedprinter.h \
    src/quick/items/boundingboxevaluator.h \
    src/quick/items/qimageitem.h \
    src/quick/items/simpletabbaritem.h \
    src/quick/items/textdocumentitem.h \
    src/quick/items/timelinecursoritem.h \
    src/quick/objects/announcement.h \
    src/quick/objects/basicfileiconprovider.h \
    src/quick/objects/basicfileinfo.h \
    src/quick/objects/batchchange.h \
    src/quick/objects/colorimageprovider.h \
    src/quick/objects/completionmodel.h \
    src/quick/objects/contextmenuevent.h \
    src/quick/objects/delayedproperty.h \
    src/quick/objects/deltadocument.h \
    src/quick/objects/filemanager.h \
    src/quick/objects/flickscrollspeedcontrol.h \
    src/quick/objects/itempositionmapper.h \
    src/quick/objects/modelaggregator.h \
    src/quick/objects/propertyalias.h \
    src/quick/objects/syntaxhighlighter.h \
    src/quick/objects/tabsequencemanager.h \
    src/quick/objects/notification.h \
    src/quick/objects/searchengine.h \
    src/quick/objects/eventfilter.h \
    src/quick/objects/polygontesselator.h \
    src/quick/objects/textdocument.h \
    src/quick/objects/textlimiter.h \
    src/quick/objects/trackobject.h \
    src/quick/objects/notificationmanager.h \
    src/quick/objects/errorreport.h \
    src/quick/objects/resetonchange.h \
    src/quick/objects/focustracker.h \
    src/quick/objects/standardpaths.h \
    src/quick/objects/aggregation.h \
    src/quick/items/textshapeitem.h \
    src/quick/items/ruleritem.h \
    src/quick/items/abstractshapeitem.h \
    src/quick/items/gridbackgrounditem.h \
    src/quick/items/painterpathitem.h \
    src/reports/characterreport.h \
    src/reports/locationreport.h \
    src/reports/notebookreport.h \
    src/reports/scenecharactermatrixreport.h \
    src/reports/statisticsreport.h \
    src/reports/statisticsreport_p.h \
    src/reports/twocolumnreport.h \
    src/utils/booleanresult.h \
    src/utils/execlatertimer.h \
    src/utils/fountain.h \
    src/utils/graphlayout.h \
    src/utils/garbagecollector.h \
    src/utils/hourglass.h \
    src/utils/genericarraymodel.h \
    src/utils/qobjectfactory.h \
    src/utils/qobjectserializer.h \
    src/utils/modifiable.h \
    src/document/formatting.h \
    src/document/scritedocument.h \
    src/document/documentfilesystem.h \
    src/document/structure.h \
    src/document/screenplaytextdocument.h \
    src/document/undoredo.h \
    src/document/screenplayadapter.h \
    src/document/screenplay.h \
    src/document/scene.h \
    src/core/application.h \
    src/core/autoupdate.h \
    src/exporters/finaldraftexporter.h \
    src/exporters/structureexporter.h \
    src/exporters/textexporter.h \
    src/exporters/fountainexporter.h \
    src/exporters/htmlexporter.h \
    src/exporters/pdfexporter.h \
    src/exporters/odtexporter.h \
    src/importers/fountainimporter.h \
    src/importers/finaldraftimporter.h \
    src/importers/htmlimporter.h \
    src/interfaces/abstracttextdocumentexporter.h \
    src/interfaces/abstractreportgenerator.h \
    src/interfaces/abstractexporter.h \
    src/interfaces/abstractimporter.h \
    src/interfaces/abstractdeviceio.h \
    src/interfaces/abstractscreenplaysubsetreport.h \
    src/reports/characterscreenplayreport.h \
    src/reports/progressreport.h \
    src/reports/screenplaysubsetreport.h \
    # src/reports/locationscreenplayreport.h \
    src/utils/urlattributes.h

SOURCES += \
    main.cpp \
    3rdparty/phtranslator/PhTranslateLib.cpp \
    3rdparty/phtranslator/PhTranslator.cpp \
    3rdparty/phtranslator/stdafx.cpp \
    3rdparty/poly2tri/common/shapes.cc \
    3rdparty/poly2tri/sweep/advancing_front.cc \
    3rdparty/poly2tri/sweep/cdt.cc \
    3rdparty/poly2tri/sweep/sweep.cc \
    3rdparty/poly2tri/sweep/sweep_context.cc \
    src/core/actionmanager.cpp \
    src/core/application_build_timestamp.cpp \
    src/core/appwindow.cpp \
    src/core/enumerationmodel.cpp \
    src/core/filelocker.cpp \
    src/core/filemodificationtracker.cpp \
    src/core/languageengine.cpp \
    src/core/localstorage.cpp \
    src/core/systemrequirements.cpp \
    src/core/utils.cpp \
    src/core/valueindexlookup.cpp \
    src/document/screenplaypaginator.cpp \
    src/document/screenplaypaginatorworker.cpp \
    src/document/screenplaytreeadapter.cpp \
    src/network/networkstatus.cpp \
    src/core/pdfexportablegraphicsscene.cpp \
    src/core/peerapplookup.cpp \
    src/core/qobjectlistmodel.cpp \
    src/core/qobjectproperty.cpp \
    src/core/scrite.cpp \
    src/document/attachments.cpp \
    src/document/characterrelationshipgraph.cpp \
    src/document/form.cpp \
    src/document/notebookmodel.cpp \
    src/document/notes.cpp \
    src/document/screenplaytextdocumentoffsets.cpp \
    src/document/scritedocumentvault.cpp \
    src/document/scritefileinfo.cpp \
    src/document/scritefilelistmodel.cpp \
    src/exporters/characterrelationshipsgraphexporter.cpp \
    src/exporters/characterrelationshipsgraphexporter_p.cpp \
    src/exporters/structureexporter_p.cpp \
    src/importers/openfromlibrary.cpp \
    src/network/networkaccessmanager.cpp \
    src/network/restapicall.cpp \
    src/network/user.cpp \
    src/printing/qtextdocumentpagedprinter.cpp \
    src/quick/items/boundingboxevaluator.cpp \
    src/quick/items/qimageitem.cpp \
    src/quick/items/simpletabbaritem.cpp \
    src/quick/items/textdocumentitem.cpp \
    src/quick/items/timelinecursoritem.cpp \
    src/quick/objects/announcement.cpp \
    src/quick/objects/basicfileiconprovider.cpp \
    src/quick/objects/basicfileinfo.cpp \
    src/quick/objects/batchchange.cpp \
    src/quick/objects/colorimageprovider.cpp \
    src/quick/objects/completionmodel.cpp \
    src/quick/objects/contextmenuevent.cpp \
    src/quick/objects/delayedproperty.cpp \
    src/quick/objects/deltadocument.cpp \
    src/quick/objects/filemanager.cpp \
    src/quick/objects/flickscrollspeedcontrol.cpp \
    src/quick/objects/itempositionmapper.cpp \
    src/quick/objects/modelaggregator.cpp \
    src/quick/objects/propertyalias.cpp \
    src/quick/objects/syntaxhighlighter.cpp \
    src/quick/objects/tabsequencemanager.cpp \
    src/quick/objects/focustracker.cpp \
    src/quick/objects/notificationmanager.cpp \
    src/quick/objects/notification.cpp \
    src/quick/objects/resetonchange.cpp \
    src/quick/objects/eventfilter.cpp \
    src/quick/objects/aggregation.cpp \
    src/quick/objects/textdocument.cpp \
    src/quick/objects/textlimiter.cpp \
    src/quick/objects/trackobject.cpp \
    src/quick/objects/errorreport.cpp \
    src/quick/objects/standardpaths.cpp \
    src/quick/objects/polygontesselator.cpp \
    src/quick/objects/searchengine.cpp \
    src/quick/items/gridbackgrounditem.cpp \
    src/quick/items/painterpathitem.cpp \
    src/quick/items/textshapeitem.cpp \
    src/quick/items/abstractshapeitem.cpp \
    src/quick/items/ruleritem.cpp \
    src/reports/characterreport.cpp \
    src/reports/locationreport.cpp \
    src/reports/notebookreport.cpp \
    src/reports/scenecharactermatrixreport.cpp \
    src/reports/statisticsreport.cpp \
    src/reports/statisticsreport_p.cpp \
    src/reports/twocolumnreport.cpp \
    src/utils/execlatertimer.cpp \
    src/utils/fountain.cpp \
    src/utils/genericarraymodel.cpp \
    src/utils/graphlayout.cpp \
    src/utils/garbagecollector.cpp \
    src/utils/qobjectserializer.cpp \
    src/document/scritedocument.cpp \
    src/document/screenplay.cpp \
    src/document/scene.cpp \
    src/document/documentfilesystem.cpp \
    src/document/structure.cpp \
    src/document/screenplaytextdocument.cpp \
    src/document/undoredo.cpp \
    src/document/screenplayadapter.cpp \
    src/document/formatting.cpp \
    src/core/autoupdate.cpp \
    src/core/application.cpp \
    src/exporters/htmlexporter.cpp \
    src/exporters/structureexporter.cpp \
    src/exporters/odtexporter.cpp \
    src/exporters/textexporter.cpp \
    src/exporters/pdfexporter.cpp \
    src/exporters/fountainexporter.cpp \
    src/exporters/finaldraftexporter.cpp \
    src/importers/finaldraftimporter.cpp \
    src/importers/fountainimporter.cpp \
    src/importers/htmlimporter.cpp \
    src/interfaces/abstracttextdocumentexporter.cpp \
    src/interfaces/abstractdeviceio.cpp \
    src/interfaces/abstractexporter.cpp \
    src/interfaces/abstractscreenplaysubsetreport.cpp \
    src/interfaces/abstractimporter.cpp \
    src/interfaces/abstractreportgenerator.cpp \
    src/reports/screenplaysubsetreport.cpp \
    src/reports/characterscreenplayreport.cpp \
    src/reports/progressreport.cpp \
    # src/reports/locationscreenplayreport.cpp \
    src/utils/urlattributes.cpp

RESOURCES += \
    scrite_bengali_font.qrc \
    scrite_english_font.qrc \
    scrite_gujarati_font.qrc \
    scrite_hindi_font.qrc \
    scrite_kannada_font.qrc \
    scrite_malayalam_font.qrc \
    scrite_marathi_font.qrc \
    scrite_misc.qrc \
    scrite_oriya_font.qrc \
    scrite_punjabi_font.qrc \
    scrite_sanskrit_font.qrc \
    scrite_tamil_font.qrc \
    scrite_telugu_font.qrc \
    scrite_rubik_font.qrc \
    scrite_icons.qrc \
    scrite_images.qrc \
    scrite_ui.qrc

exists(../profilingtools/timeprofiler.cpp) {
    HEADERS += ../profilingtools/timeprofiler.h
    SOURCES += ../profilingtools/timeprofiler.cpp
}

exists(../profilingtools/callgraph.cpp) {
    HEADERS += ../profilingtools/callgraph.h
    SOURCES += ../profilingtools/callgraph.cpp
}

# https://doc.qt.io/qt-5/qtwebengine-deploying.html#javascript-files-in-qt-resource-files
QTQUICK_COMPILER_SKIPPED_RESOURCES += scrite_misc.qrc

macx {
    ICON = appicon.icns
    QMAKE_INFO_PLIST = Info.plist
    VERSION_INFO = "1.9.9-macos"

    HEADERS += src/core/platformtransliterator_macos.h
    OBJECTIVE_SOURCES += src/core/platformtransliterator_macos.mm
    QMAKE_LIBS_OPENGL = -framework OpenGL
    LIBS -= -framework AGL
    LIBS += -framework Carbon -framework OpenGL
    CONFIG+=sdk_no_version_check
}

win32 {
    contains(QT_ARCH, i386) {
        VERSION_INFO = "1.9.9-windows-x86"
    } else {
        VERSION_INFO = "1.9.9-windows-x64"
    }

    RC_ICONS = appicon.ico
    HEADERS += src/core/platformtransliterator_windows.h
    SOURCES += src/core/platformtransliterator_windows.cpp

    LIBS += User32.lib
}

linux {
    QT += dbus

    CONFIG+=use_gold_linker
    VERSION_INFO = "1.9.9-linux"

    HEADERS += src/core/platformtransliterator_linux.h
    SOURCES += src/core/platformtransliterator_linux.cpp
}

include($$PWD/3rdparty/sonnet/sonnet.pri)
include($$PWD/3rdparty/quazip/quazip.pri)
include($$PWD/3rdparty/simplecrypt/simplecrypt.pri)
include($$PWD/3rdparty/crashpad/crashpad.pri)

DISTFILES += \
    3rdparty/openssl/README \
    3rdparty/openssl/license.txt \
    Info.plist \
    README \
    packaging/linux/package.sh \
    packaging/linux/Scrite.desktop \
    packaging/windows/FileAssociation.nsh \
    packaging/windows/license.txt \
    packaging/windows/package-x86.bat \
    packaging/windows/package-x64.bat \
    packaging/windows/installer-x86.nsi.in \
    packaging/windows/installer-x64.nsi.in \
    packaging/mac/package.sh \
    packaging/mac/prepare.sh \
    packaging/mac/dmgbackdrop.qml \
    3rdparty/poly2tri/License.txt \
    packaging/windows/qt.conf \
    tools/urlattribs/urlattribs.php \
    tools/urlattribs/OpenGraph.php

# The following lines ensure that timestamp of application_build_timestamp.cpp is
# modified to current time stamp before every build. This ensures that build
# timestamp is always accurate whenever we initialte a build of Scrite.
win32 {
    CODE_PATH=$$shell_path($$PWD)
    QMAKE_POST_LINK = 'call COPY /B $$CODE_PATH\\src\\core\\application_build_timestamp.cpp+,,$$CODE_PATH\\src\\core\\application_build_timestamp.cpp'
} else {
    QMAKE_POST_LINK = '/bin/bash -c "touch $$PWD/src/core/application_build_timestamp.cpp"'
}


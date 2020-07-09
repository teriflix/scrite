QT += gui qml quick widgets xml concurrent network quickcontrols2
DESTDIR = $$PWD/../Release/
TARGET = Scrite

DEFINES += PHTRANSLATE_STATICLIB

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

INCLUDEPATH += . src/

HEADERS += \
    3rdparty/phtranslator/LanguageCodes.h \
    3rdparty/phtranslator/PhTranslateLib.h \
    3rdparty/phtranslator/PhTranslator.h \
    3rdparty/phtranslator/stdafx.h \
    3rdparty/phtranslator/targetver.h \
    src/abstractdeviceio.h \
    src/abstractexporter.h \
    src/abstractimporter.h \
    src/abstractreportgenerator.h \
    src/abstractscreenplaysubsetreport.h \
    src/abstractshapeitem.h \
    src/abstracttextdocumentexporter.h \
    src/aggregation.h \
    src/application.h \
    src/autoupdate.h \
    src/characterreportgenerator.h \
    src/characterscreenplayreport.h \
    src/completer.h \
    src/delayedpropertybinder.h \
    src/documentfilesystem.h \
    src/errorreport.h \
    src/eventfilter.h \
    src/fileinfo.h \
    src/finaldraftexporter.h \
    src/finaldraftimporter.h \
    src/focustracker.h \
    src/formatting.h \
    src/fountainexporter.h \
    src/fountainimporter.h \
    src/garbagecollector.h \
    src/genericarraymodel.h \
    src/gridbackgrounditem.h \
    src/hourglass.h \
    src/htmlexporter.h \
    src/htmlimporter.h \
    src/imageprinter.h \
    src/locationreportgenerator.h \
    src/locationscreenplayreport.h \
    src/materialcolors.h \
    src/modifiable.h \
    src/note.h \
    src/notification.h \
    src/notificationmanager.h \
    src/odtexporter.h \
    src/painterpathitem.h \
    src/pdfexporter.h \
    3rdparty/poly2tri/common/shapes.h \
    3rdparty/poly2tri/common/utils.h \
    3rdparty/poly2tri/poly2tri.h \
    3rdparty/poly2tri/sweep/advancing_front.h \
    3rdparty/poly2tri/sweep/cdt.h \
    3rdparty/poly2tri/sweep/sweep.h \
    3rdparty/poly2tri/sweep/sweep_context.h \
    src/polygontesselator.h \
    src/progressreport.h \
    src/qobjectfactory.h \
    src/qobjectserializer.h \
    src/qtextdocumentpagedprinter.h \
    src/resetonchange.h \
    src/ruleritem.h \
    src/scene.h \
    src/scenecharactermatrixreportgenerator.h \
    src/screenplay.h \
    src/screenplayadapter.h \
    src/screenplaysubsetreport.h \
    src/screenplaytextdocument.h \
    src/scritedocument.h \
    src/searchengine.h \
    src/simpletimer.h \
    src/structure.h \
    src/structureexporter.h \
    src/textexporter.h \
    src/textshapeitem.h \
    src/tightboundingbox.h \
    src/timeprofiler.h \
    src/standardpaths.h \
    src/trackobject.h \
    src/transliteration.h \
    src/undoredo.h

SOURCES += \
    3rdparty/phtranslator/PhTranslateLib.cpp \
    3rdparty/phtranslator/PhTranslator.cpp \
    3rdparty/phtranslator/stdafx.cpp \
    src/abstractdeviceio.cpp \
    src/abstractexporter.cpp \
    src/abstractimporter.cpp \
    src/abstractreportgenerator.cpp \
    src/abstractscreenplaysubsetreport.cpp \
    src/abstractshapeitem.cpp \
    src/abstracttextdocumentexporter.cpp \
    src/aggregation.cpp \
    src/application.cpp \
    src/autoupdate.cpp \
    src/characterreportgenerator.cpp \
    src/characterscreenplayreport.cpp \
    src/completer.cpp \
    src/delayedpropertybinder.cpp \
    src/documentfilesystem.cpp \
    src/errorreport.cpp \
    src/eventfilter.cpp \
    src/fileinfo.cpp \
    src/finaldraftexporter.cpp \
    src/finaldraftimporter.cpp \
    src/focustracker.cpp \
    src/formatting.cpp \
    src/fountainexporter.cpp \
    src/fountainimporter.cpp \
    src/garbagecollector.cpp \
    src/genericarraymodel.cpp \
    src/gridbackgrounditem.cpp \
    src/htmlexporter.cpp \
    src/htmlimporter.cpp \
    src/imageprinter.cpp \
    src/locationreportgenerator.cpp \
    src/locationscreenplayreport.cpp \
    main.cpp \
    src/materialcolors.cpp \
    src/note.cpp \
    src/notification.cpp \
    src/notificationmanager.cpp \
    src/odtexporter.cpp \
    src/painterpathitem.cpp \
    src/pdfexporter.cpp \
    3rdparty/poly2tri/common/shapes.cc \
    3rdparty/poly2tri/sweep/advancing_front.cc \
    3rdparty/poly2tri/sweep/cdt.cc \
    3rdparty/poly2tri/sweep/sweep.cc \
    3rdparty/poly2tri/sweep/sweep_context.cc \
    src/polygontesselator.cpp \
    src/progressreport.cpp \
    src/qobjectserializer.cpp \
    src/qtextdocumentpagedprinter.cpp \
    src/resetonchange.cpp \
    src/ruleritem.cpp \
    src/scene.cpp \
    src/scenecharactermatrixreportgenerator.cpp \
    src/screenplay.cpp \
    src/screenplayadapter.cpp \
    src/screenplaysubsetreport.cpp \
    src/screenplaytextdocument.cpp \
    src/scritedocument.cpp \
    src/searchengine.cpp \
    src/simpletimer.cpp \
    src/structure.cpp \
    src/structureexporter.cpp \
    src/textexporter.cpp \
    src/textshapeitem.cpp \
    src/tightboundingbox.cpp \
    src/timeprofiler.cpp \
    src/standardpaths.cpp \
    src/trackobject.cpp \
    src/transliteration.cpp \
    src/undoredo.cpp

RESOURCES += \
    scrite_bengali_font.qrc \
    scrite_english_font.qrc \
    scrite_gujarati_font.qrc \
    scrite_hindi_font.qrc \
    scrite_kannada_font.qrc \
    scrite_malayalam_font.qrc \
    scrite_oriya_font.qrc \
    scrite_punjabi_font.qrc \
    scrite_sanskrit_font.qrc \
    scrite_tamil_font.qrc \
    scrite_telugu_font.qrc \
    scrite_raleway_font.qrc \
    scrite_icons.qrc \
    scrite_images.qrc \
    scrite_ui.qrc

macx {
    ICON = appicon.icns
    QMAKE_INFO_PLIST = Info.plist
}

win32 {
    RC_ICONS = appicon.ico
}

include($$PWD/3rdparty/sonnet/sonnet.pri)

DISTFILES += \
    Info.plist \
    README \
    packaging/linux/install.sh \
    packaging/linux/package.sh \
    packaging/linux/scrite.desktop \
    packaging/windows/package-x86.bat \
    packaging/windows/package-x64.bat \
    packaging/windows/installer-x86.nsi \
    packaging/windows/installer-x64.nsi \
    packaging/mac/package.sh \
    packaging/mac/prepare.sh \
    3rdparty/poly2tri/License.txt



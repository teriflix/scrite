QT += gui qml quick widgets xml concurrent network quickcontrols2
DESTDIR = $$PWD/../Release/

DEFINES += PHTRANSLATE_STATICLIB

HEADERS += \
    3rdparty/phtranslator/LanguageCodes.h \
    3rdparty/phtranslator/PhTranslateLib.h \
    3rdparty/phtranslator/PhTranslator.h \
    3rdparty/phtranslator/stdafx.h \
    3rdparty/phtranslator/targetver.h \
    abstractdeviceio.h \
    abstractexporter.h \
    abstractimporter.h \
    abstractreportgenerator.h \
    abstractshapeitem.h \
    abstracttextdocumentexporter.h \
    aggregation.h \
    application.h \
    autoupdate.h \
    characterreportgenerator.h \
    completer.h \
    delayedpropertybinder.h \
    errorreport.h \
    eventfilter.h \
    finaldraftexporter.h \
    finaldraftimporter.h \
    focustracker.h \
    formatting.h \
    fountainexporter.h \
    fountainimporter.h \
    garbagecollector.h \
    genericarraymodel.h \
    gridbackgrounditem.h \
    hourglass.h \
    htmlexporter.h \
    htmlimporter.h \
    imageprinter.h \
    locationreportgenerator.h \
    materialcolors.h \
    modifiable.h \
    note.h \
    notification.h \
    notificationmanager.h \
    odtexporter.h \
    painterpathitem.h \
    pdfexporter.h \
    3rdparty/poly2tri/common/shapes.h \
    3rdparty/poly2tri/common/utils.h \
    3rdparty/poly2tri/poly2tri.h \
    3rdparty/poly2tri/sweep/advancing_front.h \
    3rdparty/poly2tri/sweep/cdt.h \
    3rdparty/poly2tri/sweep/sweep.h \
    3rdparty/poly2tri/sweep/sweep_context.h \
    polygontesselator.h \
    progressreport.h \
    qobjectfactory.h \
    qobjectserializer.h \
    qtextdocumentpagedprinter.h \
    resetonchange.h \
    scene.h \
    scenecharactermatrixreportgenerator.h \
    screenplay.h \
    screenplaytextdocument.h \
    scritedocument.h \
    searchengine.h \
    simpletimer.h \
    structure.h \
    structureexporter.h \
    textexporter.h \
    textshapeitem.h \
    timeprofiler.h \
    standardpaths.h \
    trackobject.h \
    transliteration.h \
    undoredo.h

SOURCES += \
    3rdparty/phtranslator/PhTranslateLib.cpp \
    3rdparty/phtranslator/PhTranslator.cpp \
    3rdparty/phtranslator/stdafx.cpp \
    abstractdeviceio.cpp \
    abstractexporter.cpp \
    abstractimporter.cpp \
    abstractreportgenerator.cpp \
    abstractshapeitem.cpp \
    abstracttextdocumentexporter.cpp \
    aggregation.cpp \
    application.cpp \
    autoupdate.cpp \
    characterreportgenerator.cpp \
    completer.cpp \
    delayedpropertybinder.cpp \
    errorreport.cpp \
    eventfilter.cpp \
    finaldraftexporter.cpp \
    finaldraftimporter.cpp \
    focustracker.cpp \
    formatting.cpp \
    fountainexporter.cpp \
    fountainimporter.cpp \
    garbagecollector.cpp \
    genericarraymodel.cpp \
    gridbackgrounditem.cpp \
    htmlexporter.cpp \
    htmlimporter.cpp \
    imageprinter.cpp \
    locationreportgenerator.cpp \
    main.cpp \
    materialcolors.cpp \
    note.cpp \
    notification.cpp \
    notificationmanager.cpp \
    odtexporter.cpp \
    painterpathitem.cpp \
    pdfexporter.cpp \
    3rdparty/poly2tri/common/shapes.cc \
    3rdparty/poly2tri/sweep/advancing_front.cc \
    3rdparty/poly2tri/sweep/cdt.cc \
    3rdparty/poly2tri/sweep/sweep.cc \
    3rdparty/poly2tri/sweep/sweep_context.cc \
    polygontesselator.cpp \
    progressreport.cpp \
    qobjectserializer.cpp \
    qtextdocumentpagedprinter.cpp \
    resetonchange.cpp \
    scene.cpp \
    scenecharactermatrixreportgenerator.cpp \
    screenplay.cpp \
    screenplaytextdocument.cpp \
    scritedocument.cpp \
    searchengine.cpp \
    simpletimer.cpp \
    structure.cpp \
    structureexporter.cpp \
    textexporter.cpp \
    textshapeitem.cpp \
    timeprofiler.cpp \
    standardpaths.cpp \
    trackobject.cpp \
    transliteration.cpp \
    undoredo.cpp

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
    packaging/mac/packaging.sh \
    3rdparty/poly2tri/License.txt



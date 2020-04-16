QT += gui qml quick widgets xml concurrent network
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
    garbagecollector.h \
    gridbackgrounditem.h \
    hourglass.h \
    htmlexporter.h \
    htmlimporter.h \
    locationreportgenerator.h \
    logger.h \
    note.h \
    notification.h \
    notificationmanager.h \
    odtexporter.h \
    painterpathitem.h \
    pdfexporter.h \
    poly2tri/common/shapes.h \
    poly2tri/common/utils.h \
    poly2tri/poly2tri.h \
    poly2tri/sweep/advancing_front.h \
    poly2tri/sweep/cdt.h \
    poly2tri/sweep/sweep.h \
    poly2tri/sweep/sweep_context.h \
    polygontesselator.h \
    progressreport.h \
    qobjectfactory.h \
    qobjectserializer.h \
    qtextdocumentpagedprinter.h \
    scene.h \
    screenplay.h \
    scritedocument.h \
    searchengine.h \
    structure.h \
    structureexporter.h \
    textexporter.h \
    textshapeitem.h \
    timeprofiler.h \
    standardpaths.h \
    transliteration.h \
    undostack.h

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
    garbagecollector.cpp \
    gridbackgrounditem.cpp \
    htmlexporter.cpp \
    htmlimporter.cpp \
    locationreportgenerator.cpp \
    logger.cpp \
    main.cpp \
    note.cpp \
    notification.cpp \
    notificationmanager.cpp \
    odtexporter.cpp \
    painterpathitem.cpp \
    pdfexporter.cpp \
    poly2tri/common/shapes.cc \
    poly2tri/sweep/advancing_front.cc \
    poly2tri/sweep/cdt.cc \
    poly2tri/sweep/sweep.cc \
    poly2tri/sweep/sweep_context.cc \
    polygontesselator.cpp \
    progressreport.cpp \
    qobjectserializer.cpp \
    qtextdocumentpagedprinter.cpp \
    scene.cpp \
    screenplay.cpp \
    scritedocument.cpp \
    searchengine.cpp \
    structure.cpp \
    structureexporter.cpp \
    textexporter.cpp \
    textshapeitem.cpp \
    timeprofiler.cpp \
    standardpaths.cpp \
    transliteration.cpp \
    undostack.cpp

RESOURCES += \
    scrite.qrc

macx {
    ICON = appicon.icns
    QMAKE_INFO_PLIST = Info.plist
}

win32 {
    RC_ICONS = appicon.ico
}

DISTFILES += \
    Info.plist \
    License.txt \
    packaging/windows/package.bat \
    packaging/windows/installer.nsi \
    packaging/mac/packaging.sh \
    poly2tri/License.txt



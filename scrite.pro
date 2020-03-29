QT += gui qml quick widgets xml
DESTDIR = $$PWD/../Release/

HEADERS += \
    abstractdeviceio.h \
    abstractexporter.h \
    abstractimporter.h \
    abstractshapeitem.h \
    aggregation.h \
    application.h \
    completer.h \
    errorreport.h \
    eventfilter.h \
    finaldraftexporter.h \
    finaldraftimporter.h \
    formatting.h \
    gridbackgrounditem.h \
    logger.h \
    notification.h \
    notificationmanager.h \
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
    scene.h \
    screenplay.h \
    scritedocument.h \
    structure.h \
    textshapeitem.h \
    timeprofiler.h

SOURCES += \
    abstractdeviceio.cpp \
    abstractexporter.cpp \
    abstractimporter.cpp \
    abstractshapeitem.cpp \
    aggregation.cpp \
    application.cpp \
    completer.cpp \
    errorreport.cpp \
    eventfilter.cpp \
    finaldraftexporter.cpp \
    finaldraftimporter.cpp \
    formatting.cpp \
    gridbackgrounditem.cpp \
    logger.cpp \
    main.cpp \
    notification.cpp \
    notificationmanager.cpp \
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
    scene.cpp \
    screenplay.cpp \
    scritedocument.cpp \
    structure.cpp \
    textshapeitem.cpp \
    timeprofiler.cpp

RESOURCES += \
    scrite.qrc

macx {
    ICON = appicon.icns
    QMAKE_INFO_PLIST = Info.plist
}

DISTFILES += \
    GPLv3.txt \
    Info.plist \
    License.txt

# ~/Qt5.13.2/5.13.2/clang_64/bin/macdeployqt scrite.app -qmldir=/Users/prashanthudupa/GitHubCode/scrite/qml -verbose=1 -appstore-compliant -dmg


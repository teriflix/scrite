TEMPLATE = lib
CONFIG += static
DESTDIR = $$PWD/../../../binary

macx: QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

HEADERS +=  poly2tri.h \
            common/utils.h \
            common/shapes.h \
            sweep/advancing_front.h \
            sweep/sweep.h \
            sweep/sweep_context.h \
            sweep/cdt.h 

SOURCES +=  common/shapes.cc \
            sweep/sweep.cc \
            sweep/sweep_context.cc \
            sweep/cdt.cc \
            sweep/advancing_front.cc

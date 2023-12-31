
include(qu-particle-accel.pri)

VERSION_HEX = 0x0100000
VERSION = 1.0.0

TARGET = $${qu_pa_a_LIB}
TEMPLATE = lib

QT += gui svg

TEMPLATE = lib
DEFINES += QUPARTICLEACCEL_LIBRARY

DEFINES -= QT_NO_DEBUG_OUTPUT
CONFIG += debug

CONFIG += c++17 pkgconfig


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/qujson2svg_w.cpp \
    src/qupaitem.cpp \
    src/qusvgcomponentloader.cpp

HEADERS += \
    qu-particle-accel_global.h \
    src/qujson2svg_w.h \
    src/qupaitem.h \
    src/qusvgcomponentloader.h


DISTFILES += \
    Doxyfile

unix {
    doc.commands = \
    doxygen \
    Doxyfile;

    doc.files = doc/*
    doc.path = $${QUMBIA_PA_A_DOCDIR}
    QMAKE_EXTRA_TARGETS += doc

    inc.files = $${HEADERS}
    inc.path = $${QUMBIA_PA_A_INCLUDES}

    other_inst.files = $${DISTFILES}
    other_inst.path = $${QUMBIA_PA_A_INCLUDES}

    target.path = $${QUMBIA_PA_A_LIBDIR}
    INSTALLS += target inc other_inst

    !android-g++ {
        INSTALLS += doc
    }
} # unix

# generate pkg config file
CONFIG += create_pc create_prl no_install_prl

    QMAKE_PKGCONFIG_NAME = $${qu_pa_a_LIB}
    QMAKE_PKGCONFIG_DESCRIPTION = cumbia module for Qt PA_A integration
    QMAKE_PKGCONFIG_PREFIX = $${INSTALL_ROOT}
    QMAKE_PKGCONFIG_LIBDIR = $${target.path}
    QMAKE_PKGCONFIG_INCDIR = $${inc.path}
    QMAKE_PKGCONFIG_VERSION = $${VERSION}
    QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INCLUDEPATH += src src/cumbia src/events

# remove ourselves from -l (.pri)
LIBS -= -l$${qu_pa_a_LIB}

RESOURCES += \
    qu-particle-accel.qrc

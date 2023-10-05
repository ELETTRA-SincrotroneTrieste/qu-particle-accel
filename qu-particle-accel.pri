# + ----------------------------------------------------------------- +
#
# Customization section:
#
# Customize the following paths according to your installation:
#
#
# Here qumbia-pa_a will be installed
# INSTALL_ROOT can be specified from the command line running qmake "INSTALL_ROOT=/my/install/path"
#

isEmpty(INSTALL_ROOT) {
    INSTALL_ROOT = /usr/local/cumbia-libs
}

#
#
# Here qumbia-pa_a include files will be installed
    QUMBIA_PA_A_INCLUDES=$${INSTALL_ROOT}/include/qu-particle-accel
#
#
# Here qumbia-pa_a share files will be installed
#
    QUMBIA_PA_A_SHARE=$${INSTALL_ROOT}/share/qu-particle-accel
#
#
# Here qumbia-pa_a libraries will be installed
    QUMBIA_PA_A_LIBDIR=$${INSTALL_ROOT}/lib
#
#
# Here qumbia-pa_a documentation will be installed
    QUMBIA_PA_A_DOCDIR=$${INSTALL_ROOT}/share/doc/qu-particle-accel
#
# The name of the library
    qu_pa_a_LIB=qu-particle-accel
#
#


MOC_DIR=moc
OBJECTS_DIR=objs

unix:!android-g++ {
    CONFIG += link_pkgconfig
    PKGCONFIG += cumbia cumbia-qtcontrols
}

QT       += widgets xml svg opengl

LIBS += -L$${INSTALL_ROOT}/lib -l$${qu_pa_a_LIB}


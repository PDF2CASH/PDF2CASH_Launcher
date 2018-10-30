#-------------------------------------------------
#
# Project created by QtCreator 2018-10-13T13:11:31
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += network
QT       += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Launcher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    downloadmanager.cpp \
    miniz/miniz.c

HEADERS += \
        mainwindow.h \
    downloadmanager.h \
    miniz/miniz.h

FORMS += \
        mainwindow.ui

#INCLUDEPATH += "Includes"
#LIBS += -L"Libs"
#LIBS +=  -lz

QUAZIPCODEDIR = "/home/litwin/Launcher/ThirdParty/quazip-0.7.3/quazip/"
ZLIBCODEDIR = "/home/litwin/Launcher/ThirdParty/zlib-1.2.11/"
LIBSCODEDIR = "/home/litwin/Launcher/Libs/"

#INCLUDEPATH += $${QUAZIPCODEDIR}

#unix {
    LIBS += -L$${LIBSCODEDIR}
    #LIBS += -lquazip -lz
#}

#win32 {
#    LIBS += -L$${ZLIBCODEDIR} -lzdll
#}

#LIBS += -L$${LIBSCODEDIR} -lquazip


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


QT       += core gui

TARGET = qtEip
TEMPLATE = app


SOURCES += src/main.cpp src/mainwindow.cpp \
    lib/QPlc.cpp \
    lib/PlcException.cpp \
    lib/QPlcTag.cpp

HEADERS  += src/mainwindow.h \
    lib/QPlc.h \
    lib/PlcException.h \
    lib/QPlcTag.h

FORMS    += ui/mainwindow.ui

# CONFIG += qt release warn_on console

DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

DEFINES = QT __cplusplus
LIBS = -ltuxeip

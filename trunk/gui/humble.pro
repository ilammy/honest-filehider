#-------------------------------------------------
#
# Project created by QtCreator 2012-11-01T21:41:15
#
#-------------------------------------------------

QT       += core gui

TARGET = humble
TEMPLATE = app


SOURCES += \
        src\main.cpp \
        src\MainWindow.cpp \
    src/HiddenFilesModel.cpp \
    src/HiddenFile.cpp

HEADERS += \
        src\MainWindow.hpp \
    src/HiddenFilesModel.h \
    src/HiddenFile.h

FORMS   += \
        ui\MainWindow.ui

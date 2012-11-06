#-------------------------------------------------
#
# Project created by QtCreator 2012-11-01T21:41:15
#
#-------------------------------------------------

QT       += core gui

TARGET = humble
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/HiddenFile.cpp \
    src/HiddenModel.cpp

HEADERS += \
    src/MainWindow.hpp \
    src/HiddenFile.h \
    src/HiddenModel.h

FORMS   += \
    ui/MainWindow.ui

test {
  QT += testlib
  SOURCES -= src/main.cpp

  SOURCES += \
    test/main.cpp \
    test/Test_HiddenModel.cpp \
    test/Test_HiddenFile.cpp

  HEADERS += \
    test/Test_HiddenModel.h \
    test/Test_HiddenFile.h
}

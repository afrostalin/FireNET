QT += core
QT -= gui

CONFIG += c++11

TARGET = AutoPatcher
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    global.cpp \
    worker.cpp \
    patchreader.cpp \
    variables.cpp

HEADERS += \
    global.h \
    worker.h \
    patchreader.h \
    variables.h

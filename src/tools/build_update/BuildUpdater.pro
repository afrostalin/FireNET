QT += core
QT -= gui

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TARGET = BuildUpdater
MOC_DIR += $$PWD/../../../build/moc/BuildUpdater
OBJECTS_DIR += $$PWD/../../../build/obj/BuildUpdater

INCLUDEPATH += $$PWD/../../server/


TEMPLATE = app

SOURCES += main.cpp

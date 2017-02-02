QT += core
QT += network
QT -= gui

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TARGET = AutoTest
MOC_DIR += $$PWD/../../../build/moc/AutoTest
OBJECTS_DIR += $$PWD/../../../build/obj/AutoTest

INCLUDEPATH += $$PWD/../../server/

TEMPLATE = app

SOURCES += main.cpp \
    netpacket.cpp \
    tcpclient.cpp

HEADERS += \
    netpacket.h \
    tcpclient.h

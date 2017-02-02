QT += core
QT -= gui
QT += network

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TARGET = RemoteAdminPanel
MOC_DIR += $$PWD/../../../build/moc/RemoteAdminPanel
OBJECTS_DIR += $$PWD/../../../build/obj/RemoteAdminPanel

INCLUDEPATH += $$PWD/../../server/


TEMPLATE = app

SOURCES += main.cpp \
    inputlistener.cpp \
    global.cpp \
    client.cpp \
    netpacket.cpp

HEADERS += \
    inputlistener.h \
    global.h \
    client.h 
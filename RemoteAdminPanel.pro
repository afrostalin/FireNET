QT += core
QT -= gui
QT += network

CONFIG += c++11

MOC_DIR += $$PWD/temp/moc/remote_client
OBJECTS_DIR += $$PWD/temp/obj/remote_client

TARGET = RemoteAdminPanel
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += src/remote_admin_panel/main.cpp \
    src/remote_admin_panel/inputlistener.cpp \
    src/remote_admin_panel/global.cpp \
    src/remote_admin_panel/client.cpp

HEADERS += \
    src/remote_admin_panel/inputlistener.h \
    src/remote_admin_panel/global.h \
    src/remote_admin_panel/client.h

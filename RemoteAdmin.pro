QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RemoteAdmin
TEMPLATE = app

MOC_DIR += $$PWD/temp/moc/remote_client
OBJECTS_DIR += $$PWD/temp/obj/remote_client

SOURCES += src/remote_aministration/main.cpp\
        src/remote_aministration/mainwindow.cpp

HEADERS  += src/remote_aministration/mainwindow.h

FORMS    += src/remote_aministration/mainwindow.ui

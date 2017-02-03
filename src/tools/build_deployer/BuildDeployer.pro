QT += core
QT -= gui

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TARGET = BuildDeployer
MOC_DIR += $$PWD/../../../build/moc/BuildDeployer
OBJECTS_DIR += $$PWD/../../../build/obj/BuildDeployer

TEMPLATE = app

SOURCES += main.cpp
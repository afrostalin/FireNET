QT += core
QT += network
QT -= gui

CONFIG += c++11

TARGET = auto_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    netpacket.cpp \
    tcpclient.cpp

HEADERS += \
    netpacket.h \
    tcpclient.h

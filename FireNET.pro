QT -= gui
QT += core
QT += network
QT += sql

CONFIG += c++11

TARGET = FireNET
CONFIG += console
CONFIG -= app_bundle

MOC_DIR += $$PWD/temp/moc/server
OBJECTS_DIR += $$PWD/temp/obj/server

TEMPLATE = app

SOURCES += src/server/clientquerys.cpp \
    src/server/main.cpp \
    src/server/tcpconnection.cpp \
    src/server/tcpserver.cpp \
    src/server/tcpthread.cpp \
    src/server/redisconnector.cpp \
    src/server/global.cpp \
    src/server/helper.cpp \
    src/server/dbworker.cpp \
    src/server/mysqlconnector.cpp \
    src/server/httpconnector.cpp \
    src/server/remoteclientquerys.cpp \
    src/server/remoteserver.cpp \
    src/server/remoteconnection.cpp \
    src/server/settings.cpp \
    src/server/netpacket.cpp

HEADERS += \
    src/server/clientquerys.h \
    src/server/tcpconnection.h \
    src/server/tcpserver.h \
    src/server/tcpthread.h \
    src/server/redisconnector.h \
    src/server/global.h \
    src/server/dbworker.h \
    src/server/mysqlconnector.h \
    src/server/httpconnector.h \
    src/server/remoteclientquerys.h \
    src/server/remoteserver.h \
    src/server/remoteconnection.h \
    src/server/settings.h \
    src/server/netpacket.h

INCLUDEPATH += $$PWD/3rd/includes
INCLUDEPATH += $$PWD/3rd/includes/libssh2
INCLUDEPATH += $$PWD/3rd/includes/cutelogger/includes

win32 {
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/3rd/libs/windows/Debug -lqredisclient -lLogger -llibssh2 -llibeay32 -lssleay32 -lgdi32 -lws2_32 -lkernel32 -luser32 -lshell32 -luuid -lole32 -ladvapi32
}
CONFIG(release, debug|release) {
    LIBS += -L$$PWD/3rd/libs/windows/Release -lqredisclient -lLogger -llibssh2 -llibeay32 -lssleay32 -lgdi32 -lws2_32 -lkernel32 -luser32 -lshell32 -luuid -lole32 -ladvapi32
}
}

unix {
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/3rd/libs/linux/Debug -lqredisclient -lLogger -lz -lssh2 -lssl
}
CONFIG(release, debug|release) {
    LIBS += -L$$PWD/3rd/libs/linux/Release -lqredisclient -lLogger -lz -lssh2 -lssl
}
}

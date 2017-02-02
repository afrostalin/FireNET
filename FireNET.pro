QT += core
QT += gui
QT += network
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = FireNET
TEMPLATE = app

MOC_DIR += $$PWD/build/moc/server
OBJECTS_DIR += $$PWD/build/obj/server

SOURCES += src/server/workers/packets/clientquerys.cpp \
    src/server/main.cpp \
    src/server/core/tcpconnection.cpp \
    src/server/core/tcpserver.cpp \
    src/server/core/tcpthread.cpp \
    src/server/workers/databases/redisconnector.cpp \
    src/server/core/global.cpp \
    src/server/workers/packets/helper.cpp \
    src/server/workers/databases/dbworker.cpp \
    src/server/workers/databases/mysqlconnector.cpp \
    src/server/workers/databases/httpconnector.cpp \
    src/server/workers/packets/remoteclientquerys.cpp \
    src/server/core/remoteserver.cpp \
    src/server/core/remoteconnection.cpp \
    src/server/tools/settings.cpp \
    src/server/core/netpacket.cpp \
    src/server/tools/scripts.cpp \
    src/server/ui/mainwindow.cpp \
    src/server/ui/UILogger.cpp \
    src/server/serverThread.cpp

HEADERS += \
    src/server/workers/packets/clientquerys.h \
    src/server/core/tcpconnection.h \
    src/server/core/tcpserver.h \
    src/server/core/tcpthread.h \
    src/server/workers/databases/redisconnector.h \
    src/server/global.h \
    src/server/workers/databases/dbworker.h \
    src/server/workers/databases/mysqlconnector.h \
    src/server/workers/databases/httpconnector.h \
    src/server/workers/packets/remoteclientquerys.h \
    src/server/core/remoteserver.h \
    src/server/core/remoteconnection.h \
    src/server/tools/settings.h \
    src/server/core/netpacket.h \
    src/server/tools/scripts.h \
    src/server/ui/mainwindow.h \
    src/server/ui/UILogger.h \
    src/server/serverThread.h

FORMS += \
	src/server/ui/mainwindow.ui

INCLUDEPATH += $$PWD/src/server/
INCLUDEPATH += $$PWD/3rd/cutelogger/includes
INCLUDEPATH += $$PWD/3rd/cpp_redis/includes
INCLUDEPATH += $$PWD/3rd/tacopie/includes

win32 {
CONFIG(debug, debug|release) {
	LIBS += -L$$PWD/3rd/cutelogger/lib/Debug -lLogger
	LIBS += -L$$PWD/3rd/cpp_redis/lib/Debug -lcpp_redis
	LIBS += -L$$PWD/3rd/tacopie/lib/Debug -ltacopie -lws2_32
}
CONFIG(release, debug|release) {
    LIBS += -L$$PWD/3rd/cutelogger/lib/Release -lLogger
	LIBS += -L$$PWD/3rd/cpp_redis/lib/Release -lcpp_redis
	LIBS += -L$$PWD/3rd/tacopie/lib/Release -ltacopie -lws2_32
}
}

unix {
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/3rd/cutelogger/lib/Debug -lLogger
	LIBS += -L$$PWD/3rd/cpp_redis/lib/Debug -lcpp_redis
	LIBS += -L$$PWD/3rd/tacopie/lib/Debug -ltacopie -lpthread
}
CONFIG(release, debug|release) {
    LIBS += -L$$PWD/3rd/cutelogger/lib/Release -lLogger
	LIBS += -L$$PWD/3rd/cpp_redis/lib/Release -lcpp_redis
	LIBS += -L$$PWD/3rd/tacopie/lib/Release -ltacopie -lpthread
}
}

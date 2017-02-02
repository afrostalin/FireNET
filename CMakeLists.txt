cmake_minimum_required (VERSION 3.6.0)
project (FireNET VERSION 2.0 LANGUAGES CXX)

# QT automoc + autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable using folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Includes
set(3RD_DIR ${PROJECT_SOURCE_DIR}/3rd)
set(LOGGER_INCLUDES ${3RD_DIR}/cutelogger/includes)
set(CPP_REDIS_INCLUDES ${3RD_DIR}/cpp_redis/includes)
set(TACOPIE_INCLUDES ${3RD_DIR}/tacopie/includes)
set(SRC_INCLUDES ${PROJECT_SOURCE_DIR}/src/server)

include_directories(${LOGGER_INCLUDES}
	${CPP_REDIS_INCLUDES}
	${TACOPIE_INCLUDES}
	${SRC_INCLUDES}
)

# Qt creator fix
add_definitions(-DQT_CREATOR_FIX_COMPILE)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /O2")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wextra -O3")
endif()

# Find Qt libs and includes
set(QT_DIR ${PROJECT_SOURCE_DIR}/3rd/qt)
set(Qt5_DIR ${QT_DIR})
find_package(Qt5 COMPONENTS Core Network Widgets Gui Sql REQUIRED PATHS "${QT_DIR}")

# Path to libs
set(CPP_REDIS_LIBS ${3RD_DIR}/cpp_redis/lib)
set(TACOPIE_LIBS ${3RD_DIR}/tacopie/lib)
set(LOGGER_LIBS ${3RD_DIR}/cutelogger/lib)

link_directories(${CPP_REDIS_LIBS} ${TACOPIE_LIBS} ${LOGGER_LIBS})
	
# CODE - Main
set (SourceGroup_Main
	"src/server/global.h"
	"src/server/main.cpp"
	"src/server/serverThread.cpp"
	"src/server/serverThread.h"
)

# CODE - Core
set (SourceGroup_Core
	"src/server/core/global.cpp"
	"src/server/core/netpacket.cpp"
	"src/server/core/netpacket.h"	
)
# CODE - Core/MasterServer
set (SourceGroup_Core_MS
	"src/server/core/tcpconnection.cpp"
	"src/server/core/tcpconnection.h"
	"src/server/core/tcpserver.cpp"
	"src/server/core/tcpserver.h"
	"src/server/core/tcpthread.cpp"
	"src/server/core/tcpthread.h"
)
# CODE - Core/RemoteServer
set (SourceGroup_Core_RS
	"src/server/core/remoteconnection.cpp"
	"src/server/core/remoteconnection.h"
	"src/server/core/remoteserver.cpp"
	"src/server/core/remoteserver.h"
)
# CODE - Tools
set (SourceGroup_Tools
	"src/server/tools/scripts.cpp"
	"src/server/tools/scripts.h"
	"src/server/tools/settings.cpp"
	"src/server/tools/settings.h"	
)
# CODE - UI
set (SourceGroup_UI
	"src/server/ui/mainwindow.cpp"
	"src/server/ui/mainwindow.h"
	"src/server/ui/mainwindow.ui"
	"src/server/ui/UILogger.cpp"
	"src/server/ui/UILogger.h"
)
# CODE - Workers/Packets
set (SourceGroup_Workers_Packets
	"src/server/workers/packets/clientquerys.cpp"
	"src/server/workers/packets/clientquerys.h"
	"src/server/workers/packets/helper.cpp"
	"src/server/workers/packets/remoteclientquerys.cpp"
	"src/server/workers/packets/remoteclientquerys.h"
)
# CODE - Workers/Databases
set (SourceGroup_Workers_DB
	"src/server/workers/databases/dbworker.cpp"
	"src/server/workers/databases/dbworker.h"
	"src/server/workers/databases/httpconnector.cpp"
	"src/server/workers/databases/httpconnector.h"
	"src/server/workers/databases/mysqlconnector.cpp"
	"src/server/workers/databases/mysqlconnector.h"
	"src/server/workers/databases/redisconnector.cpp"
	"src/server/workers/databases/redisconnector.h"
)

# Filtres
source_group("Main" FILES ${SourceGroup_Main})
source_group("Core" FILES ${SourceGroup_Core})
source_group("Core\\MasterServer" FILES ${SourceGroup_Core_MS})
source_group("Core\\RemoteServer" FILES ${SourceGroup_Core_RS})
source_group("Tools" FILES ${SourceGroup_Tools})
source_group("UI" FILES ${SourceGroup_UI})
source_group("Workers\\Packets" FILES ${SourceGroup_Workers_Packets})
source_group("Workers\\Databases" FILES ${SourceGroup_Workers_DB})

# All source code
set (SOURCE
	${SourceGroup_Main}
	${SourceGroup_Core}
	${SourceGroup_Core_MS}
	${SourceGroup_Core_RS}
	${SourceGroup_Tools}
	${SourceGroup_UI}
	${SourceGroup_Workers_Packets}
	${SourceGroup_Workers_DB}
)

# Set output folder
if(WIN32)
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Windows/server")
else()
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Unix/server")
endif()

add_executable(${PROJECT_NAME} ${SOURCE})

# Update build number
if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} 
		PRE_BUILD 
		COMMAND ../tools/BuildUpdater
		)	
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Network)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Gui)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Sql)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::WinMain)
target_link_libraries(${PROJECT_NAME} PRIVATE Logger cpp_redis tacopie)

if(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER Server
)

# Tools - Remote administration panel
add_subdirectory("src/tools/remote_admin_panel" "${CMAKE_CURRENT_BINARY_DIR}/Projects/tools/remote_admin")
# Tools - Build updater
add_subdirectory("src/tools/build_update" "${CMAKE_CURRENT_BINARY_DIR}/Projects/tools/build_update")
# Tools - Auto test
add_subdirectory("src/tools/auto_test" "${CMAKE_CURRENT_BINARY_DIR}/Projects/tools/auto_test")

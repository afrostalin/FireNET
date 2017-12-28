cmake_minimum_required (VERSION 3.6.0)
project (AutoTest VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find Qt libs and includes
set(QT_DIR ${PROJECT_SOURCE_DIR}/../../../3rd/qt)
set(Qt5_DIR ${QT_DIR})
find_package(Qt5 COMPONENTS Core Network REQUIRED PATHS "${QT_DIR}")

set(SourceGroup_Main
	"main.cpp"
	"tcpclient.cpp"
	"tcpclient.h"
	"netpacket.cpp"
	"global.cpp"
	"global.h"
)
source_group("Main" FILES ${SourceGroup_Main})

set (SOURCE ${SourceGroup_Main})

if(WIN32)
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../../bin/Windows/Server")
else()
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../../bin/Linux/Server")
endif()

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Network)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../../../includes/FireNet)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER Tools)
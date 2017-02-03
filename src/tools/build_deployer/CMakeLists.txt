cmake_minimum_required (VERSION 3.6.0)
project (BuildDeployer VERSION 2.0 LANGUAGES CXX)

# Find Qt libs and includes
set(QT_DIR ${PROJECT_SOURCE_DIR}/../../../3rd/qt)
set(Qt5_DIR ${QT_DIR})
find_package(Qt5 COMPONENTS Core REQUIRED PATHS "${QT_DIR}")

set(SourceGroup_Main
	"main.cpp"
)
source_group("Main" FILES ${SourceGroup_Main})

set (SOURCE ${SourceGroup_Main})

if(WIN32)
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../../bin/Windows/tools")
else()
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../../bin/Unix/tools")
endif()

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER Tools
)
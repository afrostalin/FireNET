cmake_minimum_required (VERSION 3.6.2)
set(CRYENGINE_DIR "F:/_Development/CustomCryEngine/Crytek/CRYENGINE_5.4")
set(TOOLS_CMAKE_DIR "${CRYENGINE_DIR}/Tools/CMake")

set(PROJECT_BUILD 1)
set(PROJECT_DIR "G:/Projects/FireNET/plugins/FireNetCore")

include("${TOOLS_CMAKE_DIR}/CommonOptions.cmake")

add_subdirectory("${CRYENGINE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/CRYENGINE")

include("${TOOLS_CMAKE_DIR}/Configure.cmake")
start_sources()

sources_platform(ALL)
add_sources("Code_uber.cpp"
    PROJECTS CryFireNetCore
    SOURCE_GROUP "Root"
		"StdAfx.cpp"
		"StdAfx.h"
)
add_sources("Main_uber.cpp"
    PROJECTS CryFireNetCore
    SOURCE_GROUP "Main"
		"Main/Plugin.cpp"
		"Main/Plugin.h"
		"Main/PluginEnv.h"
)
add_sources("Network_uber.cpp"
    PROJECTS CryFireNetCore
    SOURCE_GROUP "Network"
		"Network/NetworkThread.cpp"
		"Network/ReadQueue.cpp"
		"Network/TcpClient.cpp"
		"Network/TcpPacket.cpp"
		"Network/NetworkThread.h"
		"Network/ReadQueue.h"
		"Network/TcpClient.h"
		"Network/TcpPacket.h"
)

end_sources()

CryEngineModule(CryFireNetCore PCH "StdAfx.cpp" SOLUTION_FOLDER "Project")

target_include_directories(${THIS_PROJECT}
PRIVATE 
    "${CRYENGINE_DIR}/Code/CryEngine/CryCommon"
    "${CRYENGINE_DIR}/Code/CryEngine/CryAction"
	"${CRYENGINE_DIR}/Code/CryEngine/CrySchematyc/Core/Interface"
	"${CRYENGINE_DIR}/Code/CryPlugins/CryDefaultEntities/Module"
)

# Set StartUp project in Visual Studio

add_library(GameLauncher STATIC "${CRYENGINE_DIR}/Code/CryEngine/CryCommon/CryCore/Platform/platform.h")
set_target_properties(GameLauncher PROPERTIES LINKER_LANGUAGE CXX)
if (WIN32)
    set_visual_studio_debugger_command(GameLauncher "${CRYENGINE_DIR}/bin/win_x64/GameLauncher.exe" "-project \"G:/Projects/FireNET/plugins/FireNetCore/CryFireNetCore.cryproject\"")
endif()

add_library(Sandbox STATIC "${CRYENGINE_DIR}/Code/CryEngine/CryCommon/CryCore/Platform/platform.h")
set_target_properties(Sandbox PROPERTIES LINKER_LANGUAGE CXX)
if (WIN32)
    set_visual_studio_debugger_command(Sandbox "${CRYENGINE_DIR}/bin/win_x64/Sandbox.exe" "-project \"G:/Projects/FireNET/plugins/FireNetCore/CryFireNetCore.cryproject\"")
endif()

add_library(GameServer STATIC "${CRYENGINE_DIR}/Code/CryEngine/CryCommon/CryCore/Platform/platform.h")
set_target_properties(GameServer PROPERTIES LINKER_LANGUAGE CXX)
if (WIN32)
    set_visual_studio_debugger_command(GameServer "${CRYENGINE_DIR}/bin/win_x64/Game_Server.exe" "-project \"G:/Projects/FireNET/plugins/FireNetCore/CryFireNetCore.cryproject\"")
endif()

set_solution_startup_target(GameLauncher)

if (WIN32)
    set_visual_studio_debugger_command( ${THIS_PROJECT} "${CRYENGINE_DIR}/bin/win_x64/GameLauncher.exe" "-project \"G:/Projects/FireNET/plugins/FireNetCore/CryFireNetCore.cryproject\"" )
endif()

#BEGIN-CUSTOM
option(USE_STEAM_SDK "Disable some features for Steam integration" OFF)

# BOOST 
set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd/boost)
set(BOOST_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd/boost/boost)
set(BOOST_LIBRARYDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd/boost/lib)
set(Boost_NO_SYSTEM_PATHS        ON)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.63.0 COMPONENTS date_time system regex REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_definitions("/EHsc") # Boost need trow 
else()
  message("Boost NOT INCLUDED!")
endif()

if(USE_STEAM_SDK)
	add_definitions(-DSTEAM_SDK_ENABLED)
endif()

# OpenSsl
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 

set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd/openssl)
set(OPENSSL_MSVC_STATIC_RT ON)
set(OPENSSL_USE_STATIC_LIBS ON)
find_package(OPENSSL REQUIRED)
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
else()
  message("OpenSsl NOT INCLUDED!")
endif()

target_include_directories(${THIS_PROJECT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../includes/FireNet)
target_link_libraries(${THIS_PROJECT} PRIVATE ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

if (WIN32)
	target_link_libraries(${THIS_PROJECT} PRIVATE Ntdll)
endif(WIN32)

set_target_properties(${THIS_PROJECT} PROPERTIES LINK_FLAGS "/ignore:4099")

set_solution_startup_target(${THIS_PROJECT})
#END-CUSTOM
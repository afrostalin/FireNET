cmake_minimum_required (VERSION 3.6.0)
project (FireNetClient VERSION 1.0 LANGUAGES CXX)

option(OPTION_PCH "Enable Precompiled headers" ON)

find_package (CryCommon REQUIRED)
find_package (CryAction REQUIRED)

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
else()
  message("Boost NOT INCLUDED!")
endif()

set (SourceGroup_PluginMain
	"StdAfx.cpp"
	"StdAfx.h"
	"Main/Plugin.cpp"
	"Main/Plugin.h"
	"Main/PluginEnv.h"
)

set (SourceGroup_NetActor
	"NetActor/NetPlayer.cpp"
	"NetActor/NetPlayer.h"
	"../../Common/Actors/ISimpleActor.h"
)

set(SourceGroup_NetActorAnimations
	"NetActor/Animations/NetPlayerAnimations.cpp"
	"NetActor/Animations/NetPlayerAnimations.h"
)

set(SourceGroup_NetActorInput
	"NetActor/Input/NetPlayerInput.cpp"
	"NetActor/Input/NetPlayerInput.h"
)

set(SourceGroup_NetActorMovement
	"NetActor/Movement/NetPlayerMovement.cpp"
	"NetActor/Movement/NetPlayerMovement.h"
)

set(SourceGroup_Entities
	"../../Common/Entities/ISimpleExtension.h"
)

set(SourceGroup_Network
	"Network/UdpClient.cpp"
	"Network/UdpClient.h"
	"../../Common/Network/UdpPacket.cpp"
	"../../Common/Network/UdpPacket.h"
	"Network/SyncGameState.cpp"
	"Network/SyncGameState.h"
	"Network/ReadQueue.cpp"
	"Network/ReadQueue.h"
	"Network/NetworkThread.cpp"
	"Network/NetworkThread.h"
)

source_group("Main" FILES ${SourceGroup_PluginMain})
source_group("NetActor" FILES ${SourceGroup_NetActor})
source_group("NetActor\\Animations" FILES ${SourceGroup_NetActorAnimations})
source_group("NetActor\\Input" FILES ${SourceGroup_NetActorInput})
source_group("NetActor\\Movement" FILES ${SourceGroup_NetActorMovement})
source_group("Interfaces" FILES ${SourceGroup_Interfaces})
source_group("Entities" FILES ${SourceGroup_Entities})
source_group("Network" FILES ${SourceGroup_Network})

set (SOURCE
	${SourceGroup_PluginMain}
	${SourceGroup_NetActor}
	${SourceGroup_NetActorAnimations}
	${SourceGroup_NetActorInput}
	${SourceGroup_NetActorMovement}
	${SourceGroup_Entities}
	${SourceGroup_Network}
)

if(OPTION_PCH AND MSVC)
	set (PRECOMPILED_SOURCE ${SOURCE})
	list(REMOVE_ITEM PRECOMPILED_SOURCE "StdAfx.cpp")

	set (PRECOMPILED_BINARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pch)
	set_source_files_properties (${PRECOMPILED_SOURCE} PROPERTIES COMPILE_FLAGS "/Yu\"StdAfx.h\" /Fp\"${PRECOMPILED_BINARY}\"" OBJECT_DEPENDS "${PRECOMPILED_BINARY}")
	set_source_files_properties (StdAfx.cpp PROPERTIES COMPILE_FLAGS "/Yc\"StdAfx.h\" /Fp\"${PRECOMPILED_BINARY}\"" OBJECT_OUTPUTS "${PRECOMPILED_BINARY}")
endif (OPTION_PCH AND MSVC)

add_library(${PROJECT_NAME} MODULE ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Common)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../includes/FireNetCommon)
target_link_libraries(${PROJECT_NAME} PRIVATE CryCommon CryAction ${Boost_LIBRARIES})

if (WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE Ntdll)
endif(WIN32)
cmake_minimum_required (VERSION 3.6.0)
project (CryFireNetUI VERSION 1.0 LANGUAGES CXX)

option(OPTION_PCH "Enable Precompiled headers" ON)

find_package (CryCommon REQUIRED)

# Main
set (SourceGroup_PluginMain
	"StdAfx.cpp"
	"StdAfx.h"
	"Main/Plugin.cpp"
	"Main/Plugin.h"
	"Main/PluginEnv.h"
)

# UI
set(SourceGroup_UI
	"UI/UIManager.cpp"
	"UI/UIManager.h"
	"UI/IUIPage.h"
)

# UI pages
set(SourceGroup_UI_Pages
	"UI/Pages/UI_Pages_HUD.cpp"
	"UI/Pages/UI_Pages_Menu.cpp"
	"UI/Pages/UI_Pages_Online.cpp"
)

source_group("Main" FILES ${SourceGroup_PluginMain})
source_group("UI" FILES ${SourceGroup_UI})
source_group("UI\\Pages" FILES ${SourceGroup_UI_Pages})

set (SOURCE
	${SourceGroup_PluginMain}
	${SourceGroup_UI}
	${SourceGroup_UI_Pages}
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
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../includes/FireNet)
target_link_libraries(${PROJECT_NAME} PRIVATE CryCommon)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE Ntdll)
endif(WIN32)
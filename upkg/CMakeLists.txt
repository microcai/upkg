#cmake_minimum_required(VERSION 2.6)
project(upkg)
option(ENABLE_QT_STATIC "static llink to qt" ON)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/upkg.rc.in
		${CMAKE_CURRENT_SOURCE_DIR}/src/upkg.rc @ONLY NEWLINE_STYLE LF)

set(CMAKE_AUTOMOC ON)
set(QtComponent Core Gui Widgets)

if(WIN32)
	if(MSVC AND ENABLE_QT_STATIC)
		list(APPEND QtComponent)
	endif()
endif()

find_package(Qt6 6.2.0 COMPONENTS ${QtComponent} REQUIRED)
link_libraries(Qt6::Core Qt6::Gui Qt6::Widgets)

if(WIN32 AND MSVC AND ENABLE_QT_STATIC)
	# For static Windows builds, we also need to pull in the Qt6 Platform
	# support library, which is not exposed to CMake properly, unfortunately
	get_target_property(QT_LIB_DIR "${Qt6Widgets_LIBRARIES}" LOCATION)
	get_filename_component(QT_LIB_DIR "${QT_LIB_DIR}" PATH)

	link_libraries(
		${Qt6HB_LIBRARIES}
		${Qt6PNG_LIBRARIES}
		${Qt6QTPCRE_LIBRARIES}
		${Qt6QTFREETYPE_LIBRARIES}
		Qt6::QWindowsIntegrationPlugin
		Qt6::QWindowsVistaStylePlugin
		Qt6::QICOPlugin
	)

	add_definitions(-DSTATIC_QT6)
	add_definitions(-DQT_STATICPLUGIN)
endif()

qt6_wrap_ui(UI_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/upkg.ui)

set_property(SOURCE ${UI_SRCS} PROPERTY SKIP_AUTOMOC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${Qt6Core_INCLUDE_DIRS})
include_directories(${Qt6Gui_INCLUDE_DIRS})
include_directories(${Qt6Widgets_INCLUDE_DIRS})

# qpathedit
#qt6_add_resources(RC_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qpathedit/qpathedit_res.qrc)
#set(upkg_qpathedit_SRCS
#	qpathedit/qpathedit.cpp
#	qpathedit/qpathedit.h
#)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/qpathedit)


set(upkg_SRCS
	${UI_SRCS}
	${RC_SRCS}

	include/upkg/upkg.hpp

	src/main.cpp
	src/upkg.cpp

	src/upkg.rc
)

if (WIN32)
	link_libraries(version.lib)
endif()

add_executable(upkg WIN32
	${upkg_SRCS}
)


#cmake_minimum_required(VERSION 2.6)
project(upkg)
option(ENABLE_QT_STATIC "static llink to qt" ON)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/upkg.rc.in
		${CMAKE_CURRENT_SOURCE_DIR}/src/upkg.rc @ONLY NEWLINE_STYLE LF)

set(CMAKE_AUTOMOC ON)
set(QtComponent CoreTools GuiTools WidgetsTools Core Gui Widgets Concurrent)

if(WIN32)
	if(MSVC AND ENABLE_QT_STATIC)
		list(APPEND QtComponent BundledPcre2 BundledZLIB BundledLibpng BundledHarfbuzz BundledFreetype BundledLibjpeg )
	endif()
endif()

find_package(Qt6 COMPONENTS ${QtComponent} REQUIRED)
link_libraries(Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Concurrent)

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
else()
	link_libraries(
		udev
		input
		xkbcommon
		Qt6::QICOPlugin
		Qt6::QLinuxFbIntegrationPlugin
	)
endif()

qt6_wrap_ui(UI_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/upkg.ui)

set_property(SOURCE ${UI_SRCS} PROPERTY SKIP_AUTOMOC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${Qt6Core_INCLUDE_DIRS})
include_directories(${Qt6Gui_INCLUDE_DIRS})
include_directories(${Qt6Widgets_INCLUDE_DIRS})

set(upkg_SRCS
	${UI_SRCS}
	${RC_SRCS}

	include/upkg/upkg.hpp
	include/upkg/datamodel.hpp
	include/upkg/qcommondelegate.hpp
	include/upkg/url_view.hpp
	include/upkg/misc.hpp

	src/main.cpp
	src/upkg.cpp
	src/misc.cpp
	src/datamodel.cpp
	src/upkg.rc
)

qt6_add_resources(RC_SRCS src/res.qrc)


if (WIN32)
	link_libraries(version.lib)
endif()

add_executable(upkg WIN32
	${upkg_SRCS} ${RC_SRCS}
)


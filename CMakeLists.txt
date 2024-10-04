cmake_minimum_required(VERSION 3.15)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(upkg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(TIMESTAMP VERSION_YEAR "%Y")
string(TIMESTAMP VERSION_MONTH_YEAR "%m")
string(TIMESTAMP VERSION_DAY_YEAR "%d")
string(TIMESTAMP HOUR24 "%H")
string(TIMESTAMP MINUTE "%M")

math(EXPR VERSION_MAJOR "${VERSION_YEAR}")
math(EXPR VERSION_MINOR "${VERSION_MONTH_YEAR}")
math(EXPR VERSION_TINY "${VERSION_DAY_YEAR}")
math(EXPR VERSION_BUILD "${HOUR24}*60+${MINUTE}")

include(CheckCXXCompilerFlag)
include(CheckLibraryExists)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/cmake")

option(ENABLE_BUILD_WERROR "all warnings being treated as errors" ON)
option(ENABLE_SYSTEM_OPENSSL "link to system openssl" OFF)
option(ENABLE_USE_GIT_VERSION "use git version" OFF)

option(ENABLE_LLD "enable lld linker" OFF)

if (ENABLE_LLD)
	set (CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (ENABLE_USE_GIT_VERSION)
	find_package(Git)
	gitGetVersion(${CMAKE_CURRENT_SOURCE_DIR} superpool)
	set(VERSION_GIT ${superpool_WC_REVISION_HASH})
	string(TIMESTAMP PVERSION "%Y%m%d-${VERSION_GIT}-${CMAKE_SYSTEM_NAME}")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-DBOOST_ALL_STATIC_LINK)
	add_definitions(-DBOOST_THREAD_USE_LIB)
	add_definitions(-DBOOST_FILESYSTEM_STATIC_LINK)
	add_definitions(-DBOOST_USE_WINAPI_VERSION=0x0601)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
	set(CMAKE_CXX_STACK_SIZE "20000000")

	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DENABLE_LOGGER -DDISABLE_LOGGER_TO_CONSOLE -DLOGGER_DBG_VIEW -DDISABLE_WRITE_LOGGING")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DENABLE_LOGGER -DDISABLE_LOGGER_TO_CONSOLE -DLOGGER_DBG_VIEW -DDISABLE_WRITE_LOGGING")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DENABLE_LOGGER -DDISABLE_LOGGER_TO_CONSOLE -DLOGGER_DBG_VIEW -DDISABLE_WRITE_LOGGING")

	link_libraries(opengl32.lib imm32.lib ws2_32.lib Wtsapi32.lib winmm.lib shell32.lib uxtheme.lib dwmapi.lib Crypt32.lib netapi32.lib Userenv.lib)
endif()

CHECK_CXX_COMPILER_FLAG(-fvisibility-inlines-hidden COMPILER_HAS_VISIBILITY_INLINE_HIDDEN)
CHECK_CXX_COMPILER_FLAG(-fvisibility=hidden COMPILER_HAS_VISIBILITY_HIDDEN)
CHECK_CXX_COMPILER_FLAG(-std=c++14 LIBCXX_HAS_STDCXX14_FLAG)
CHECK_CXX_COMPILER_FLAG(-std=c++17 LIBCXX_HAS_STDCXX17_FLAG)
CHECK_CXX_COMPILER_FLAG(-std=c++20 LIBCXX_HAS_STDCXX20_FLAG)
CHECK_CXX_COMPILER_FLAG(-fdiagnostics-color=always COMPILER_HAS_COLOR)

if (${COMPILER_HAS_VISIBILITY_INLINE_HIDDEN})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

if (${COMPILER_HAS_VISIBILITY_HIDDEN})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

if (${COMPILER_HAS_COLOR})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
endif()

set(Boost_USE_STATIC_LIBS	ON)
set(Boost_USE_STATIC_RUNTIME	ON)

find_package(Threads)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

set(BOOST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/boost)

add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)
add_definitions(-DBOOST_BEAST_SEPARATE_COMPILATION)

add_definitions(-DBOOST_LOCALE_HIDE_AUTO_PTR)
add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
add_subdirectory(third_party/boost EXCLUDE_FROM_ALL)

# add zip
if (WIN32)
	add_subdirectory(third_party/zlib EXCLUDE_FROM_ALL)
	set(ZLIB_LIBRARIES zlibstatic)
	set(MINIZIP_LIBRARIES minizip)
	set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zlib)
	set(MINIZIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zlib/contrib)
else()
	#find_package(ZLIB REQUIRED)
	add_subdirectory(third_party/zlib EXCLUDE_FROM_ALL)
	set(ZLIB_LIBRARIES zlibstatic)
	set(MINIZIP_LIBRARIES minizip)
	set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zlib)
	set(MINIZIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zlib/contrib)
endif()

# add tinyxml2
add_subdirectory(third_party/tinyxml2)
set(TINYXML2_LIBRARIES tinyxml2)
set(TINYXML2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2)

# add fmt
add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)
set(FMT_LIBRARIES fmt)
set(FMT_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/third_party/fmt ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fmt)

include_directories(
	${BOOST_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	${MINIZIP_INCLUDE_DIRS}
	${TINYXML2_INCLUDE_DIRS}
	${FMT_INCLUDE_DIRS}
)

if (ENABLE_SYSTEM_OPENSSL)
	find_package(OpenSSL)
else()
	add_subdirectory(third_party/openssl)
	if (NOT MSVC AND NOT APPLE)
		set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--exclude-libs,ALL ${CMAKE_SHARED_LINKER_FLAGS}")
		set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--exclude-libs,ALL ${CMAKE_MODULE_LINKER_FLAGS}")
		set(CMAKE_EXE_LINKER_FLAGS "-Wl,--exclude-libs,ALL ${CMAKE_EXE_LINKER_FLAGS}")
	endif()
endif()

include_directories(${OPENSSL_INCLUDE_DIR})

link_libraries(
		Boost::context
		Boost::thread
		Boost::date_time
		Boost::regex

		${ZLIB_LIBRARIES}
		${MINIZIP_LIBRARIES}
		${TINYXML2_LIBRARIES}
		${FMT_LIBRARIES}
)

if (WIN32)
	link_libraries(
		Secur32.lib
	)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
endif()

add_subdirectory(upkg)

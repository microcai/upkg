#cmake_minimum_required(VERSION 2.6)
project(tinyxml2)

file(GLOB tinyxml2_SRC tinyxml2.cpp tinyxml2.h)

add_library(tinyxml2 STATIC ${tinyxml2_SRC})

target_include_directories(tinyxml2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(tinyxml2 PROPERTIES FOLDER "third_party")
set_target_properties(tinyxml2 PROPERTIES POSITION_INDEPENDENT_CODE ON)


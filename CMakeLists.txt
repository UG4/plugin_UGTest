set(pluginName	unit_test)
set(SOURCES		tests/common/util/tst_string_util.cpp)

cmake_minimum_required(VERSION 2.6)

project(UG_PLUGIN_${pluginName})

# include the definitions and dependencies for ug-plugins.
include(${UG_ROOT_CMAKE_PATH}/ug_plugin_includes.cmake)

include_directories(.)

add_executable(ugtest_ugcore ${SOURCES})
target_link_libraries(ugtest_ugcore ug4)
set(pluginName	unit_test)
# I don't think that this is the optimal solution due to the massive sie of files you need to put in this var by hand
# but I didn't find a cleaner solution. Would maybe be with add_subdirectory and EXPORTSOURCES but no documentation so I don't even bother
set(SOURCES
    tests/common/util/tst_string_util.cpp)

cmake_minimum_required(VERSION 2.6)
project(UG_PLUGIN_${pluginName})

include(${UG_ROOT_CMAKE_PATH}/ug_plugin_includes.cmake)

add_executable(ugtest_ugcore ${SOURCES})
target_link_libraries(ugtest_ugcore ug4)
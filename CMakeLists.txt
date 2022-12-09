# Author: Tobias Trautmann
set(pluginName	unit_test)
# I don't think that this is the optimal solution due to the massive sie of files you need to put in this var by hand
# but I didn't find a cleaner solution. Would maybe be with add_subdirectory and EXPORTSOURCES but no documentation so I don't even bother
set(TEST_SOURCES
    tests/common/util/tst_string_util.cpp)

cmake_minimum_required(VERSION 2.6)
project(UG_PLUGIN_${pluginName})

include(${UG_ROOT_CMAKE_PATH}/ug_plugin_includes.cmake)


include_directories(.)


get_property(enabledPlugins GLOBAL PROPERTY ugPluginNames)

# Create a test executable for ugcore.
add_executable(ugtest_ugcore ${TEST_SOURCES})
target_link_libraries(ugtest_ugcore ug4)

# Create test executables for plugins (if included).
foreach(plugin ${enabledPlugins})
	 MESSAGE (STATUS "UGTest checking plugin: " ${plugin})
	
    if(${plugin} STREQUAL UGTest) # These are all activated plugins ???
    	SET(MY_TEST_EXE "ugtest_")
    	STRING(APPEND MY_TEST_EXE ${plugin})
    	# MESSAGE (STATUS "Should create: " ${MY_TEST})
    	STRING(CONCAT MY_CPP_SOURCES ${UG_ROOT_CMAKE_PATH} "/../../plugins/" ${plugin} "/tests/*.cpp")
    	# MESSAGE (STATUS "Checking: " ${MY_CPP_SOURCES})
    	FILE(GLOB_RECURSE MY_TEST_SOURCES ${MY_CPP_SOURCES}) # Recursively find all cpp files.
    	# MESSAGE (STATUS "Checking: " ${MY_TEST_SOURCES})
    	IF (MY_TEST_SOURCES)
    		MESSAGE (STATUS "Creating: " ${MY_TEST_EXE} ${MY_TEST_SOURCES})
    		add_executable("${MY_TEST_EXE}" ${MY_TEST_SOURCES})
			target_link_libraries(${MY_TEST_EXE} ug4)
		ENDIF (MY_TEST_SOURCES)
    endif()
endforeach()
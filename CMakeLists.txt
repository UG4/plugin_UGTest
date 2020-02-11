# Copyright (c) 2020:  G-CSC, Goethe University Frankfurt
# Author: Tobias Trautmann
# 
# This file is part of UG4.
# 
# UG4 is free software: you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License version 3 (as published by the
# Free Software Foundation) with the following additional attribution
# requirements (according to LGPL/GPL v3 Â§7):
# 
# (1) The following notice must be displayed in the Appropriate Legal Notices
# of covered and combined works: "Based on UG4 (www.ug4.org/license)".
# 
# (2) The following notice must be displayed at a prominent place in the
# terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
# 
# (3) The following bibliography is recommended for citation and must be
# preserved in all covered files:
# "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
#   parallel geometric multigrid solver on hierarchically distributed grids.
#   Computing and visualization in science 16, 4 (2013), 151-164"
# "Vogel, A., Reiter, S., Rupp, M., NÃ¤gel, A., and Wittum, G. UG4 -- a novel
#   flexible software system for simulating pde based models on high performance
#   computers. Computing and visualization in science 16, 4 (2013), 165-179"
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.

cmake_minimum_required(VERSION 2.6)

# include the definitions and dependencies for ug-plugins.
include(../../ugcore/cmake/ug_plugin_includes.cmake)
#currently not in use
include(CTest)

# Name of your plugin and sources.
set(pluginName	UGTest)
#set(SOURCES     src/main.cpp)



project(UG_PLUGIN_${pluginName})

#include ctest into this not enough knowlege about it yet

#Get all loaded plugins and their directories
#then iterate over each one of them and check wether they contain tests, if so, add them
get_property(tmp_names GLOBAL PROPERTY ugPluginNames)
get_property(tmp_dirs GLOBAL PROPERTY ugPluginDirs)

set(allfiles "")
set(i 0)#used to get the correct dir for each plugin
foreach(_name ${tmp_names})#iterate over every plugin(name)
    if(${${_name}} STREQUAL "ON")#check wether the plugin is activated
        list(GET tmp_dirs ${i} j)
        if(IS_DIRECTORY ${j}/tests)#check wether it has tests
            file(GLOB files ${j}/tests/*.cpp)
            if(NOT files STREQUAL "")#find all tests and double check
                #add all tests to the list of files to build with
                list(APPEND allfiles ${files})
            else()
                message(${_name} " DOES NOT contain tests!!!")
            endif()
        else()
            message(${_name} " DOES NOT contain tests!!!")
        endif()
    endif()
    #best i++ i've ever seen
    MATH(EXPR i "${i}+1")
endforeach()

#create one executable containing all files
#due to boost.test ug_test takes several arguments, see README.md
add_executable(ug_test    
        src/main.cpp
        src/common_fixtures.cpp
        ${allfiles}
    )
target_link_libraries(ug_test ug4)
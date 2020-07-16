include_directories(.) #Make UGTest.h available for unit tests

#Get all loaded plugins and their directories
get_property(tmp_names GLOBAL PROPERTY ugPluginNames)
get_property(tmp_dirs GLOBAL PROPERTY ugPluginDirs)
set(i 0)#used to get the correct dir for each plugin

#then iterate over each one of them and check wether they contain tests, if so, add them
foreach(_name ${tmp_names})#iterate over every plugin(name)
    if(${_name} STREQUAL "ON")#check wether the plugin is activated
        list(GET tmp_dirs ${i} j)
        if(IS_FILE ${j}/tests/CMakeLists.txt)#check wether it has tests
            include(${j}/tests/CMakeLists.txt)
        endif()
    endif()
    MATH(EXPR i "${i}+1")
endforeach()
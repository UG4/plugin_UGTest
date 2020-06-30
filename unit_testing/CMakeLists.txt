#create unit test executable and make UGTest.h known

#Get all loaded plugins and their directories
get_property(tmp_names GLOBAL PROPERTY ugPluginNames)
get_property(tmp_dirs GLOBAL PROPERTY ugPluginDirs)
set(allfiles "")
set(i 0)#used to get the correct dir for each plugin

#then iterate over each one of them and check wether they contain tests, if so, add them
foreach(_name ${tmp_names})#iterate over every plugin(name)
    if(${_name} STREQUAL "ON")#check wether the plugin is activated
        list(GET tmp_dirs ${i} j)
        if(IS_DIRECTORY ${j}/tests)#check wether it has tests
            file(GLOB_RECURSE files ${j}/tests/*.cpp)
            list(APPEND allfiles ${files})
        endif()
    endif()
    MATH(EXPR i "${i}+1")
endforeach()

include_directories(.)

#create one executable containing all files
#due to boost.test ugtest_unit takes several arguments, see README.md
add_executable(ugtest_unit   
        UGTest.h
        ${allfiles}
    )
target_link_libraries(ugtest_unit ug4)
#create unit test executable
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
add_executable(ugtest_unit   
        ${allfiles}
    )
target_link_libraries(ugtest_unit ug4)
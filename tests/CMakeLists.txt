cmake_minimum_required(VERSION 2.6)
project(ugtest_ugcore)

set(origin /home/tobi/ug4/plugins/UGTest/tests)
set(sources ${origin}/common/util/tst_string_util.cpp)

add_executable(ugtest_ugcore ${sources})
target_link_libraries(ugtest_ugcore ug4)
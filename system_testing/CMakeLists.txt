set(UGTEST_SYSTEM_CONFIG test_config)

add_executable(ugtest_system
    testSuite.cpp
)

target_link_libraries(ugtest_system ug4)
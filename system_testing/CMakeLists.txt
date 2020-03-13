set(UGTEST_SYSTEM_CONFIG test_config)

add_executable(ugtest_system
    testSuite.cpp
    testScripts.cpp
    script_param_parser.cpp
    fixtures/bind_boost_test.cpp
)

target_link_libraries(ugtest_system ug4)
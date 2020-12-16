# This plugin
This plugin mainly consists of three parts:
1. UGTest.h, a Header file to simplify writing unit tests
2. the unit tests for ugcore
3. This documentation about (automated) testing within ug4

You can build your own executable for only your test without needing this plugin.

**Documented here is Boost.Test 1.58, which currently is the included Boost version. It is possible to use Boost 1.72 instead, which provides a lot more features.**

## Enable tests
To enable tests for your plugin, simply create an executable with your tests as source and link it to ug4. This might look something like this:
```
set(TEST_SOURCES
    tests/common/util/tst_string_util.cpp)

get_property(enabledPlugins GLOBAL PROPERTY ugPluginNames)

#create a test executable if the Test plugin is included
foreach(plugin ${enabledPlugins})
    if(${plugin} STREQUAL "UGTest")
        add_executable(ugtest_ugcore ${TEST_SOURCES})
        target_link_libraries(ugtest_ugcore ug4)
    endif()
endforeach()
```
## Running Tests
To run the tests you wrote, run cmake for ug with the desired plugins enabled. After making, you will find your executable in the bin folder.
This executable can take the arguments listed [here](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/runtime-config/reference.html). When running in parallel only one process should log by default. To write logs or reports to fies use the log_sink/report_sink options combined with their respectie _level and _format arguments.

# BOOST.Test 1.58
See the [Boost.Test 1.58 documentation](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/index.html).
## BOOST_AUTO_TEST_SUITE
Defines a new testsuite. You should define one per class.

## [BOOST_AUTO_TEST_CASE](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/test-organization/auto-nullary-test-case.html)
Defines a testcase. Required arguments: testcase name. In newer versions of Boost, you can pass additional parameters to filter tests during test execution.

## [BOOST_AUTO_TEST_CASE_TEMPLATE](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/test-organization/auto-test-case-template.html)
The same as above, except for template functions. To define such a test case, you have to pass test_case_name, formal_type_parameter_name and the collection_of_types.
**Work in progress**

Assuming you're writing a plugin for ug4, you should create a folder in your package called "tests". In this subfolder you put all your tests and needed data. That's basically it, cmake will find these and if you did it right will automatically add your tests to the test executable.

## Checks
There are three levels of checks: WARN, CHECK and REQUIRE.

<table>
    <tr>
        <th>Level</th>
        <th>Error counter</th>
        <th>Test execution</th>
    </tr>
    <tr>   
        <td>WARN</td>
        <td>not affected</td>
        <td>continues</td>
    </tr>
    <tr>   
        <td>CHECK</td>
        <td>increases</td>
        <td>continues</td>
    </tr>
    <tr>   
        <td>REQUIRE</td>
        <td>increases</td>
        <td>aborts the testcase</td>
    </tr>
</table>

All functions take a statment as required argument as well as afailure message as optional argument (only most likely used listed):
+ `BOOST_<level>(predicate)`: Checks predicate and acts depending on level. It is recommended to use more specific tests when possible.
+ `BOOST_<level>_[GE,LE,GT,LT,NE](left, right)`: greaterequal, lessequal ... with output of variables
+ `BOOST_<level>_EQUAL(left, right)`: checks wether values are equal. **Do not use this for float comparison, instead use `BOOST_<level>_CLOSE`**
+ `BOOST_<level>_CLOSE(left, right, tolerance)`: checks wether left and right are within tolerance percent of each other
+ `BOOST_IS_DEFINED(SYMBOL)`: checks wether preprocessor SYMBOL is defined, doesn't log on its own
+ `BOOST_<level>_THROW(expression, exception_type)`: checks wether exception_type is throw when executing expression
+ `BOOST_<level>_NO_THROW(expression)` catches any exception thrown by expression but still logs it

# Setup tests for your plugin
Assuming you're writing a package for ug4, you should create a folder in your package called "tests". In this folder you copy your source code tree so that for every cpp ther is an according tst_<orginal_name>.cpp. Those testfile should include UGTest.h. You should write at least one test per function regarding correct handling of edge cases, errors and correctness of output. Name your testsuite after the filename and the testcases in it <filename>_<function_name>[_<additional_info>]. Organizing tests this way makes them easier to find and relocate if needed.
Boost.Test needs `BOOST_TEST_MODULE` to be set to generate it's own main. You can define `BOOST_TEST_MODULE` in the tests you write. It defaults to "ug_plugin_tests".

## CMakeLists
To include UGTest.h, you need to include it in your CMakeLists.txt via `include_directories(../UGTest)`.
You should create a seperate executable for your tests by adding:
```
add_executable(ugtest_${pluginName} ${SOURCES})
target_link_libraries(ugtest_${pluginName} ug4)
```
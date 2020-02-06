# BOOST.Test 1.58 for UG4
Boost.Test supplies several utilities for testing, especially unit testing.
See the [Boost.Test 1.58](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/index.html) documentation.

## How to write tests
Assuming you're writing a package for ug4, you should create a folder in your package called "tests". In this subfolder you put all your tests and needed data. That's basically it, cmake will find these and if you did it right will automatically add your tests to the test executable. Things you need to know and should be aware of are listed in this readme.
You should write tests for all (major) functions regarding correct handling of edge cases, errors and correctness of output.

## Automated testing
Boost.Test needs `BOOST_TEST_MODULE` to be set to generate it's own main. Once declared Boost will search for `BOOST_AUTO_TEST_SUITE` and underlying `BOOST_AUTO_TEST_CASE` / `BOST_AUTO_TEST_CASE_TEMPLATE`.

**Do not define `BOOST_TEST_MODULE` in the tests you write for your package!**

### BOOST_AUTO_TEST_SUITE
Defines a new testsuite. You should define one per class. Name them `<packagename>_<class>`

### [BOOST_AUTO_TEST_CASE](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/test-organization/auto-nullary-test-case.html)
Defines a testcase. Required arguments: testcase name. In newer versions of Boost, you can pass additional parameters to filter tests during execution.
Name them `<packagename>_<class>_<description>`

# TODO: Understand this better!!
### [BOOST_AUTO_TEST_CASE_TEMPLATE](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/test-organization/auto-test-case-template.html)
The same as above, except for template functions. To define such a test case, you have to pass test_case_name, formal_type_parameter_name and the collection_of_types
Name them `<packagename>_<class>_<description>_template`
```c++
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

typedef boost::mpl::list<int, long, unsigned char> test_types;

BOOST_AUTO_TEST_SUITE(PluginTests_templates)
    BOOST_AUTO_TEST_CASE_TEMPLATE(PluginTests_templates_demo_template, T, test_types)
    {
        BOOST_CHECK(sizeof(T) == (unsigned)4);
    }
BOOST_AUTO_TEST_SUITE_END()
```

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
        <td>not affected </td>
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

All functions take a statment as required argument as well as afailure message as optional argument
+ `BOOST_<level>(predicate)`: Checks predicate and acts depending on level. It is recommended to use more specific tests when possible.
+ `BOOST_<level>_[GE,LE,GT,LT,NE](left, right)`: greaterequal, lessequal ... with output of variables
+ `BOOST_<level>_EQUAL(left, right)`: checks wether values are equal. **Do not use this for float comparison, instead use `BOOST_<level>_CLOSE`**
+ `BOOST_<level>_CLOSE(left, right, tolerance)`: checks wether left and right are within tolerance percent of each other
+ `BOOST_IS_DEFINED(SYMBOL)`: checks wether preprocessor SYMBOL is defined, doesn't log on its own
+ `BOOST_<level>_THROW(expression, exception_type)`: checks wether exception_type is throw when executing expression
+ `BOOST_<level>_NO_THROW(expression)` catches any exception thrown by expression but still logs it

## [Fixtures](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/fixture.html)
You probably have similar starting conditions for some testcases you write. These can be grouped in structs that boost can use as a fixture. a fixture is set up before each test case and torn down after that testcase.

There are several options where to use fixtures:

To "rebuild" your fixture after every test case, you need to call BOOST_FIXTURE_TEST_CASE instead of BOOST_TEST_CASE. BOOST_FIXTURE_TEST_CASE takes the name of your fixture as an additional argument:
```c++
#include <boost/test/unit_test.hpp>

struct F {
    F()  { BOOST_TEST_MESSAGE( "setup fixture" ); }
    ~F() { BOOST_TEST_MESSAGE( "teardown fixture" ); }

};

BOOST_FIXTURE_TEST_CASE( PluginTests_fixture_example, F )
{
    //your test
}
```

This package also provides Testsuite wide fixtures, that you can use like this:
```c++
#include "../../PluginTests/src/common_fixtures.cpp"
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(plugin_test_class, FixtureProvidedByCommonFixtures)
    //your tests
BOOST_FIXTURE_TEST_SUITE_END()
```
Ther is an option to make fixtures globally available, but I highly recommend not to use them, because they are automatically used for every testsuite, not just your own

## Logging
You can set the [logging level](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/runtime-config/reference.html#) of Boost to `all`, `test_suite`, `message`, `warning`, `error` (standard), `cpp_exception`, `system_error` or `fatal_error`. The parameter for this is `log_level`.
Besides the output to stdout, Boost.Test is able to produce xml output, which is readable by [Cobertura](https://cobertura.github.io/cobertura/), a plugin for jenkins. To do so, pass `-log_format xml` to your test executable.

**Note:** Altough it is possible to set the logging level within you tests, it is highly recommend not to do so, since it overrides the logging level provided at execution time.

+ `BOOST_ERROR(msg)` increases the error counter and shows msg
+ `BOOST_MESSAGE` writes to stdout, doesn't get logged as long as log_level isn't below "message"
+ `BOOST_TEST_CHECKPOINT` is usefull for marking important steps. Last checkpoint will be logged in case of error.

## Example how a test ca look like for your package
```c++
#include <boost/test/unit_test.hpp>
#include "../../PluginTests/src/common_fixtures.cpp" //Fixtures provided by PluginTests, path doesn't change

BOOST_AUTO_TEST_SUITE(PluginTests_simple)

    BOOST_AUTO_TEST_CASE(PluginTests_simple_checks)
    {
        BOOST_WARN(false);
        BOOST_CHECK(false);
        BOOST_REQUIRE(false);
    }
    BOOST_FIXTURE_TEST_CASE(PluginTests_simple_fixture, F)
    {
        BOOST_WARN_EQUAL(1,2);
        BOOST_WARN_GT(1,2);
    }

BOOST_AUTO_TEST_SUITE_END()     
```
You can rename the example_tests folder of this plugin to tests to see how this plugin works and what it should look like.

## Running Tests
to run the tests you wrote, run cmake for ug with the desired plugins and PluginTests enabled. After making, you will find the executable ug_tests in the bin/plugins folder.
This executable can take the arguments listed [here](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/runtime-config/reference.html).


## This plugin
This plugin provides two important parts needed for testing:
1. the cmake file collecting all the tests and merging them into one executable
2. the unit tests for ugcore
This means that you can build your own executable for only your test without needing this plugin.
To do so, define BOOST_TEST_MODULE and add all your tests into one executable in your cmake file.
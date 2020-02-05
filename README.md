# BOOST.Test 1.58
Boost.Test supplies several utilities for testing, especially unit testing.
See the [Boost.Test 1.58](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/index.html) documentation.

## How to write tests for UG4
Assuming you're writing a package for ug4, you should create a folder in your package called "tests". In this subfolder you put all your tests and needed data. That's basically it, cmake will findthese and if you did it right will automatically add your tests to the unit_test executable. Things you need to know and should be aware of are listed in this readme.
You should write tests for all (major) functions regarding correct handling of edge cases, errors and correctness of output.

## Automated testing
Boost.Test needs `BOOST_TEST_MODULE` to be set to generate it's own main. Once declared Boost will search for `BOOST_AUTO_TEST_SUITE` and underlying `BOOST_AUTO_TEST_CASE` / `BOST_AUTO_TEST_CASE_TEMPLATE`.

**Do not define `BOOST_TEST_MODULE` in the tests you write for your package!**

### BOOST_AUTO_TEST_SUITE
Defines a new testsuite. You should define one per class with an indicator which package it belongs to.

### [BOOST_AUTO_TEST_CASE](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/test-organization/auto-nullary-test-case.html)
Defines a testcase. Required arguments: testcase name. In newer versions of Boost, you can pass additional parameters to filter tests during execution.


### [BOOST_AUTO_TEST_CASE_TEMPLATE](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/test-organization/auto-test-case-template.html)
The same as above, except for template functions. To define such a tes case, you have to pass

## Checks
There are three levels of checks: WARN, CHECK and REQUIRE.

Level   | Error counter | Test execution
WARN    | not affected  | continues
CHECK   | increases     | continues
REQUIRE | increases     | aborts

All functions take a statment as required argument as well as afailure message as optional argument
+ `BOOST_<level>(predicate)`: Checks predicate and acts depending on level.
+ `BOOST_<level>_[GE,LE,GT,LT,NE](left, right)`: greaterequal, lessequal ... with output of variables
+ `BOOST_<level>_EQUAL(left, right)`: checks wether values are equal. **Do not use this for float comparison, instead use `BOOST_<level>_CLOSE`**
+ `BOOST_<level>_CLOSE(left, right, tolerance)`: checks wether left and right are within tolerance percent of each other
+ `BOOST_IS_DEFINED(SYMBOL)`: checks wether preprocessor SYMBOL is defined, doesn't log on its own
+ `BOOST_<level>_THROW(expression, exception_type)`: checks wether exception_type is throw when executing expression
+ `BOOST_<level>_NO_THROW(expression)` catches any exception thrown by expression but still logs it

## Logging
You can set the [logging level](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/runtime-config/reference.html#) of Boost to `all`, `test_suite`, `message`, `warning`, `error` (standard), `cpp_exception`, `system_error` or `fatal_error`. The parameter for this is `log_level`.
Besides the output to stdout, Boost.Test is able to produce xml output, which is readable by [Cobertura](https://cobertura.github.io/cobertura/), a plugin for jenkins. To do so, pass `-log_format xml` to your test executable.

**Note:** Altough it is possible to set the logging level within you tests, it is highly recommend not to do so, since it overrides the logging level provided at execution time.

+ `BOOST_ERROR(msg)` increases the error counter and shows msg
+ `BOOST_MESSAGE` writes to stdout, doesn't get logged as long as log_level isn't below "message"
+ `BOOST_TEST_CHECKPOINT` is usefull for marking important steps. Last checkpoint will be logged in case of error.

## Example
```c++
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(packagetests)

    BOOST_AUTO_TEST_CASE(testcase_1,
        *utf::label("label") *utf::disabled() * utf::description("description")){
        BOOST_WARN(false, "false alarm");
        BOOST_CHECK(false, "I just increased your error count");
        BOOST_MESSAGE("if the next test fails, testing will abort");
        BOOST_REQUIRE(BOOST_IS_DEFINED(BOOST_TEST_MODULE));
    }

BOOST_AUTO_TEST_SUITE_END()
```

## Running Tests
to run the tests you wrote, run cmake for ug with the desired plugins and plugin_tests enabled. After making, you will find the executable ug_tests in the bin/plugins folder.
This executable takes the following arguments: ...

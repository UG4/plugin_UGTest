# BOOST.Test 1.58 for UG4
Migrated from the ug4 app unit_test by Martin Scherer.
Please look at the wiki which is more up to date.
Boost.Test supplies several utilities for testing, especially unit testing.
See the [Boost.Test 1.58](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/index.html) documentation.

## How to write tests
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

All functions take a statment as required argument as well as afailure message as optional argument
+ `BOOST_<level>(predicate)`: Checks predicate and acts depending on level. It is recommended to use more specific tests when possible.
+ `BOOST_<level>_[GE,LE,GT,LT,NE](left, right)`: greaterequal, lessequal ... with output of variables
+ `BOOST_<level>_EQUAL(left, right)`: checks wether values are equal. **Do not use this for float comparison, instead use `BOOST_<level>_CLOSE`**
+ `BOOST_<level>_CLOSE(left, right, tolerance)`: checks wether left and right are within tolerance percent of each other
+ `BOOST_IS_DEFINED(SYMBOL)`: checks wether preprocessor SYMBOL is defined, doesn't log on its own
+ `BOOST_<level>_THROW(expression, exception_type)`: checks wether exception_type is throw when executing expression
+ `BOOST_<level>_NO_THROW(expression)` catches any exception thrown by expression but still logs it

## Running Tests
to run the tests you wrote, run cmake for ug with the desired plugins and PluginTests enabled. After making, you will find the executable ug_tests in the bin/plugins folder.
This executable can take the arguments listed [here](https://www.boost.org/doc/libs/1_58_0/libs/test/doc/html/utf/user-guide/runtime-config/reference.html). When running in parallel only one process should log by default. To write logs or reports to fies use the log_sink/report_sink options combined with their respectie _level and _format arguments.

## Jenkins
The current Jenkins Configuration to run tests is stored in config.xml. This config by default lies in /var/lib/jenkins/jobs/job-name/config.xml. 

## This plugin
This plugin provides two important parts needed for testing:
1. the cmake file collecting all the tests and merging them into one executable
2. the unit tests for ugcore
This means that you can build your own executable for only your test without needing this plugin.
To do so, define BOOST_TEST_MODULE and add all your tests into one executable in your cmake file.

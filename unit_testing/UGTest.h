#ifndef UGTEST_H
#define UGTEST_H

#define BOOST_TEST_MODULE UG_plugin_tests
#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/parameterized_test.hpp>

#include <cstdlib>     /* atexit */
#include "ug.h"

using namespace boost::unit_test;
using namespace ug;

#ifdef UG_PARALLEL
#include <boost/mpl/list.hpp>
#include "pcl/pcl_base.h"
    // exit handler function to call UGFinalize() in case no tests have been run.
    // this is neccessary because the global fixture is not used in case of empty testsuite
    void exitHandler() {
        ug::UGFinalize();
    }
#endif


//Global init for ug run at the beginning and end of each testsuite
struct UGTestInit
{
    UGTestInit()
    {
        ug::UGInit(&framework::master_test_suite().argc, &framework::master_test_suite().argv);
        // register exit handler
        #ifdef UG_PARALLEL
            atexit(exitHandler);
        #endif
    }
    //Global teardown
    ~UGTestInit()  {
        ug::UGFinalize();
    }
};
#endif

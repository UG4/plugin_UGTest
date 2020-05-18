#ifndef UGTEST_H
#define UGTEST_H

#define BOOST_TEST_MODULE UG_plugin_tests
#include <boost/test/included/unit_test.hpp>
#include <cstdlib>     /* atexit */
#include "ug.h"

using namespace boost::unit_test;
using namespace ug;

#ifdef UG_PARALLEL
#include "pcl/pcl_base.h"
    // exit handler function to call UGFinalize() in case no tests have been run.
    // this is neccessary because the global fixture is not used in case of empty testsuite
    void exitHandler() {
        ug::UGFinalize();
    }
#endif

struct UGbase
{
    UGbase()
    {
        ug::UGInit(&framework::master_test_suite().argc, &framework::master_test_suite().argv);
        // register exit handler
        #ifdef UG_PARALLEL
            atexit(exitHandler);
        #endif
    }
    //Global teardown
    ~UGbase()  {
        ug::UGFinalize();
    }
};

BOOST_GLOBAL_FIXTURE(UGbase);
#endif
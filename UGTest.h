#ifndef UGTEST_H
#define UGTEST_H

#define BOOST_TEST_MODULE ug_plugin_tests
#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/parameterized_test.hpp>

#include <cstdlib>     /* atexit */
#include "ug.h"

namespace utf = boost::unit_test;

#ifdef UG_PARALLEL
#include <boost/mpl/list.hpp>
#include "pcl/pcl_base.h"
    /**
     * Terminator for parallel execution
     */
    void exitHandler() {
        ug::UGFinalize();
    }
#endif

/**
 * \defgroup UnitTests
 * \addtogroup UnitTests
 * \brief The global fixture for all Unit tests
 * This struct is used as global fixture for all Unit test applications,
 * meaning that before every start of a master testsuite, its constructor is called and after
 * every master testsuite execution, its deconstructor is called
 */
struct UGTestInit
{
    /**
     * initialzes ug for further use in tests using the paramteters given to the executable
     */
    UGTestInit()
    {
        ug::UGInit(&utf::framework::master_test_suite().argc, &utf::framework::master_test_suite().argv);
        // register exit handler
        #ifdef UG_PARALLEL
            atexit(exitHandler);
        #endif
    }
    /**
     * calls UGFinalize for the used ug instance
     */
    ~UGTestInit()  {
        ug::UGFinalize();
    }
};
#endif

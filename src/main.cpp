#define BOOST_TEST_MODULE UG_plugin_tests
#include <boost/test/included/unit_test.hpp>
#include <boost/test/framework.hpp> 
#include <cstdlib>     /* atexit */

#include "ug.h"

using namespace boost::unit_test;

#ifdef UG_PARALLEL
#include "pcl/pcl_base.h"
    // exit handler function to call UGFinalize() in case no tests have been run.
    // this is neccessary because the global fixture is not used in case of empty testsuite
    void exitHandler() {
        ug::UGFinalize();
    }
#endif

    /**
 * \author  Tobias Trautmann <Tobias.Trautmann@gcsc.uni-frankfurt.de>
 * \note    most of the code seen here originates from Martin Scherer as seen in unit_tests.
 * \brief   The global fixture. In older versions of BOST.Test, this was the function init_unit_test_suite. 
 * \bug     Currently, this throws LoadPlugins fails (called by UGInit), but still execues all tests
 * \todo    It is unknown, if pcl works
 * \todo    dependecies need to be updated
 * \note    newer versions of Boost allow tagging, descriptions and  other usefull stuff to organise tests.
 *          The constructor of this struct is equivalent to a main function for the ug_test executable. It calls UGInit and sets up logging.
 */

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
        //ofs.close();
    }
    //std::ofstream ofs;
};

BOOST_GLOBAL_FIXTURE(UGbase);
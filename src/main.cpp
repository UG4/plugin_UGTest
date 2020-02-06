#define BOOST_TEST_MODULE UG_plugin_tests
#include <boost/test/included/unit_test.hpp>
#include <boost/test/framework.hpp>
#include <fstream>
#include <cstdlib>     /* atexit */

#include "bridge/bridge.h"
#include "common/log.h"
#include "common/error.h"

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
 * \bug     Currently, this does not work properly, bc LoadPlugins fails (called by UGInit).
 * \todo    It is unknown, if pcl works
 * \todo    dependecies need to be updated
 * \note    newer versions of Boost allow tagging, descriptions and  other usefull stuff to organise tests.
 *          The constructor of this struct is equivalent to a main function for the ug_test executable. It calls UGInit and sets up logging.
 */
struct UGbase
{
    UGbase()
    {
        std::cout << "global setup\n";
        //UGInit doesn't take zero previous versions didn't pass args either
        ug::UGInit(&framework::master_test_suite().argc, &framework::master_test_suite().argv);
        //Is this necessary?
        ug::GetLogAssistant().enable_terminal_output(false);
        /*
        * in parallel we log for every process, as checks eg. in scripts may fail only
        * on one specific processor which could not be detected then.
        */
        #ifdef UG_PARALLEL
            int numProcs = pcl::NumProcs();
        #else
            int numProcs = 1;
        #endif
        std::stringstream ss;
        ss << "ug_test_numprocs_" << numProcs;
        #ifdef UG_PARALLEL
            ss << "_rank_" << pcl::ProcRank();
        #endif

        /**
         * \todo make it compatible with jenkins
         * override boosts logsink filename to append the proc rank.
         * This is necessary because else all processes will write to one file, which
         * causes malformed logs.
         */
        std::ofstream* ofs = dynamic_cast<std::ofstream*>(runtime_config::log_sink());
        // log_sink file given, so reopen it with proper suffix
        if(ofs) {
            if(ofs->is_open()){
                ofs->close();
            }
            const char* filename = ss.str().append(".xml").c_str();
            ofs->open(filename);
            if(!ofs->good()) {
                ss.str("");
                ss << "could not open boost test logfile: " << filename;
                throw boost::framework::setup_error(ss.str());
            }
        }
        ss << ".log";
        if(not ug::GetLogAssistant().enable_file_output(true, ss.str().c_str())) {
            std::string f = ss.str();
            ss.str("could not enable file output for file: ");
            ss << f;
            throw boost::framework::setup_error(ss.str());
        }

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
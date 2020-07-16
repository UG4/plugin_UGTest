#define BOOST_TEST_MODULE Systemtests
/**
 * master test suite for all sub test suites.
 * it registers a report redirector to the log file "test_results.xml" to
 * the boost testing framework
 */
#include <fstream>
#include <cstdlib>     /* atexit */
#include <filesystem>  // from C++ 17 on


#include <boost/test/included/unit_test.hpp>
#include <boost/test/framework.hpp>

#include "common/util/path_provider.h"
#include "common/util/parameter_parsing.h"
#include "bindings/lua/lua_util.h"
#include "bridge/bridge.h"
#include "common/log.h"
#include "common/error.h"

#include "ug.h"

#include "testScripts.h"
#include "fixtures/bind_boost_test.h"
#include "script_param_parser.h"

using namespace std;
using namespace ug;
using namespace boost::framework;
using namespace boost::unit_test;


// global var to indicate whether UGFinalize() has been called already
bool finalized = false;

#ifdef UG_PARALLEL
#include "pcl/pcl_base.h"

// exit handler function to call UGFinalize() in case no tests have been run.
// this is neccessary because the global fixture is not used in case of empty testsuite (
void exitHandler() {
	if(not finalized) {
		ug::UGFinalize();
		finalized = true;
	}
}
#endif

struct UGBase{
    UGBase(){
        typedef std::vector<std::string> TFilenameList;
	    TFilenameList collection_of_filenames;

///////////////////////////////////////////////////////////////////////////////
// init ug and register default lua bridge
        try {
            ug::UGInit(&master_test_suite().argc, &master_test_suite().argv);
            ug::script::RegisterDefaultLuaBridge(&ug::bridge::GetUGRegistry());
            // register boost test tool wrappers (check etc.)
            RegisterBoostTestFunctions();
        }
        catch (ug::UGError&e) {
            // let boost testing framework handle it
            throw(framework::setup_error(e.get_stacktrace()));
        }

///////////////////////////////////////////////////////////////////////////////
// logging
        ug::GetLogAssistant().enable_terminal_output(false);
        if(ug::FindParam("--console", master_test_suite().argc, master_test_suite().argv)) {
            ug::GetLogAssistant().enable_terminal_output(true);
        }
        int numProcs = 1;
/*
 * in parallel we log for every process, as checks eg. in scripts may fail only
 * on one specific processor which could not be detected then.
 */
#ifdef UG_PARALLEL
	numProcs = pcl::NumProcs();
#endif
	std::stringstream ss;
	ss << "ug_test_numprocs_" << numProcs;
#ifdef UG_PARALLEL
	ss << "_rank_" << pcl::ProcRank();
#endif

        /**
         * override boosts logsink filename to append the proc rank.
         * This is necessary because else all processes will write to one file, which
         * causes malformed logs.
         */
        std::ofstream* ofs = dynamic_cast<std::ofstream*>(
                runtime_config::log_sink());
        // log_sink file given, so reopen it with proper suffix
        if(ofs) {
            if(ofs->is_open())
                ofs->close();
            const char* filename = ss.str().append(".xml").c_str();
            ofs->open(filename);
            if(!ofs->good()) {
                ss.str("");
                ss << "could not open boost test logfile: " << filename;
                throw framework::setup_error(ss.str());
            }
        }

        ss << ".log";
        if(not ug::GetLogAssistant().enable_file_output(true, ss.str().c_str())) {
            std::string f = ss.str();
            ss.str("could not enable file output for file: ");
            ss << f;
            throw framework::setup_error(ss.str());
        }


        // Bootstrap all apps
        filesystem::path apps_path = ug::PathProvider::get_path(ug::ROOT_PATH).c_str();
        apps_path.append("apps");

        // Iterate over all directories in apps folder.
        for(auto& app_entry : filesystem::directory_iterator(apps_path))
        {
			filesystem::path test_directory_path = app_entry.path();
			test_directory_path.append("tests");

			try
			{
				// Add all .xml files to collection_of_filenames.
                for(auto& test_entry : filesystem::directory_iterator(test_directory_path))
				{
					std::string test_path = test_entry.path().string();
					if(test_path.substr(test_path.length() - 4) == ".xml") // check for xml files and add them
						collection_of_filenames.push_back(test_path);
				}
            }
			// If tests folder does not exist.
			catch(const filesystem::filesystem_error& e)
			{
				//TODO how to handle?
				//For now: ignore.
			}
        }
        // Iterate list of files;
        for (TFilenameList::iterator iter = collection_of_filenames.begin(); iter != collection_of_filenames.end(); ++iter)
        {
            std::cerr << "TestSuite: Using " << *iter << std::endl;
            script_params_filename = iter->c_str();
            try {
                ScriptParamsReader reader;
                reader.parse_file(script_params_filename);
                initScriptTestSuites(reader.getScriptCalls());
            } catch(ug::UGError& e) {
                // let boost testing framework handle it
                throw(framework::setup_error(e.get_stacktrace()));
            }

	}

#ifdef UG_PARALLEL
	// register exit handler
	atexit(exitHandler);
#endif
    }

    ~UGBase() {
		if(not finalized) {
			ug::UGFinalize();
			finalized = true;
		}
	}
};

BOOST_GLOBAL_FIXTURE(UGBase);
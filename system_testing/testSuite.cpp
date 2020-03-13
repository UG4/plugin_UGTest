#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include <boost/test/framework.hpp>
#include <fstream>
#include <cstdlib>     /* atexit */

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

#include <dirent.h>

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

//is this necessary?
void setup_logging(int argc, char *argv[])
{

  ///////////////////////////////////////////////////////////////////////////////
  // logging
  ug::GetLogAssistant().enable_terminal_output(false);
  if (ug::FindParam("--console", argc, argv))
  {
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
  std::ofstream *ofs = dynamic_cast<std::ofstream *>(
      runtime_config::log_sink());
  // log_sink file given, so reopen it with proper suffix
  if (ofs)
  {
    if (ofs->is_open())
      ofs->close();
    const char *filename = ss.str().append(".xml").c_str();
    ofs->open(filename);
    if (!ofs->good())
    {
      ss.str("");
      ss << "could not open boost test logfile: " << filename;
      throw framework::setup_error(ss.str());
    }
  }

  ss << ".log";
  if (not ug::GetLogAssistant().enable_file_output(true, ss.str().c_str()))
  {
    std::string f = ss.str();
    ss.str("could not enable file output for file: ");
    ss << f;
    throw framework::setup_error(ss.str());
  }
}

// used to finalize the ug environment, if there are tests to run
struct ug_global_fixture {
  ug_global_fixture(){
    ug::GetLogAssistant().enable_terminal_output(false);
    if (ug::FindParam("--console", master_test_suite().argc, master_test_suite().argv))
    {
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
      UGInit(&master_test_suite().argc, &master_test_suite().argv);
      ug::script::RegisterDefaultLuaBridge(&ug::bridge::GetUGRegistry());
      // register boost test tool wrappers (check etc.)
      RegisterBoostTestFunctions();
      setup_logging(master_test_suite().argc, master_test_suite().argv);

      //find system test configs, parse and "execute them"
      ScriptParamsReader reader;
      DIR *dir = opendir(PathProvider::get_path(PathTypes::APPS_PATH).c_str());
      struct dirent *files;
      while (files = readdir(dir))
      {
        //this is as ugly as possible
        string configpath = PathProvider::get_path(PathTypes::APPS_PATH) + "/" + files->d_name + string("/tests/config.xml");
        try
        {
          reader.parse_file(configpath);
          initScriptTestSuites(reader.getScriptCalls());
        }
        catch (...)
        {
        }
    }
    closedir(dir);

#ifdef UG_PARALLEL
    // register exit handler
    atexit(exitHandler);
#endif
  }

	~ug_global_fixture() {
		if(not finalized) {
			ug::UGFinalize();
			finalized = true;
		}
	}
};

BOOST_GLOBAL_FIXTURE(ug_global_fixture);


void debug(string config){
  cout << config << " : " << endl;
  ifstream cfgfile (config);
  if(cfgfile){
    while(cfgfile){
      string line;
      getline(cfgfile, line);
      cout << line << endl;
    }
  }else{
    cout << config << " does not exist" <<endl;
  }
  cfgfile.close();
}
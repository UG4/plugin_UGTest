#include <fstream>
#include <iostream>
#include <filesystem>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/framework.hpp>
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
// used to finalize the ug environment, if there are tests to run
struct ug_global_fixture {
	~ug_global_fixture() {
		if(not finalized) {
			ug::UGFinalize();
			finalized = true;
		}
	}
};

void find_configs() {
  ScriptParamsReader reader;
  path p = path(getenv("UG4_ROOT")) / "apps";
  directory_iterator end;
  for(auto itr : directory_iterator(p)){
      path config (itr.path() / "tests" / "config.xml");
      if(exists(config)){
        reader.parse_file(config);
        initScriptTestSuites(reader.getScriptCalls());
      }
  }
}

test_suite* init_unit_test_suite(int argc, char* argv[]) {
  ug::UGInit(&argc, &argv);
	ug::script::RegisterDefaultLuaBridge(&ug::bridge::GetUGRegistry());
	// register boost test tool wrappers (check etc.)
	RegisterBoostTestFunctions();

  find_configs();
	
  #ifdef UG_PARALLEL
    // register exit handler
    atexit(exitHandler);
  #endif

	return 0;
}
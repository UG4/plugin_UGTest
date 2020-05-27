#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include <boost/test/framework.hpp>
#include <fstream>

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

struct UGBase{
    UGBase(){
        UGInit(&master_test_suite().argc, &master_test_suite().argv);
        ug::script::RegisterDefaultLuaBridge(&ug::bridge::GetUGRegistry());
        // register boost test tool wrappers (check etc.)
        RegisterBoostTestFunctions();

        //find system test configs, parse and "execute them"
        ScriptParamsReader reader;
        DIR *dir = opendir(PathProvider::get_path(PathTypes::APPS_PATH).c_str());
        struct dirent *files;
        while (files = readdir(dir))
        {
            //this is as ugly as possible i dont know if posix is actually the right choice but hey
            string configpath = PathProvider::get_path(PathTypes::APPS_PATH) + "/" + files->d_name + string("/tests/config.xml");
            if(access( configpath.c_str(), F_OK ) != -1 ){
                reader.parse_file(configpath);
                initScriptTestSuites(reader.getScriptCalls());
            }
        }
        closedir(dir);
    }
    ~UGBase(){
        ug::UGFinalize();
    }
};

BOOST_GLOBAL_FIXTURE(UGBase);

BOOST_AUTO_TEST_SUITE(System)
    BOOST_AUTO_TEST_CASE(all_lua){
        
    }
BOOST_AUTO_TEST_SUITE_END();

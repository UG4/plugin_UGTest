/**
 * testScripts.cpp
 *
 *  Created on: 18.10.2010
 *      Author: Martin Scherer
 */
#include "testScripts.h"

#include <boost/test/unit_test.hpp>
#include <boost/archive/iterators/mb_from_wchar.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/archive/iterators/xml_escape.hpp>

#include <string>
#include <vector>
#include <stdexcept>

#include "bindings/lua/lua_util.h"
#include "common/error.h"
#include "common/util/parameter_parsing.h"
#ifdef UG_PARALLEL
#include "pcl/pcl.h"
#endif

#include "fixtures/UGScriptingEnvFixture.h"
#include "script_param_parser.h"

namespace {
using namespace std;
using namespace ug::script;
using ug::UGError;

using namespace boost::unit_test;
using namespace boost::test_tools;

// first defined here:
// ugbase/bindings/lua/bindings_lua.cpp
std::string PrintUGErrorTraceback(UGError &err)
{
	const static char* errSymb = " % ";
	stringstream ss;
//	header
	ss <<errSymb<<"  Error traceback (innermost first): \n";

//	padding to insert
	std::string pad(errSymb); pad.append("     ");

//	print each message
	for(size_t i=0;i<err.num_msg();++i)
	{
	//	get copy of original string
		std::string msg = err.get_msg(i);

	//	add paddings
		std::string::size_type pos = 0;
		while (1) {
		    pos = msg.find('\n', pos);
		    if (pos == std::string::npos) break;
		    pos++;
		    msg.insert(pos, pad);
		}

	//	write message
		ss <<errSymb<<std::setw(3)<<i<<": "<<msg<<endl;

	//	write file and line
		ss <<pad << "[at "<<err.get_file(i)<<", line "<<err.get_line(i)<<"]\n";
	}
	return ss.str();
}

/**
 * informs the utf about the occured exception.
 */
void handleException(UGError& e, const string& caller) {
	const string& failing_file = e.get_file(0).compare(" -- no file -- ")
			? e.get_file(0): caller;

	// get stack trace
	string stack = PrintUGErrorTraceback(e);

	// filter xml meta chars out of stacktrace string
	typedef boost::archive::iterators::xml_escape<const char*> xml_escape_iter;
	std::stringstream tmp;
	std::copy(xml_escape_iter(stack.c_str()),
			xml_escape_iter(stack.c_str() + stack.size()),
			ostream_iterator<char>(tmp));

	tt_detail::check_impl(
			false,
			lazy_ostream::instance() << tmp.str(),
			// use file and line at top of stack (if not set to "no file")
			failing_file, e.get_line(0),
			tt_detail::CHECK,
			tt_detail::CHECK_MSG, 0);
}

/**
 * should be used to parse and run lua test scripts. It handles LuaError exceptions
 * and passes them to the boost framework with file and line information.
 * @param filename to script
 */
void loadScriptWrapper(const script_call& c) {
	UG_ASSERT(not c.path.empty(), "script call has empty path!")
	const char* filename = c.path.c_str();

	lua_State* L = GetDefaultLuaState();
	lua_getglobal(L, "ugargc");

	// push argv table to top of the stack
	lua_getglobal(L, "ugargv");
	int ugargc = lua_tonumber(L, -2);

	// set script call parameters to ugargv
	for(map<string, string>::const_iterator i = c.params.begin();
			i != c.params.end(); ++i) {

		const string& value = (*i).second;

		// push the index to the table
		lua_pushnumber(L, ++ugargc);
		// push the value to the table
		lua_pushstring(L, (*i).first.c_str());
		// create the entry
		lua_settable(L, -3);

		// if value is not empty, this is a key value parameter -> add value to ugargv
		if(!value.empty()) {
			// push the index to the table
			lua_pushnumber(L, ++ugargc);
			// push the value to the table
			lua_pushstring(L, (*i).second.c_str());
			// create the entry
			lua_settable(L, -3);
		}
	}
	// set the tables name
	lua_setglobal(L, "ugargv");

	lua_pushnumber(L, ugargc);
	lua_setglobal(L, "ugargc");

	try {
		UG_LOG("################################################################################\n");
		UG_LOG("#\tOutput of " << c.get_name() << endl);
		// todo consider using LoadUGScript, since this is also called in serial mode
		BOOST_CHECK_MESSAGE(LoadUGScript_Parallel(filename),
				"Execute script '" << filename << "'");

		UG_LOG("################################################################################\n");
		UG_LOG("#\tEnd of output of " << c.get_name() << endl)
	} catch (LuaError& e) {
		handleException(e, filename);
	} catch(SoftAbort& err){
		UG_LOG("Execution of test-script aborted!\n")
		handleException(err, filename);
	} catch(UGError& e) {
		handleException(e, filename);
	} catch (...) {
		tt_detail::check_impl(
				false,
				lazy_ostream::instance() << "Unknown exception occurred while"
				" parsing script.",
				filename, 1,
				tt_detail::CHECK,
				tt_detail::CHECK_MSG, 0);
	}
}

/**
 * little functor to create a clean lua scripting environment for
 * each script test case with UGScriptingEnvFixture.
 * Stores a copy of script_call
 */
struct ug_lua_test_case_functor
{
	ug_lua_test_case_functor(const script_call&c) : m_call_params(c) {}

	void operator()() {
		// the instance will be destroyed after the script has been run
		UGScriptingEnvFixture instance;
		loadScriptWrapper(m_call_params);
	}

	const script_call m_call_params;
};

test_case* makeScriptTestCase(const script_call& c) {
	ug_lua_test_case_functor tc(c);
	string tc_name = c.get_name();
	return make_test_case(tc, tc_name);
}

} // end of impl namespace

////////////////////////////////////////////////////////////////////////////////
// adds test cases of given script calls to corresponding test suites
////////////////////////////////////////////////////////////////////////////////
void initScriptTestSuites(const vector<script_call>& calls) {
	if(calls.empty()) {
		std::cout << "didn't get a call";
		return;
	}

	int procs = 1;
#ifdef UG_PARALLEL
	procs = pcl::NumProcs();
#endif
	stringstream name;
	name << "LUAScriptsNumProc" << procs;

	// create it with empty name and set it later
	test_suite* suite_scripts = new test_suite(name.str());

	int global_timeout = -1;
	bool b_global_timeout = ParamToInt(global_timeout, "--timeout",
			framework::master_test_suite().argc,
			framework::master_test_suite().argv);
 
	for(size_t i = 0; i < calls.size(); ++i) {
		script_call c = calls[i];

		// if no numProcs are specified, run it in serial mode
		if(c.np.empty()) {
			c.np.push_back(1);
		}

		// if script should not be executed for this amount of processors, ignore it
		if(not binary_search(c.np.begin(), c.np.end(), procs)) {
			continue;
		}

		if(not binary_search(valid_np, valid_np +
				sizeof(valid_np) / sizeof(uint), procs))
			UG_THROW("invalid num proc parameter (" << procs <<
					") given for script: " << c.path);

		test_case* tc = makeScriptTestCase(c);

		// default timeout 2 minutes, if not set
		uint timeout;
		if(b_global_timeout) {
			timeout = global_timeout;
		} else {
			try {
				const string& t = c.params.at("timeout");
				stringstream ss(t, ios::in);
				ss >> timeout;
			} catch (std::out_of_range&) {
				timeout = 120;
			}
		}

		stringstream append_timeout;
		append_timeout << tc->p_name.get() << "_t" << timeout;

		tc->p_name.set(append_timeout.str());

		// add to test suite with expected failures = 0 and set timeout
		suite_scripts->add(tc, 0, timeout);
	}

	// scripts test suite to master ts
	// note: suite may be empty, which is usally ignored by the framework
	framework::master_test_suite().add(suite_scripts);
}

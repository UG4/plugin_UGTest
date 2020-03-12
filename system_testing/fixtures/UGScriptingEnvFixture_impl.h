/*
 * UGScriptingEnvFixture_impl.h
 *
 *  Created on: 10.01.2012
 *      Author: marscher
 */

#ifndef UGSCRIPTINGENVFIXTURE_IMPL_H_
#define UGSCRIPTINGENVFIXTURE_IMPL_H_

#include "UGScriptingEnvFixture.h"
#include "bind_boost_test.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>
#include <sstream>

#include "ug.h"
#include "common/profiler/profile_node.h"
#include "common/util/parameter_parsing.h"
#include "bindings/lua/lua_util.h"

namespace {
	using namespace boost::unit_test;
	using namespace ug;
	using namespace ug::script;
}

/**
 * pushes argc and argv on lua stack and overwrite lua print, write functions with
 * ug methods.
 */
UGScriptingEnvFixture::UGScriptingEnvFixture() {
	int argc = framework::master_test_suite().argc;
	char** argv = framework::master_test_suite().argv;

	try {
		lua_State* L = script::GetDefaultLuaState();

		// todo: use SetLuaUGArgs

		//	create ugargc and ugargv in lua
		int firstParamIndex = 1;
		int ugargc = argc - firstParamIndex;
		lua_pushnumber(L, ugargc);
		lua_setglobal(L, "ugargc");
		lua_newtable(L);
		for (int i = 0; i < ugargc; ++i) {
			//	push the index to the table
			lua_pushnumber(L, i + 1);
			//	push the value to the table
			lua_pushstring(L, argv[firstParamIndex + i]);
			//	create the entry
			lua_settable(L, -3);
		}
		//	set the tables name
		lua_setglobal(L, "ugargv");

		// replace LUAs print function with our own, to use UG_LOG
		RegisterStdLUAFunctions(L);

	} catch(UGError& err) {
		BOOST_ERROR("scripting fixture setup went wrong: " << err.get_msg());
	} catch(...) {
		BOOST_ERROR("scripting fixture setup went wrong with unknown error");
	}
}

/**
 * clean up lua script space
 */
UGScriptingEnvFixture::~UGScriptingEnvFixture() {
	// release the lua state which has been created in constructor of this fixture
	ReleaseDefaultLuaState();
}

#endif /* UGSCRIPTINGENVFIXTURE_IMPL_H_ */

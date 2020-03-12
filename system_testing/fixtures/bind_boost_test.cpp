/**
 * \file bind_boost_test.cpp
 *
 * \author Martin Scherer
 *
 * \date 04.03.2011
 *
 * Goethe-Center for Scientific Computing 2010.
 *
 *
 * This file defines some functions often used in the boost testing framework
 * for checking expressions etc.
 * see: http://www.boost.org/doc/libs/1_44_0/libs/test/doc/html/utf/testing-tools/reference.html
 */

#ifndef __BIND_BOOST_TEST_IMPL__
#define __BIND_BOOST_TEST_IMPL__

#include "bind_boost_test.h"
#include <string>
#include <exception>
#include <ostream>

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/execution_monitor.hpp>

#include "bridge/bridge.h"
#include "common/log.h"
#include "bindings/lua/lua_util.h"
#include "bindings/lua/bindings_lua.h"
#include "bindings/lua/externals/lua/lua.h"

namespace {

using namespace boost;
using namespace std;

/**
 * Wrapper for the lua exported functionality. Should avoid porting efforts for
 * future boost releases.
 */
bool wrap_boost_predicate_checking(bool p, const char* msg,
		const char* file, int line) {
	unit_test::unit_test_log.set_checkpoint(file, line);

	// this is the macro expansion of BOOST_CHECK_* taken from boost-1.48.
	// this might change in future version of boost, as it is impl detail!
	return test_tools::tt_detail::check_impl(p,
			unit_test::lazy_ostream::instance() << msg, file, line,
			test_tools::tt_detail::CHECK,
			test_tools::tt_detail::CHECK_PRED, 0);
}

/**
 * basic test function. Tests whether expression is true
 * @param expression to test
 * @param msg message to pass to boost framework what is intended to test
 */
void boost_check_impl(bool expression, const char* msg, const char* file,
		const int line) {
	wrap_boost_predicate_checking(expression, msg, file, line);
}

/**
 * requires that expression is true, otherwise throws an UGError.
 * @param expression to test
 * @param msg message to pass to boost framework what is intended to test
 */
bool boost_require_impl(bool expression, const char* msg, const char* file,
		const int line) {

	try{
		bool value = wrap_boost_predicate_checking(expression, msg, file, line);
		if(!value){
		//todo:	Use a boost exception or a special test-suite exeption
		//		instead of a ug exception.
			UG_THROW("Requirement not fulfilled");
		}
		return value;
	}
	catch(boost::execution_aborted& ex)
	{
		UG_THROW("Boost require failed. Aborting script execution.");
	}

	return false;
}

/**
 * checks whether n is smaller than epsilon
 * @param n number to test
 * @param epsilon small number
 * @param msg message to pass to boost framework what is intended to test
 */
void boost_check_small_impl(number n, number epsilon, const char* msg,
		const char* file, const int line) {

	unit_test::unit_test_log.set_checkpoint(file, line);

	// todo wrap to avoid clashes with future boost versions
	test_tools::tt_detail::check_frwd(test_tools::check_is_small,
			unit_test::lazy_ostream::instance() << msg, file,
			line, test_tools::tt_detail::CHECK,
			test_tools::tt_detail::CHECK_SMALL, n, "number", epsilon,
			"epsilon");
}

/**
 * passes the given message to the boost testing framework
 * @param msg
 */
void boost_message_impl(const char* msg, const char* file, const int line) {
	unit_test::unit_test_log.set_checkpoint(file, line);
	unit_test::lazy_ostream::instance() << msg;
}
} // end of impl namespace

/**
 * registers all required boost functions
 */
void RegisterBoostTestFunctions() {
	ug::bridge::Registry& reg = ug::bridge::GetUGRegistry();
	reg.add_function("boost_check_impl",&boost_check_impl).
		add_function("boost_require_impl", &boost_require_impl).
		add_function("boost_check_small_impl", &boost_check_small_impl).
		add_function("boost_message_impl", &boost_message_impl);
}

#endif

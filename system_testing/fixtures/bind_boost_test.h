/**
 * \file bind_boost_testsuite.cpp
 *
 * \author Martin Scherer
 *
 * \date 30.12.2010
 *
 * Goethe-Center for Scientific Computing 2010.
 */
#ifndef __BIND_BOOST_TEST_H_
#define __BIND_BOOST_TEST_H_

/*
void boost_check_impl(bool expression, const char* msg, const char* file,
		const int line);
bool boost_require_impl(bool expression, const char* msg, const char* file,
		const int line);
void boost_check_small_impl(number n, number epsilon, const char* msg,
		const char* file, const int line);
void boost_message_impl(const char* msg, const char* file, const int line);
*/

/**
 * registers all required boost functions with ug registry
 */
void RegisterBoostTestFunctions();

#endif

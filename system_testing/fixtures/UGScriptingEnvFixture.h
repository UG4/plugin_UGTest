/*
 * MasterTestSuite.h
 *
 * \author Martin Scherer
 *
 * \date 22.11.2010
 *  Goethe-Center for Scientific Computing 2010.
 */

#ifndef UGSCRIPTING_ENV_FIXTURE_H_
#define UGSCRIPTING_ENV_FIXTURE_H_

struct UGScriptingEnvFixture {
	/**
	 * this inits the UG Suite with the argv from the boost master testsuite
	 * @return
	 */
	UGScriptingEnvFixture();

	/**
	 * deconstruct ug environment.
	 */
	~UGScriptingEnvFixture();
};

// include definition
#include "UGScriptingEnvFixture_impl.h"

#endif /* MASTERTESTSUITE_H_ */

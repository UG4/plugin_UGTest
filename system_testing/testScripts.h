/*
 * testScripts.h
 *
 *  Created on: 13.02.2012
 *      Author: marscher
 */

#ifndef TESTSCRIPTS_H_
#define TESTSCRIPTS_H_
#include <vector>
// predecl
struct script_call;

/**
 * number of processes tested by jenkins
 */
const unsigned int valid_np[] = { 1, 2, 3, 4, 5, 6, 7, 8,
									9, 10, 11, 12, 13,14, 15, 16 };

void initScriptTestSuites(const std::vector<script_call>&);

#endif /* TESTSCRIPTS_H_ */

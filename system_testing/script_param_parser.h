
#ifndef __script_param_parser__
#define __script_param_parser__

#include <sstream>
#include <string>
#include <map>
#include <vector>
#include "common/parser/rapidxml/rapidxml.hpp"
#include "common/types.h"
/**
 * Stores path and parameters of script to test.
 * By default a script will be executed __only__ in serial mode.
 */
struct script_call {
	/**
	 * map parameters to their values, note that option switches have an empty
	 * value string.
	 */
	typedef std::map<std::string, std::string> PropertyMap;

	// path to script
	std::string path;
	// mapping of parameters to their values
	PropertyMap params;
	// stores on much processes each configuration will be executed
	std::vector<uint> np;
	// get name consisting out of all parameters
	std::string get_name() const;
};

std::ostream& operator<<(std::ostream& os, const script_call& c);

class ScriptParamsReader {
public:
	bool parse_file(const std::string& filename);
	const std::vector<script_call>& getScriptCalls() const;

private:
	std::vector<script_call> m_vCalls;

	void readNumProcs(rapidxml::xml_node<>* node, script_call& c);
	void readParam(rapidxml::xml_node<>* node, script_call& c);
	void parseConfig(rapidxml::xml_node<>* node, script_call& c);
};

#endif

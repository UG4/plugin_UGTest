#include "script_param_parser.h"
#include "testScripts.h"

#include <iostream>

#include "common/assert.h"
#include "common/error.h"
#include "common/parser/rapidxml/rapidxml.hpp"
#include "common/parser/rapidxml/rapidxml_print.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#ifdef UG_PARALLEL
#include "pcl/pcl.h"
#endif

using namespace std;
using namespace rapidxml;

string script_call::get_name() const {
	stringstream testcase_name;
	testcase_name << path;

	for(PropertyMap::const_iterator i =
			params.begin();	i != params.end(); ++i) {
		testcase_name << "__" << i->first << "_" << i->second;
	}

	int procs = 1;
#ifdef UG_PARALLEL
	procs = pcl::NumProcs();
#endif
	testcase_name << "_p" << procs;

	return testcase_name.str();
}

ostream& operator<<(ostream& os, const script_call& c) {
	os << c.get_name() << "\tnp: ";
	for(uint i = 0; i < c.np.size(); ++i)
		os << c.np[i] << ", ";

	return os;
}

bool ScriptParamsReader::parse_file(const string& filename)
{
	ifstream in(filename.c_str(), ios::binary);
	if(!in)
		UG_THROW(filename << " could not be opened.");

	rapidxml::xml_document<> doc;

//	get the length of the file
	streampos posStart = in.tellg();
	in.seekg(0, ios_base::end);
	streampos posEnd = in.tellg();
	streamsize size = posEnd - posStart;

//	go back to the start of the file
	in.seekg(posStart);

//	read the whole file en-block and terminate it with 0
	char* fileContent = doc.allocate_string(0, size + 1);
	in.read(fileContent, size);
	fileContent[size] = 0;
	in.close();


	//read filecontent to string

//	parse the xml-data
	doc.parse<0>(fileContent);

//	iterate through all script elements (start with first script element
	if(!doc.first_node("scripts")){
		std::cout << "check xml file: document does not start with 'scripts' element";
		UG_THROW("check xml file: document does not start with 'scripts' element");
	}

	xml_node<>* curNode = doc.first_node("scripts")->first_node("script");

	while(curNode) {
		script_call c;

		// parse path attribute of <script> element
		if(xml_attribute<>* path_attr = curNode->first_attribute("path")) {
			c.path = string(path_attr->value(), path_attr->value_size());
		} else {
			std::cout << "no path attribute given for script";
			UG_THROW("no path attribute given for script")
		}

		// parse config elements:
		xml_node<>* curConfNode = curNode->first_node("config");
		while(curConfNode) {
			parseConfig(curConfNode, c);
			curConfNode = curConfNode->next_sibling("config");
		}

		curNode = curNode->next_sibling("script");
	}
	return true;
}

const vector<script_call>& ScriptParamsReader::getScriptCalls() const {
	return m_vCalls;
}

void ScriptParamsReader::readNumProcs(xml_node<>* node, script_call& c) {
	UG_ASSERT(node, "not not set");
	UG_ASSERT(strcmp(node->name(), "numProcs") == 0, "not a numProcs node")

	string str(node->value(), node->value_size());

	// since there may be a parent numProc, clear it before parsing
	c.np.clear();
	vector<string> tokens;
	// split the string by whitespaces (spaces and tabs)
	boost::split(tokens, str, boost::is_any_of("\t "),
			 boost::token_compress_on);

	// try to interpret token as integer first, then as string
	for(uint i = 0; i < tokens.size(); ++i) {
		// todo temporary workaround, because boost does not "compress" tokens correctly
		if(tokens[i].empty())
			continue;
		try {
			uint np = boost::lexical_cast<uint>(tokens[i]);
			c.np.push_back(np);
		} catch(boost::bad_lexical_cast&) {
			if(tokens[i] == "all") {
				c.np.assign(valid_np, valid_np +
						sizeof(valid_np) / sizeof(uint));
				break;
			} else
				UG_THROW("bad sequence in numProcs: " << tokens[i])
		}
	}

	// ensure np is sorted for later binary search on it
	sort(c.np.begin(), c.np.end());
}

void ScriptParamsReader::readParam(xml_node<>* node, script_call& c) {
	UG_ASSERT(node, "node not set");
	UG_ASSERT(strcmp(node->name(), "param") == 0, "not a param node");

	script_call::PropertyMap& m = c.params;

	xml_attribute<>* name = node->first_attribute("name");
	UG_ASSERT(name, "name attribute has to be set");
	const char* value = node->value();
	if(name && value) {
		string param_s(name->value(), name->value_size());
		// note that a params value with the same name name (key) will be overwritten here
		m[param_s] = value;
	} else if(name && !value) {
		m[string(name->value(), name->value_size())] = "";
	} else {
		UG_THROW("neither parameter name or value set properly.")
	}
}

/**
 * recursively parses nested 'config' elements
 */
void ScriptParamsReader::parseConfig(xml_node<>* node, script_call& cParent) {
	UG_ASSERT(node, "node not valid");
	UG_ASSERT(strcmp(node->name(), "config") == 0, "not a config element");

	// copy settings from parent
	script_call c = cParent;

	// parse num procs
	if(xml_node<>* numProcs_node = node->first_node("numProcs")) {
		readNumProcs(numProcs_node, c);
	}

	// parse associated script parameters
	xml_node<>* curParam = node->first_node("param");
	while(curParam) {
		readParam(curParam, c);
		curParam = curParam->next_sibling("param");
	}

	// read subsequent config children
	// config has a nested config tag, parse recursivly
	xml_node<>* child_config_node = node->first_node("config");
	if (child_config_node) {
		while (child_config_node) {
			parseConfig(child_config_node, c);
			child_config_node = child_config_node->next_sibling("config");
		}
	} else {
		// config is a leaf
		m_vCalls.push_back(c);
	}
}


/*
 * Copyright (c) 2012-2015:  G-CSC, Goethe University Frankfurt
 * Author: Martin Rupp
 * 
 * This file is part of UG4.
 * 
 * UG4 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 3 (as published by the
 * Free Software Foundation) with the following additional attribution
 * requirements (according to LGPL/GPL v3 §7):
 * 
 * (1) The following notice must be displayed in the Appropriate Legal Notices
 * of covered and combined works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (2) The following notice must be displayed at a prominent place in the
 * terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (3) The following bibliography is recommended for citation and must be
 * preserved in all covered files:
 * "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
 *   parallel geometric multigrid solver on hierarchically distributed grids.
 *   Computing and visualization in science 16, 4 (2013), 151-164"
 * "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
 *   flexible software system for simulating pde based models on high performance
 *   computers. Computing and visualization in science 16, 4 (2013), 165-179"
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 */


#include "profiler.h"
#include <iomanip>
#include <cmath> // for floor
#include <algorithm>
#include <string>
#include <string.h>
#include "common/log.h"
#include "common/util/string_util.h"
#include "profile_node.h"
#include "common/util/string_util.h"
#include "common/util/path_provider.h"
#include <map>
#include <fstream>
#include "compile_info/compile_info.h"
#include "pcl/pcl_base.h"
#include "common/error.h"
#include "memtracker.h"

#ifdef UG_PARALLEL
#include "pcl/pcl.h"
#endif

#include "profile_call.h"


using namespace std;

namespace ug
{

#if SHINY_PROFILER

void ProfilerUpdate()
{
	Shiny::ProfileManager::instance.update(1.0);
	UpdateTotalMem();
}


static const int PROFILER_BRIDGE_OUTPUT_WIDTH_NAME = 70; // Shiny::OUTPUT_WIDTH_NAME
static const int PROFILER_BRIDGE_OUTPUT_WIDTH_HIT  = 13; // Shiny::OUTPUT_WIDTH_HIT
static const int PROFILER_BRIDGE_OUTPUT_WIDTH_TIME =  7; // Shiny::OUTPUT_WIDTH_TIME
static const int PROFILER_BRIDGE_OUTPUT_WIDTH_PERC =  4; // Shiny::OUTPUT_WIDTH_PERC


/*static string cut(string &s, size_t L)
{
	return s.substr(0, L);
}*/

static string cut(const char *p, size_t L)
{
	string s(p);
	return s.substr(0, L);
}


#define SHINY_DAMPING_FACTOR 1.0

double UGProfileNode::get_avg_entry_count() const
{
	if(!valid()) return 0.0;
	return data.entryCount.avg; // * SHINY_DAMPING_FACTOR;
}

double UGProfileNode::get_avg_self_time_ms() const
{
	return get_avg_self_time() * Shiny::GetTickInvFreq() * 1000.0;
}

double UGProfileNode::get_avg_total_time_ms() const
{
	return get_avg_total_time() * Shiny::GetTickInvFreq() * 1000.0;
}

double UGProfileNode::get_avg_self_time() const
{
	if(!valid()) return 0.0;
	return data.selfTicks.avg * SHINY_DAMPING_FACTOR;
}

double UGProfileNode::get_avg_total_time() const
{
	if(!valid()) return 0.0;
	return data.totalTicksAvg() * SHINY_DAMPING_FACTOR;
}

double UGProfileNode::get_self_mem() const
{
	if(!valid()) return 0.0;
	return GetSelfMem(this);
}


double UGProfileNode::get_total_mem() const
{
	if(!valid()) return 0.0;
	return GetTotalMem(this);
}

string UGProfileNode::get_mem_info(double fullMem) const
{
	if(HasMemTracking())
	{
		stringstream s;
		s << GetBytesSizeString((size_t)get_self_mem(), 10) << setw(5) << floor(get_self_mem() / fullMem * 100) << "%  ";
		s << GetBytesSizeString((size_t)get_total_mem(), 10) << setw(5) << floor(get_total_mem() / fullMem * 100) << "%  ";
		return s.str();
	}
	else
		return "";
}

string UGProfileNode::call_tree(double dSkipMarginal) const
{
	if(!valid()) return "Profile Node not valid!";

	stringstream s;
	UGProfileNode::log_header(s, "call tree");

	rec_print(get_avg_total_time_ms(), get_total_mem(), s, 0, dSkipMarginal);

	return s.str();
}

string UGProfileNode::child_self_time_sorted(double dSkipMarginal) const
{
	return print_child_sorted("self time sorted", UGProfileNode::self_time_sort, dSkipMarginal);
}

string UGProfileNode::total_time_sorted(double dSkipMarginal) const
{
	return print_child_sorted("total time sorted", UGProfileNode::total_time_sort, dSkipMarginal);
}

string UGProfileNode::child_self_memory_sorted(double dSkipMarginal) const
{
	return print_child_sorted("self memory sorted", UGProfileNode::self_memory_sort, dSkipMarginal);
}

string UGProfileNode::total_memory_sorted(double dSkipMarginal) const
{
	return print_child_sorted("total memory sorted", UGProfileNode::total_memory_sort, dSkipMarginal);
}

string UGProfileNode::entry_count_sorted(double dSkipMarginal) const
{
	return print_child_sorted("entry count sorted", UGProfileNode::entry_count_sort, dSkipMarginal);
}


bool UGProfileNode::valid() const
{
	return this != PROFILER_NULL_NODE;
}

string SimplifyUG4Path(string s)
{
	const string &ug4root = PathProvider::get_path(ROOT_PATH);
	if(StartsWith(s, ug4root))
		return string("$") + s.substr(ug4root.length());
	else
		return s;
}

// private functions

void UGProfileNode::PDXML_rec_write(ostream &s) const
{
	if(!valid()) return;
	
	/*
	 * <node>
	 * name
	 * group
	 * file
	 * line
	 * hits
	 * self
	 * total	 * 
	 * </node>	 
	 */
	
	s << "<node>\n";
	if(zone->name)
	{
		if(strcmp(zone->name, "<root>") == 0)
			s << "<name>root</name>\n";
		else
			s << "<name>" << XMLStringEscape(zone->name) << "</name>\n";
	}
	if(zone->groups)
		s << "<groups>" << XMLStringEscape(zone->groups) << "</groups>\n";
	
	if(zone->file != NULL)
	{
		s << "<file>" << SimplifyUG4Path(zone->file) << "</file>\n";
		s << "<line>" << zone->line << "</line>\n";
	}

#ifdef SHINY_CALL_LOGGING
	s << "<id>" << this << "</id>\n";
#endif
	 
	s << "<hits>" << floor(get_avg_entry_count()) << "</hits>\n"
	  << "<self>" << get_avg_self_time_ms() * 1000.0 << "</self>\n"
	  << "<total>" << get_avg_total_time_ms() * 1000.0 << "</total>\n";

	if(HasMemTracking())
	{
		s << "<totalMemory>" << get_total_mem() << "</totalMemory>\n";
		s << "<selfMemory>" << get_self_mem() << "</selfMemory>\n";
	}
			
	for(const UGProfileNode *p=get_first_child(); p != NULL; p=p->get_next_sibling())
	{
		p->PDXML_rec_write(s);
		if(p==get_last_child())
			break;
	}

	s << "</node>\n";
}

string GetProfilingFileLine(const char *name)
{
	const char *p = strchr(name, ':'); // search for line number
	if(!(p == NULL || p[0] == 0x00 || p[1] == 0x00))
	{
		// if we find the corresponding code in the LUA file, print the code as "name"
		// and the filename above

		int line = strtol(p+1, NULL, 10);
		char file[255];
		strncpy(file, name, p-name);
		file[p-name]=0x00;
		string str = GetFileLine(file, line);
		for(size_t i=0; i<str.size(); i++) if(str[i] == '\t') str[i] = ' ';
		return str;
	}
	else return "";

}

string UGProfileNode::print_node(double fullMs, double fullMem, size_t offset) const
{
	if(!valid()) return "";
	const Shiny::TimeUnit *selfUnit = Shiny::GetTimeUnit(get_avg_self_time());
	const Shiny::TimeUnit *totalUnit = Shiny::GetTimeUnit(get_avg_total_time());
	stringstream s;
	if(offset)	s << setw(offset) << " ";

	// name
	if(zone->name[0] == '@')
	{
		// zone name is a filename
		// this can happen when we are using LUA script profiling.

		// check if filename is too long
		if(strlen(zone->name) > (PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset) )
			s << "@... " << zone->name+strlen(zone->name)-(PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset-5);
		else
			s << left << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset) << cut(zone->name, PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset);

		string str = GetProfilingFileLine(zone->name+1);
		if(str.size())
		{
			s << "\n";
			if(offset)	s << setw(offset) << " ";
			s << left << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset) <<
					cut(str.c_str(), PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset);
		}
	}
	else
		s << left << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset) << cut(zone->name, PROFILER_BRIDGE_OUTPUT_WIDTH_NAME-offset);

	// entry count
	s <<	right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_HIT) << floor(get_avg_entry_count()) << " " <<
			setprecision(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME-1);

	// self time
	s << 	setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME) << get_avg_self_time() * selfUnit->invTickFreq << " " <<
			left << setw(2) << selfUnit->suffix << " " <<
			right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_PERC) << floor(get_avg_self_time_ms() / fullMs * 100) << "%  ";

	// total time
	s << 	setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME) << get_avg_total_time() * totalUnit->invTickFreq << " " <<
			left << setw(2) << totalUnit->suffix << " " <<
			right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_PERC) << floor(get_avg_total_time_ms() / fullMs * 100) << "%  ";
	if(fullMem >= 0.0)
		s << get_mem_info(fullMem);
	if(zone->groups != NULL)
		s << zone->groups;
	return s.str();
}

const UGProfileNode *UGProfileNode::get_first_child() const
{
	return reinterpret_cast<const UGProfileNode*>(firstChild);
}

const UGProfileNode *UGProfileNode::get_last_child() const
{
	return reinterpret_cast<const UGProfileNode*>(lastChild);
}

const UGProfileNode *UGProfileNode::get_next_sibling() const
{
	return reinterpret_cast<const UGProfileNode*>(nextSibling);
}

const UGProfileNode *UGProfileNode::find_next_in_tree() const
{
	return reinterpret_cast<const UGProfileNode*>(findNextInTree());
}

void UGProfileNode::rec_print(double fullMs, double fullMem, stringstream &s, size_t offset, double dSkipMarginal) const
{
	if(!valid()) return;
	if(dSkipMarginal==0.0 ||
			(fullMs*dSkipMarginal < get_avg_total_time_ms() || (HasMemTracking() && fullMem*dSkipMarginal< get_total_mem() ) )
			)
	{
		s << print_node(fullMs, fullMem, offset) << "\n";
		for(const UGProfileNode *p=get_first_child(); p != NULL; p=p->get_next_sibling())
		{
			p->rec_print(fullMs, fullMem, s, offset+1, dSkipMarginal);
			if(p==get_last_child())
				break;
		}
	}
}

string UGProfileNode::groups() const
{
	if (!valid()) return "Profile Node not valid!";

	vector<const UGProfileNode*> nodes;
	rec_add_nodes(nodes);

	map<string, double> mapGroups;
	for(size_t i=0; i<nodes.size(); i++)
	{
		if(nodes[i]->zone->groups == NULL) continue;
		vector<string> g;
		TokenizeString(nodes[i]->zone->groups, g, ' ');
		for(size_t j=0; j<g.size(); j++)
			mapGroups[g[j]] += nodes[i]->get_avg_self_time();
	}

	vector<string> gs;
#ifdef UG_PARALLEL
	if(pcl::ProcRank() == 0)
#endif
	for(map<string, double>::iterator it = mapGroups.begin(); it != mapGroups.end();++it)
		gs.push_back(it->first);

#ifdef UG_PARALLEL
	pcl::ProcessCommunicator pc;
	pc.broadcast(gs);
	vector<double> t(gs.size(), 0.0), tMax, tMin;
	for(size_t i=0; i<gs.size(); i++)
		t[i] = mapGroups[gs[i]];
	pc.allreduce(t, tMax, PCL_RO_MAX);
	pc.allreduce(t, tMin, PCL_RO_MIN);
#endif
	stringstream s;
	for(size_t i=0; i<gs.size(); i++)
	{
		string name = gs[i];
		double time = mapGroups[name];
		const Shiny::TimeUnit *unit = Shiny::GetTimeUnit(time);
		s << left << setw(20) << name
		  << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME) << time * unit->invTickFreq << " " <<
			left << setw(2) << unit->suffix;
#ifdef UG_PARALLEL
		double maxTime = tMax[i];
		double minTime = tMin[i];
		double diffTime = maxTime - minTime;
		const Shiny::TimeUnit *maxUnit  = Shiny::GetTimeUnit(maxTime);
		const Shiny::TimeUnit *minUnit  = Shiny::GetTimeUnit(minTime);
		const Shiny::TimeUnit *diffUnit = Shiny::GetTimeUnit(diffTime);
		s << left << " max: " << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME)
				<< maxTime * maxUnit->invTickFreq << " " <<	left << setw(2) << maxUnit->suffix;
		s << left << " min: " << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME)
						<< minTime * maxUnit->invTickFreq << " " <<	left << setw(2) << minUnit->suffix;
		s << left << " diff: " << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME)
								<< diffTime * diffUnit->invTickFreq << " " <<	left << setw(2) << diffUnit->suffix
								<< " (" << diffTime/maxTime*100 << " %)";
#endif
		s << "\n";
	}
	return s.str();
}


void UGProfileNode::rec_add_nodes(vector<const UGProfileNode*> &nodes) const
{
	nodes.push_back(this);
	for(const UGProfileNode *p=get_first_child(); p != NULL; p=p->get_next_sibling())
	{
		p->rec_add_nodes(nodes);
		if(p==get_last_child())
			break;
	}
}

string UGProfileNode::print_child_sorted(const char *name, bool sortFunction(const UGProfileNode *a, const UGProfileNode *b),
		double dSkipMarginal) const
{
	if(!valid()) return "";
	stringstream s;
	vector<const UGProfileNode*> nodes;
	rec_add_nodes(nodes);
	sort(nodes.begin(), nodes.end(), sortFunction);

	UGProfileNode::log_header(s, name);
	double fullMs = get_avg_total_time_ms();
	double fullMem = get_total_mem();
	for(size_t i=0; i<nodes.size(); i++)
	{
		if(dSkipMarginal==0.0 || fullMs*dSkipMarginal < nodes[i]->get_avg_total_time_ms())
			s << nodes[i]->print_node(fullMs, fullMem) << "\n";
	}
	return s.str();
}

void UGProfileNode::log_header(stringstream &s, const char *name)
{

	s << 	left << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_NAME) << name << " " <<
			right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_HIT) << "hits" << " " <<
			setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME+4+PROFILER_BRIDGE_OUTPUT_WIDTH_PERC+1 -4) << "self time" << " " <<
			setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME+4+PROFILER_BRIDGE_OUTPUT_WIDTH_PERC+1) << "total time";
	if(HasMemTracking())
	{
		s << "  " << setw(10+5+3) << "self mem" << "   " <<
				setw(10) << "total mem";
	}

	s << "\n";
}

bool UGProfileNode::self_time_sort(const UGProfileNode *a, const UGProfileNode *b)
{
	return a->get_avg_self_time() < b->get_avg_self_time();
}

bool UGProfileNode::total_time_sort(const UGProfileNode *a, const UGProfileNode *b)
{
	return a->get_avg_total_time() < b->get_avg_total_time();
}

bool UGProfileNode::self_memory_sort(const UGProfileNode *a, const UGProfileNode *b)
{
	return a->get_self_mem() < b->get_self_mem();
}

bool UGProfileNode::total_memory_sort(const UGProfileNode *a, const UGProfileNode *b)
{
	return a->get_total_mem() < b->get_total_mem();
}


bool UGProfileNode::entry_count_sort(const UGProfileNode *a, const UGProfileNode *b)
{
	return a->get_avg_entry_count() < b->get_avg_entry_count();
}

/*
void WriteCompressedProfileData(const char *filename)
{
	// compressed:
	// filedata: file0, file1, file2, file3
	// zonedata: zone1 {name, fileid, line}, zone2, zone3	
    // nodes: node0 {self, total, children}, node1, node2, node3.
 
    // combine file and zone data, so that profile data is
 * 
 * // <files>...</files>
 * <zones><zone id="732"><name>bla</name><group>algebra</group><file>8</file><line>344</line> </zone> ... </zones>
 * <core id=0>
 *   <node zoneId=732><self>8734.2</self><total>8333.2</total></node>
    
}
*/

#ifndef NDEBUG
#define PROFILE_NODE_MIN_HITS 500
#define PROFILE_NODE_MIN_TOTAL_TIME_MS 100
#define PROFILE_NODE_MAX_TIME_PER_CALL_MS 0.01
#else
#define PROFILE_NODE_MIN_HITS 1000
#define PROFILE_NODE_MIN_TOTAL_TIME_MS 1000
#define PROFILE_NODE_MAX_TIME_PER_CALL_MS 0.01
#endif

void UGProfileNode::check_for_too_small_nodes(double fullMs, map<string, const UGProfileNode *> &list) const
{
	// also don't check nodes which require less time than 0.01% of the whole problem
	if(get_avg_total_time_ms() < 0.01*0.01*fullMs) return;
	for(const UGProfileNode *p=get_first_child(); p != NULL; p=p->get_next_sibling())
	{
		double t_ms = p->get_avg_total_time_ms();
		size_t entry = (size_t)p->get_avg_entry_count();

		if(t_ms > PROFILE_NODE_MIN_TOTAL_TIME_MS)
		{
			if(entry > PROFILE_NODE_MIN_HITS && t_ms/entry < PROFILE_NODE_MAX_TIME_PER_CALL_MS)
			{
				string desc = p->zone->name;
				if(p->zone->file != NULL)
					desc.append(string(p->zone->file) + string(":") + ToString(p->zone->line));
				const UGProfileNode *p2 = list[desc];

				if(p2 == NULL)
					list[desc] = p;
				else
				{
					double t_ms2 = p->get_avg_total_time_ms();
					size_t entry2 = (size_t)p->get_avg_entry_count();
					if(entry2/t_ms2 < entry/t_ms)
						list[desc] = p;
				}
			}

			p->check_for_too_small_nodes(fullMs, list);
		}
		if(p==get_last_child())
			break;
	}

}

void ProfileTestFunction1()
{
	PROFILE_FUNC();
}

void ProfileTestFunction2()
{
	PROFILE_FUNC();
}


void UGProfileNode::CheckForTooSmallNodes()
{
	Shiny::ProfileManager::instance.update(1.0); // WE call with damping = 1.0
	const UGProfileNode *pnRoot = UGProfileNode::get_root();

	const size_t numProfileCall = 100;
	for(size_t i=0; i<numProfileCall; i++)
	{
		ProfileTestFunction1();
		ProfileTestFunction2();
	}

	const UGProfileNode *ptf1 = GetProfileNode("ProfileTestFunction1");

	double fullMs = pnRoot->get_avg_total_time_ms();

	if(fullMs > 1)
	{
		map<string, const UGProfileNode *> list;
		pnRoot->check_for_too_small_nodes(fullMs, list);

		if(list.size() != 0)
		{
			UG_LOG(resetiosflags( ::ios::scientific ));
			UG_LOG("WARNING: Some profile nodes might be too small\n");
			UG_LOG("----------------------------------------------------------------------\n");

			double tProfileCall= ptf1->get_avg_total_time()/numProfileCall;
			const Shiny::TimeUnit *unit2 = Shiny::GetTimeUnit(tProfileCall);

			UG_LOG("Profile Call overhead estimated at " << tProfileCall * unit2->invTickFreq << " " << unit2->suffix << ".\n");

			UG_LOG("Displaying nodes with hits > " << PROFILE_NODE_MIN_HITS << " and totalTime > " << PROFILE_NODE_MIN_TOTAL_TIME_MS
					<< " and totalTime/hits < " << PROFILE_NODE_MAX_TIME_PER_CALL_MS << " ms :\n");


			stringstream s;
			s << 	left << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_NAME) << "name" << " " <<
					right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_HIT) << "hits" << " " <<
					setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME+4) << "total  " << " " <<
					setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME+5) << "total/hits  " <<
					"  potentially lost (prediction)\n";
			for(map<string, const UGProfileNode *>::iterator it = list.begin();
					it != list.end(); ++it)
			{
				const UGProfileNode *p = it->second;
				if(p->zone->file != NULL)
					s << SimplifyUG4Path(p->zone->file) << ":" << p->zone->line << " : \n";

				s << left << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_NAME) << p->zone->name << " ";
				// entry count
				s <<	right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_HIT) << floor(p->get_avg_entry_count()) << " " <<
						setprecision(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME-1);

				// total time
				const Shiny::TimeUnit *unit = Shiny::GetTimeUnit(p->get_avg_total_time());
				s << 	right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME) << p->get_avg_total_time() * unit->invTickFreq << " " <<
						left << setw(2) << unit->suffix << "   ";

				// fraction
				double t = p->get_avg_total_time()/p->get_avg_entry_count();
				unit = Shiny::GetTimeUnit(t);
				s << 	right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME) <<  t * unit->invTickFreq << " " <<
						left << setw(2) << unit->suffix << "   ";

				// lost

				t = tProfileCall*p->get_avg_entry_count();
				unit = Shiny::GetTimeUnit(t);
				s << 	right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME) <<  t * unit->invTickFreq << " " <<
						left << setw(2) << unit->suffix << "   ";

				// %

//				t = tProfileCall*p->get_avg_entry_count();
//				unit = Shiny::GetTimeUnit(t);
//				s << (tProfileCall*p->get_avg_entry_count()*100)/ p->get_avg_total_time() << "% ";

				s << "\n";
			}
			UG_LOG(s.str());

		}
	}
}

const UGProfileNode *UGProfileNode::get_root()
{
	const Shiny::ProfileNode *node = &Shiny::ProfileManager::instance.rootNode;
	return reinterpret_cast<const UGProfileNode*> (node);
}


void WriteProfileDataXML(const char *filename)
{
	WriteProfileDataXML(filename, ug::GetLogAssistant().get_output_process());
}


void WriteCallLog(const char *filename, int procId)
{
#ifdef SHINY_CALL_LOGGING
	if(pcl::ProcRank() != procId) return;
	fstream f(filename, ios::out);

	Shiny::tick_t curTime;
	Shiny::GetTicks(&curTime);

	FinishShinyCallLogging();
	UG_LOG("Writing Call log to " << filename << ". Max Frequency is " << g_ShinyCallLoggingMaxFreq << " Hz.\n")
	// print call log
	int depth=0;
	for(size_t i=0; i<profileCalls.size(); i++)
	{
		f << profileCalls[i].c << " ";
		if(profileCalls[i].p == NULL)
		{
			depth--;
			f << repeat(' ', depth) << "}\n";
		}
		else
		{
			f << repeat(' ', depth);

			if(profileCalls[i].p->zone->name[0] == '@')
			{
				string str = GetProfilingFileLine(profileCalls[i].p->zone->name +1);
				if(str.size())
				{	f << str << " "; }

				f << "(" << SimplifyUG4Path(profileCalls[i].p->zone->name+1) << ")\n";
			}
			else
				f << profileCalls[i].p->zone->name << " (" << SimplifyUG4Path(profileCalls[i].p->zone->file) << ":" << profileCalls[i].p->zone->line << ")\n";

			if(i+1 < profileCalls.size() && profileCalls[i+1].p == NULL)
				i++;
			else
			{
				f << profileCalls[i].c << " ";
				f << repeat(' ', depth) << "{\n";
				depth++;
			}
		}
	}
	for(int i=0; i<depth; i++)
		f << curTime << " " << repeat(' ', depth-i-1) << "}\n";
#else
	UG_LOG("Did NOT write Call Log since Call Logging is disabled (enable with cmake -DSHINY_CALL_LOGGING=ON ..)\n");
#endif
}

void WriteCallLog(const char *filename)
{
	WriteCallLog(filename, ug::GetLogAssistant().get_output_process());
}

void WriteProfileDataXML(const char *filename, int procId)
{
	#ifdef UG_PARALLEL
		UG_COND_THROW(procId >= pcl::NumProcs(),
					  "Bad process id: " << procId << ". Maximum allowed is "
					  << pcl::NumProcs() - 1);
	#else
		UG_COND_THROW(procId >0,
					  "Bad process id: " << procId
					  << ". Only one process available (with id 0)");
	#endif

#ifdef SHINY_CALL_LOGGING
	Shiny::tick_t curTime;
	Shiny::GetTicks(&curTime);
#endif
	ProfilerUpdate();
	const UGProfileNode *pnRoot = UGProfileNode::get_root();

//	procId represents the id of the process which will write the file.
//	if all processes are considered (procId < 0), then process 0 will write the file.
	bool gatherFromAllProcs = false;
	if(procId < 0){
		gatherFromAllProcs = true;
		procId = 0;
	}

#ifdef UG_PARALLEL
	typedef pcl::SingleLevelLayout<pcl::OrderedInterface<size_t, vector> >
		IndexLayout;

	pcl::InterfaceCommunicator<IndexLayout> ic;
	if(pcl::ProcRank() == procId)
	{
#endif
		fstream f(filename, ios::out);
		f << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
		f << "<!-- ug4 created profile data -->\n";
		f << "<ProfileData>\n";
		f << "<AdditionalInfo>\n";
		f << "<SVNRevision>" << UGSvnRevision() << "</SVNRevision>\n";
		f << "<BuildHostname>" << UGBuildHost() << "</BuildHostname>\n";
		f << "<CompileDate>" << UGCompileDate() << "</CompileDate>\n";
		f << "</AdditionalInfo>\n";
		f << "<basepath>" << PathProvider::get_path(ROOT_PATH) << "</basepath>\n";

		f << "<core id=\""<<procId<<"\">\n";
		
		pnRoot->PDXML_rec_write(f);
		f << "</core>\n";

#ifdef SHINY_CALL_LOGGING
		FinishShinyCallLogging();
		f << "<log>\n";
		int depth=0;
		for(size_t i=0; i<profileCalls.size(); i++)
		{
			if(profileCalls[i].p == NULL)
			{
				f << "<stop>" << profileCalls[i].c << "</stop>\n";
				f << "</call>\n";
				depth--;
			}
			else
			{
				f << "<call>\n<start>" << profileCalls[i].c << "</start>\n<id>" << profileCalls[i].p << "</id>\n";
				depth++;
			}
		}
		for(int i=0; i<depth; i++)
		{
			f << "<stop>" << curTime << "</stop>\n";
			f << "</call>\n";
		}
		f << "</log>\n";

#endif
		
#ifdef UG_PARALLEL
		if(gatherFromAllProcs)
		{
			vector<ug::BinaryBuffer> buffers(pcl::NumProcs()-1);
			for(int i=1; i<pcl::NumProcs(); i++)
				ic.receive_raw(i, buffers[i-1]);
			ic.communicate();

			for(int i=1; i<pcl::NumProcs(); i++)
			{
				f << "\n<core id=\"" << i << "\">";
				string s;
				Deserialize(buffers[i-1], s);
				f << s;			
				f << "\n</core>";
			}		
		}
#endif
		f << "</ProfileData>\n";
		
#ifdef UG_PARALLEL		
	}	
	else		
	{
		if(gatherFromAllProcs)
		{
			stringstream ss;
			pnRoot->PDXML_rec_write(ss);
			BinaryBuffer buf;
			Serialize(buf, ss.str());
			ic.send_raw(procId, buf.buffer(), buf.write_pos(), false);
			ic.communicate();
		}
	}	
#endif	
	
}

const UGProfileNode *GetProfileNode(const char *name, const UGProfileNode *node)
{
	ProfilerUpdate();
	if(node == NULL) node = UGProfileNode::get_root();
	if(name == NULL)
		return node;
	do
	{
		if(strcmp(node->zone->name, name) == 0)
			return node;
		node = node->find_next_in_tree();
	} while (node);

//	UG_LOG("Profiler Node \"" << name << "\" not found\n");
	return PROFILER_NULL_NODE;
}

const UGProfileNode *GetProfileNode(const char *name)
{
	return GetProfileNode(name, NULL);
}

bool GetProfilerAvailable()
{
	return true;
}

void PrintLUA()
{
	const UGProfileNode *rootNode = GetProfileNode(NULL);
	vector<const UGProfileNode*> nodes;
	rootNode->rec_add_nodes(nodes);
	double full = rootNode->get_avg_total_time_ms();

	map<string, vector<double> > files;
	for(size_t i=0; i<nodes.size(); i++)
	{
		if(nodes[i]->zone->groups == NULL ||
				strcmp(nodes[i]->zone->groups, "lua") != 0)
			continue;
		const char *name = nodes[i]->zone->name;
		cout << name << "\n";
		if(name[0]==0x00 || name[1]==0x00) continue;
		name++; // skip @
		const char *p = strchr(name, ':'); // search for line number
		if(p == NULL || p[0] == 0x00 || p[1] == 0x00) continue;
		int line = strtol(p+1, NULL, 10);
		if(line > 10000 || line < 0) continue;
		char file[255];
		strncpy(file, name, p-name);
		file[p-name]=0x00;
		vector<double> &v = files[file];
		if(v.size() < (size_t)line+1)
		{
			size_t s=v.size();
			v.resize(line+1);
			for(; s<(size_t)line+1; s++)
				v[s]=0.0;
		}
		v[line] = nodes[i]->get_avg_total_time_ms();
	}


	for(map<string, vector<double> >::iterator it = files.begin(); it != files.end();
		++it)
	{
		string name = it->first;
		vector<double> &v = it->second;
		cout << "\n" << name << ":\n\n";

		char buf[512];
		fstream file(name.c_str(), ios::in);
		if(file.is_open() == false) continue;
		size_t lineNr=0;
		while(!file.eof())
		{
			file.getline(buf, 512);
			double time = lineNr >= v.size() ? 0.0 : v[lineNr];
			const Shiny::TimeUnit *unit = Shiny::GetTimeUnit(time);
			cout << resetiosflags( ::ios::scientific ) <<
					left << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_TIME)
					<< time * unit->invTickFreq << " " <<
				left << setw(2) << unit->suffix << " " <<
				right << setw(PROFILER_BRIDGE_OUTPUT_WIDTH_PERC) << floor(time / full * 100) << "%  "
				<< setw(3) << lineNr << "| " << buf << "\n";
			lineNr ++;
		}
	}
}



#else


double UGProfileNode::get_avg_entry_count() const
{
	return 0;
}

/// \return time in milliseconds spend in this node excluding subnodes
double UGProfileNode::get_avg_self_time_ms() const
{
	return 0.0;
}

/// \return time in milliseconds spend in this node including subnodes
double UGProfileNode::get_avg_total_time_ms() const
{
	return 0.0;
}

double UGProfileNode::get_total_mem() const
{
	return 0.0;
}

double UGProfileNode::get_self_mem() const
{
	return 0.0;
}
///////////////////////////////////////////////////////////////////
string UGProfileNode::call_tree(double dSkipMarginal) const
{
	return "Profiler not available!";
}

string UGProfileNode::child_self_time_sorted(double dSkipMarginal) const
{
	return "Profiler not available!";
}

string UGProfileNode::total_time_sorted(double dSkipMarginal) const
{
	return "Profiler not available!";
}


string UGProfileNode::child_self_memory_sorted(double dSkipMarginal) const
{
	return "Profiler not available!";
}

string UGProfileNode::total_memory_sorted(double dSkipMarginal) const
{
	return "Profiler not available!";
}

string UGProfileNode::entry_count_sorted(double dSkipMarginal) const
{
	return "Profiler not available!";
}

/*
string UGProfileNode::child_self_time_sorted(double dSkipMarginal) const
{
	return "Profiler not available!";
}

string UGProfileNode::child_self_time_sorted() const
{
	return child_self_time_sorted(0.0);
}

string UGProfileNode::total_time_sorted() const
{
	return total_time_sorted(0.0);
}
*/

/// \return true if node has been found
bool UGProfileNode::valid() const
{
	return false;
}

string UGProfileNode::groups() const
{
	return "Profiler not available!";
}

const UGProfileNode *GetProfileNode(const char *name)
{
	return PROFILER_NULL_NODE;
}

const UGProfileNode *GetProfileNode(const char *name, const UGProfileNode *node)
{
	return PROFILER_NULL_NODE;
}

void WriteProfileDataXML(const char *filename)
{
	return;
}

void WriteProfileDataXML(const char *filename, int procId)
{
	return;
}

bool GetProfilerAvailable()
{
	return false;
}

void PrintLUA()
{
	UG_LOG("LUA Profiler not available.");
}

void UGProfileNode::CheckForTooSmallNodes()
{

}

void WriteCallLog(const char *filename) {}
void WriteCallLog(const char *filename, int procId) {}
#endif // SHINY

} // namespace ug


//////////////////////////////////////////////////////////////////////////////////////////

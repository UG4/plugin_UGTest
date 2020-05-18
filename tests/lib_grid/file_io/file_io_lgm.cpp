/*
 * Copyright (c) 2009-2015:  G-CSC, Goethe University Frankfurt
 * Authors: Sebastian Reiter, Nicolas Tessore
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

#include "file_io_lgm.h"

#include <vector>

#include "lib_grid/lg_base.h"
#include "common/math/ugmath.h"

extern "C" {
#include <lib_grid/file_io/externals/include/lgm/lgm.h>
}

using namespace std;

namespace ug
{

////////////////////////////////////////////////////////////////////////
//	ImportGridFromLGM
bool ImportGridFromLGM(Grid& grid,
                       const char* filename,
                       AVector3& aPos,
                       ISubsetHandler* pSurfaceHandler)
{
//	we'll read the lgm in two steps:
//	first we'll try to load a 3d lgm. If that fails we'll try to
//	load a 2d on. If that fails too, we'll give up.

	// create lgm object
	lgm* l = lgm_new();
	// read lgm file
	lgm_info* linfo = lgm_info_new();
	
//	load 3d
	if(lgm_read(filename, l, linfo))
	{/*
		LOG("WARNING in ImportGridFromLGM: " << linfo->err_msg << endl);
		lgm_delete(l);
		lgm_info_delete(linfo);
		return false;*/

	//	3d could not be loaded. Try 2d.
	//TODO: add full 2d support to lgm_parser. There are problems
	//		with line left and line right (They are not yet parsed).
	//		This leads to an error in the current implementation.
		lgm_info_delete(linfo);
		lgm_delete(l);
		l = lgm_new();
		l->dim = 2;
		linfo = lgm_info_new();

		if(lgm_read(filename, l, linfo)){
			LOG("WARNING in ImportGridFromLGM: " << linfo->err_msg << endl);
			lgm_info_delete(linfo);
			lgm_delete(l);
			return false;
		}
	}
	lgm_info_delete(linfo);

	// make sure lgm has dimension of 2 or 3
	if(l->dim != 2 && l->dim != 3)
	{
		LOG("WARNING in ImportGridFromLGM: "
		    << "LGM is does not have dimension of 2 or 3!"
		    << endl);
		lgm_delete(l);
		return false;
	}

	//	set up vertex attachment
	if(!grid.has_vertex_attachment(aPos))
		grid.attach_to_vertices(aPos);

	//	set up vertex attachment accessor
	Grid::VertexAttachmentAccessor<AVector3> aaPosition(grid, aPos);

	//	read points and store them in an array for index access
	vector<RegularVertex*> vVertices;
	vVertices.reserve(l->num_points);

	// read points
	for(int i = 0; i < l->num_points; ++i)
	{
		// get point
		double* point = l->points[i];

		// create and store vertex
		RegularVertex* vert = *grid.create<RegularVertex>();
		vVertices.push_back(vert);

		// set vertex coordinates
		aaPosition[vert] = vector3(
			(number)point[0],
			(number)point[1],
			(l->dim == 3) ? (number)point[2] : (number)0.
		);
	}

	//	read lines and store them in an array for index access
	vector<RegularEdge*> vEdges;
	vEdges.reserve(l->num_lines);

	//	read lines
	for(int i = 0; i < l->num_lines; ++i)
	{
		// get line
		lgm_line* li = &l->lines[i];

		// scan through line nodes
		for(int j = 1; j < li->num_points; ++j)
		{
			// vertex indices
			int v1 = li->points[j-1];
			int v2 = li->points[j];

			// create edge
			RegularEdge* e = *grid.create<RegularEdge>(EdgeDescriptor(
				vVertices[v1],
				vVertices[v2]
			));

			// add line to line subset
			if(pSurfaceHandler)
				pSurfaceHandler->assign_subset(e, i);

			// store edge
			vEdges.push_back(e);
		}
	}

	// read surfaces
	for(int i = 0; i < l->num_surfaces; ++i)
	{
		// get surface
		lgm_surface* s = &l->surfaces[i];
		/*
		// read points
		for(int j = 0; j < s->num_points; ++j)
		{
			// vertex index
			int v = s->points[j];

			// add vertex to surface subset
			if(pSurfaceHandler)
				pSurfaceHandler->assign_subset(vVertices[v], i);
		}

		// read lines
		for(int j = 0; j < s->num_lines; ++j)
		{
			// edge index
			int e = s->lines[j];

			// add edge to surface subset
			if(pSurfaceHandler)
				pSurfaceHandler->assign_subset(vEdges[e], i);
		}*/

		// read triangles
		for(int j = 0; j < s->num_triangles; ++j)
		{
			// vertex indices
			int v1 = s->triangles[j][0];
			int v2 = s->triangles[j][1];
			int v3 = s->triangles[j][2];

			// create triangle
			Triangle* t = *grid.create<Triangle>(TriangleDescriptor(
				vVertices[v1],
				vVertices[v2],
				vVertices[v3]
			));

			// add triangle to surface subset
			if(pSurfaceHandler)
				pSurfaceHandler->assign_subset(t, i);
		}
	}

	// done importing!

	// delete lgm object
	lgm_delete(l);

	return true;
}

}//	end of namespace

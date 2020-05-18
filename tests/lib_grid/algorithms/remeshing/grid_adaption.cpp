/*
 * Copyright (c) 2010-2015:  G-CSC, Goethe University Frankfurt
 * Author: Sebastian Reiter
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

#include <queue>
#include <stack>
#include "lib_grid/lg_base.h"
#include "../geom_obj_util/geom_obj_util.h"
#include "lib_grid/refinement/regular_refinement.h"
#include "lib_grid/refinement/projectors/cylinder_cut_projector.h"
#include "grid_adaption.h"

using namespace std;

namespace ug
{

bool AdaptSurfaceGridToCylinder(Selector& selOut, Grid& grid,
							   Vertex* vrtCenter, const vector3& normal,
							   number radius, number rimSnapThreshold,  AInt& aInt,
							   APosition& aPos)
{
	if(!grid.has_vertex_attachment(aPos)){
		UG_THROW("Position attachment required!");
	}

	Grid::VertexAttachmentAccessor<APosition> aaPos(grid, aPos);

	if(rimSnapThreshold < 0)
		rimSnapThreshold = 0;

	if(rimSnapThreshold > (radius - SMALL))
		rimSnapThreshold = radius - SMALL;

	const number smallRadius = radius - rimSnapThreshold;
	const number smallRadiusSq = smallRadius * smallRadius;
	const number largeRadius = radius + rimSnapThreshold;
	const number largeRadiusSq = largeRadius * largeRadius;

//	the cylinder geometry
	vector3 axis;
	VecNormalize(axis, normal);
	vector3 center = aaPos[vrtCenter];

//	recursively select all vertices in the cylinder which can be reached from a
//	selected vertex by following an edge. Start with the given one.
//	We'll also select edges which connect inner with outer vertices. Note that
//	some vertices are considered rim-vertices (those with a distance between
//	smallRadius and largeRadius). Those are neither considered inner nor outer.
	Selector& sel = selOut;
	sel.clear();
	sel.select(vrtCenter);

	stack<Vertex*> vrtStack;
	vrtStack.push(vrtCenter);

	Grid::edge_traits::secure_container edges;
	Grid::face_traits::secure_container faces;
	vector<Quadrilateral*> quads;

	while(!vrtStack.empty()){
		Vertex* curVrt = vrtStack.top();
		vrtStack.pop();

	//	we have to convert associated quadrilaterals to triangles.
	//	Be careful not to alter the array of associated elements while we iterate
	//	over it...
		quads.clear();
		grid.associated_elements(faces, curVrt);
		for(size_t i = 0; i < faces.size(); ++i){
			if(faces[i]->num_vertices() == 4){
				Quadrilateral* q = dynamic_cast<Quadrilateral*>(faces[i]);
				if(q)
					quads.push_back(q);
			}
		}

		for(size_t i = 0; i < quads.size(); ++i){
			Triangulate(grid, quads[i], &aaPos);
		}

	//	now check whether edges leave the cylinder and mark them accordingly.
	//	Perform projection of vertices to the cylinder rim for vertices which
	//	lie in the threshold area.
		grid.associated_elements(edges, curVrt);

		for(size_t i_edge = 0; i_edge < edges.size(); ++i_edge){
			Edge* e = edges[i_edge];
			Vertex* vrt = GetConnectedVertex(e, curVrt);

			if(sel.is_selected(vrt))
				continue;

			vector3 p = aaPos[vrt];
			vector3 proj;
			ProjectPointToRay(proj, p, center, axis);
			number distSq = VecDistanceSq(p, proj);

			if(distSq < smallRadiusSq){
				sel.select(vrt);
				vrtStack.push(vrt);
			}
			else if(distSq < largeRadiusSq){
				sel.select(vrt);
			//	cut the ray from center through p with the cylinder hull to calculate
			//	the new position of vrt.
				vector3 dir;
				VecSubtract(dir, p, center);
				number t0, t1;
				if(RayCylinderIntersection(t0, t1, center, dir, center, axis, radius))
					VecScaleAdd(aaPos[vrt], 1., center, t1, dir);
			}
			else{
			//	the edge will be refined later on
				sel.select(e);
			}
		}
	}

//	refine selected edges and use a special refinement callback, which places
//	new vertices on edges which intersect a cylinder on the cylinders hull.
	CylinderCutProjector refCallback(MakeGeometry3d(grid, aPos),
									 center, axis, radius);
	Refine(grid, sel, aInt, &refCallback);

//	finally select all triangles which lie in the cylinder
	sel.clear();
	vrtStack.push(vrtCenter);
	sel.select(vrtCenter);

	while(!vrtStack.empty()){
		Vertex* curVrt = vrtStack.top();
		vrtStack.pop();
		grid.associated_elements(faces, curVrt);

		for(size_t i_face = 0; i_face < faces.size(); ++i_face){
			Face* f = faces[i_face];
			if(sel.is_selected(f))
				continue;

			sel.select(f);

			for(size_t i = 0; i < f->num_vertices(); ++i){
				Vertex* vrt = f->vertex(i);
				if(!sel.is_selected(vrt)){
					number dist = DistancePointToRay(aaPos[vrt], center, axis);
					if(dist < (radius - SMALL)){
						sel.select(vrt);
						vrtStack.push(vrt);
					}
				}
			}
		}
	}

	sel.clear<Vertex>();

	return true;
}

////////////////////////////////////////////////////////////////////////
bool AdaptSurfaceGridToCylinder(Selector& selOut, Grid& grid,
						   Vertex* vrtCenter, const vector3& normal,
						   number radius, number rimSnapThreshold, APosition& aPos)
{
	AInt aInt;
	grid.attach_to_edges(aInt);
	bool retVal = AdaptSurfaceGridToCylinder(selOut, grid, vrtCenter, normal,
											radius, rimSnapThreshold, aInt, aPos);
	grid.detach_from_edges(aInt);
	return retVal;
}

}//	end of namespace

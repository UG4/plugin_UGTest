/*
 * Copyright (c) 2009-2015:  G-CSC, Goethe University Frankfurt
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

#include <vector>
#include "edge_util.h"
#include "lib_grid/grid/grid_util.h"
#include "vertex_util.h"
#include "face_util.h"
#include "lib_grid/algorithms/debug_util.h"
#include "lib_grid/refinement/regular_refinement.h"
#include "lib_grid/refinement/projectors/plane_cut_projector.h"
#include "common/util/vec_for_each.h"
#include "common/util/vec_for_each.h"

using namespace std;

namespace ug
{

////////////////////////////////////////////////////////////////////////
int GetEdgeIndex(Face* f, Edge* e)
{
	uint numEdges = f->num_edges();
	EdgeDescriptor ed;
	for(uint i = 0; i < numEdges; ++i)
	{
		f->edge_desc(i, ed);
		if(CompareVertices(e, &ed))
			return (int)i;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////
int GetEdgeIndex(Volume* vol, Edge* e)
{
	uint numEdges = vol->num_edges();
	EdgeDescriptor ed;
	for(uint i = 0; i < numEdges; ++i)
	{
		vol->edge_desc(i, ed);
		if(CompareVertices(e, &ed))
			return (int)i;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////
bool IsBoundaryEdge(Grid& grid, Edge* e,
					Grid::face_traits::callback funcIsSurfFace)
{
	int counter = 0;
	if(grid.option_is_enabled(EDGEOPT_STORE_ASSOCIATED_FACES))
	{
		for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(e);
			iter != grid.associated_faces_end(e); ++iter)
		{
			if(funcIsSurfFace(*iter))
				++counter;
			if(counter > 1)
				return false;
		}
	}
	else
	{
	//	fill a vector using a helper function
		vector<Face*> faces;
		CollectFaces(faces, grid, e, false);
		for(size_t i = 0; i < faces.size(); ++i){
			if(funcIsSurfFace(faces[i]))
				++counter;
			if(counter > 1)
				return false;
		}
	}

	if(counter == 1)
			return true;
	return false;
}

////////////////////////////////////////////////////////////////////////
bool IsBoundaryEdge2D(Grid& grid, Edge* e)
{
//	get the number of connected faces. if only one face is connected then
//	the edge is considered to be a boundary edge.
/*	int counter = 0;
	if(grid.option_is_enabled(EDGEOPT_STORE_ASSOCIATED_FACES))
	{
		for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(e);
			iter != grid.associated_faces_end(e); ++iter)
			++counter;

		if(counter == 1)
			return true;
	}
	else
	{
	//	fill a vector using a helper function
		vector<Face*> vFaces;
		CollectFaces(vFaces, grid, e, false);
		if(vFaces.size() == 1)
			return true;
	}
	return false;*/
	return NumAssociatedFaces(grid, e) == 1;
}

////////////////////////////////////////////////////////////////////////
bool IsBoundaryEdge3D(Grid& grid, Edge* e)
{
	if(!grid.option_is_enabled(VOLOPT_AUTOGENERATE_FACES)){
		UG_LOG("WARNING in IsBoundaryEdge3D: Autoenabling VOLOPT_AUTOGENERATE_FACES.\n");
		grid.enable_options(VOLOPT_AUTOGENERATE_FACES);
	}

//	check whether an associated face is a boundary face
	if(grid.option_is_enabled(EDGEOPT_STORE_ASSOCIATED_FACES))
	{
		for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(e);
			iter != grid.associated_faces_end(e); ++iter)
		{
			if(IsBoundaryFace3D(grid, *iter))
				return true;
		}
	}
	else
	{
	//	fill a vector using a helper function
		vector<Face*> vFaces;
		CollectFaces(vFaces, grid, e, false);
		for(size_t i = 0; i < vFaces.size(); ++i){
			if(IsBoundaryFace3D(grid, vFaces[i]))
				return true;
		}
	}
	return false;
}

bool LiesOnBoundary(Grid& grid, Edge* e)
{
//	first check whether the edge is a 2d boundary element
	if((grid.num<Face>() > 0) && IsBoundaryEdge2D(grid, e)){
		return true;
	}

//	since it isn't a 2d boundary element, it might be a 3d boundary element
	if((grid.num<Volume>() > 0) && IsBoundaryEdge3D(grid, e))
		return true;

//	ok - it isn't a boundary element
	return false;
}

////////////////////////////////////////////////////////////////////////
//	GetAssociatedFaces
int GetAssociatedFaces(Face** facesOut, Grid& grid,
						Edge* e, int maxNumFaces)
{
	if(grid.option_is_enabled(EDGEOPT_STORE_ASSOCIATED_FACES))
	{
		int counter = 0;
		Grid::AssociatedFaceIterator iterEnd = grid.associated_faces_end(e);
		for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(e);
			iter != iterEnd; ++iter)
		{
			if(counter < maxNumFaces)
				facesOut[counter] = *iter;

			counter++;
		}
		return counter;
	}
	else
	{
	//	we're using grid::mark for maximal speed.
		//grid.begin_marking();
	//	mark the end-points of the edge
		//grid.mark(e->vertex(0));
		//grid.mark(e->vertex(1));

		Vertex* v0 = e->vertex(0);
		Vertex* v1 = e->vertex(1);

	//	we have to find the triangles 'by hand'
	//	iterate over all associated faces of vertex 0
		int counter = 0;
		Grid::AssociatedFaceIterator iterEnd = grid.associated_faces_end(v0);
		for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(v0);
			iter != iterEnd; ++iter)
		{
			Face* tf = *iter;
			uint numVrts = tf->num_vertices();
			int numMarked = 0;
			for(uint i = 0; i < numVrts; ++i){
				Vertex* v = tf->vertex(i);
				if((v == v0) || (v == v1))
					numMarked++;
			}
			
			if(numMarked > 1)
			{
			//	the face is connected with the edge
				if(counter < maxNumFaces)
					facesOut[counter] = tf;
				counter++;
			}
		}
	//	done with marking
		//grid.end_marking();

		return counter;
	}
}

////////////////////////////////////////////////////////////////////////
//	NumAssociatedFaces
int NumAssociatedFaces(Grid& grid, Edge* e)
{
	if(grid.option_is_enabled(EDGEOPT_STORE_ASSOCIATED_FACES))
	{
		int counter = 0;
		Grid::AssociatedFaceIterator iterEnd = grid.associated_faces_end(e);
		for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(e);
			iter != iterEnd; ++iter)
		{
			counter++;
		}
		return counter;
	}
	else
	{
	//	we're using grid::mark for maximal speed.
		grid.begin_marking();
	//	mark the end-points of the edge
		grid.mark(e->vertex(0));
		grid.mark(e->vertex(1));

	//	we have to find the triangles 'by hand'
	//	iterate over all associated faces of vertex 0
		int counter = 0;
		Vertex* v = e->vertex(0);
		Grid::AssociatedFaceIterator iterEnd = grid.associated_faces_end(v);
		for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(v);
			iter != iterEnd; ++iter)
		{
			Face* tf = *iter;
			uint numVrts = tf->num_vertices();
			int numMarked = 0;
			for(uint i = 0; i < numVrts; ++i){
				if(grid.is_marked(tf->vertex(i)))
					numMarked++;
			}
			
			if(numMarked > 1)
			{
			//	the face is connected with the edge
				counter++;
			}
		}
	//	done with marking
		grid.end_marking();

		return counter;
	}
}

////////////////////////////////////////////////////////////////////////
Edge* GetConnectingEdge(Grid& grid, Face* f1, Face* f2)
{
	Grid::edge_traits::secure_container edges1, edges2;
	grid.associated_elements(edges1, f1);
	grid.associated_elements(edges2, f2);

	for(size_t i = 0; i < edges1.size(); ++i){
		for(size_t j = 0; j < edges2.size(); ++j){
			if(edges1[i] == edges2[j])
				return edges1[i];
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////
int CalculateNormal(vector3& vNormOut, Grid& grid, Edge* e,
					Grid::AttachmentAccessor<Vertex, APosition>& aaPos,
					Grid::AttachmentAccessor<Face, ANormal>* paaNormFACE)
{
	Face* f[2];
	
	int numFaces = GetAssociatedFaces(f, grid, e, 2);
	
	switch(numFaces){
	
	case 0:{ //	if there are no associated faces.
		//	we'll assume that the edge lies in the xy plane and return its normal
		vector3 dir;
		VecSubtract(dir, aaPos[e->vertex(1)], aaPos[e->vertex(0)]);
		vNormOut.x() = dir.y();
		vNormOut.y() = -dir.x();
		vNormOut.z() = 0;
		VecNormalize(vNormOut, vNormOut);
		}break;
	
	case 1: //	if there is one face, the normal will be set to the faces normal
		if(paaNormFACE)
			vNormOut = (*paaNormFACE)[f[0]];
		else{
			CalculateNormal(vNormOut, f[0], aaPos);
		}
		break;
	
	default: //	there are at least 2 associated faces
		if(paaNormFACE)
			VecAdd(vNormOut, (*paaNormFACE)[f[0]], (*paaNormFACE)[f[1]]);
		else{
			vector3 fn0, fn1;
			CalculateNormalNoNormalize(fn0, f[0], aaPos);
			CalculateNormalNoNormalize(fn1, f[1], aaPos);
			VecAdd(vNormOut, fn0, fn1);
		}
		VecNormalize(vNormOut, vNormOut);
		break;
	}
	
	return numFaces;
}

int CalculateNormalNoNormalize(vector3& vNormOut, Grid& grid, Edge* e,
					Grid::VertexAttachmentAccessor<APosition>& aaPos,
					Grid::FaceAttachmentAccessor<ANormal>* paaNormFACE)
{
	Face* f[2];
	
	int numFaces = GetAssociatedFaces(f, grid, e, 2);
	
	switch(numFaces){
	
	case 0:{ //	if there are no associated faces.
		//	we'll assume that the edge lies in the xy plane and return its normal
		vector3 dir;
		VecSubtract(dir, aaPos[e->vertex(1)], aaPos[e->vertex(0)]);
		vNormOut.x() = dir.y();
		vNormOut.y() = -dir.x();
		vNormOut.z() = 0;
		}break;
	
	case 1: //	if there is one face, the normal will be set to the faces normal
		if(paaNormFACE)
			vNormOut = (*paaNormFACE)[f[0]];
		else{
			CalculateNormalNoNormalize(vNormOut, f[0], aaPos);
		}
		break;
	
	default: //	there are at least 2 associated faces
		if(paaNormFACE)
			VecAdd(vNormOut, (*paaNormFACE)[f[0]], (*paaNormFACE)[f[1]]);
		else{
			vector3 fn0, fn1;
			CalculateNormalNoNormalize(fn0, f[0], aaPos);
			CalculateNormalNoNormalize(fn1, f[1], aaPos);
			VecAdd(vNormOut, fn0, fn1);
			VecScale(vNormOut, vNormOut, 0.5);
		}
		VecNormalize(vNormOut, vNormOut);
		break;
	}
	
	return numFaces;
}
					
////////////////////////////////////////////////////////////////////////
bool CollapseEdge(Grid& grid, Edge* e, Vertex* newVrt)
{
//	prepare the grid, so that we may perform Grid::replace_vertex.
//	create collapse geometries first and delete old geometries.
	Grid::face_traits::secure_container assFaces;
	if(grid.num_faces() > 0){
	//	collect adjacent faces
		grid.associated_elements(assFaces, e);

	//	a vector thats is used to store the collapse-geometries.
		vector<Face*> vNewFaces;

		for(uint i = 0; i < assFaces.size(); ++i)
		{

			Face* f = assFaces[i];
			int eInd = GetEdgeIndex(f, e);

		//	create the collapse-geometry
			f->collapse_edge(vNewFaces, eInd, newVrt);

		//	register the new faces
			for(uint j = 0; j < vNewFaces.size(); ++j)
				grid.register_element(vNewFaces[j], f);

		//	before we erase the face, we first have to notify whether
		//	edges were merged
			if(f->num_edges() == 3){
			//	two edges will be merged. we have to inform the grid.
				Vertex* conVrt = GetConnectedVertex(e, f);
			//	now get the edge between conVrt and newVrt
				Edge* target = grid.get_edge(conVrt, newVrt);
			//	now get the two old edges
				Edge* e1 = grid.get_edge(f, (eInd + 1) % 3);
				Edge* e2 = grid.get_edge(f, (eInd + 2) % 3);
				grid.objects_will_be_merged(target, e1, e2);
			}
		}
	}

	Grid::volume_traits::secure_container assVols;
	if(grid.num_volumes() > 0){
	//	used to identify merge-faces
		Grid::edge_traits::secure_container assEdges;

	//	collect adjacent volumes
		grid.associated_elements(assVols, e);

	//	a vector thats used to store the collapse-geometries.
		vector<Volume*> vNewVolumes;

		for(uint i = 0; i < assVols.size(); ++i)
		{
			Volume* v = assVols[i];
		//	create the collapse-geometry
			int eInd = GetEdgeIndex(v, e);
			v->collapse_edge(vNewVolumes, eInd, newVrt);

		//	register the new volumes
			for(uint j = 0; j < vNewVolumes.size(); ++j)
				grid.register_element(vNewVolumes[j], v);

		//	if v is a tetrahedron, two faces will be merged
			if(v->num_vertices() == 4){
			//	find the opposing edge of e
				Edge* oppEdge = NULL;
				grid.associated_elements(assEdges, v);
				for_each_in_vec(Edge* te, assEdges){
					if(!GetSharedVertex(e, te)){
						oppEdge = te;
						break;
					}
				}end_for;

				if(oppEdge){
					Face* f0 = grid.get_face(FaceDescriptor(
										e->vertex(0),
										oppEdge->vertex(0),
										oppEdge->vertex(1)));
					Face* f1 = grid.get_face(FaceDescriptor(
										e->vertex(1),
										oppEdge->vertex(0),
										oppEdge->vertex(1)));
					Face* fNew = grid.get_face(FaceDescriptor(
										newVrt,
										oppEdge->vertex(0),
										oppEdge->vertex(1)));
					if(f0 && f1 && fNew)
						grid.objects_will_be_merged(fNew, f0, f1);
				}
			}
		}
	}
	
//	store the end-points of e
	Vertex* v[2];
	v[0] = e->vertex(0);
	v[1] = e->vertex(1);

//	notify the grid that the endpoints will be merged
	grid.objects_will_be_merged(newVrt, v[0], v[1]);

//	erase e, associated faces and associated volumes
	for_each_in_vec(Volume* v, assVols){
		grid.erase(v);
	}end_for;
	
	for_each_in_vec(Face* f, assFaces){
		grid.erase(f);
	}end_for;

	grid.erase(e);

//	perform replace_vertex for both endpoints
	for(uint i = 0; i < 2; ++i)
	{
		if(v[i] != newVrt)
			grid.replace_vertex(v[i], newVrt);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////
bool EdgeCollapseIsValid(Grid& grid, Edge* e)
{
//TODO: Make sure that this is sufficient for geometries including Quads.
//TODO: Check validity for volumes.
//	in order to not destroy the topology of the grid, a collapse is
//	only valid if no three edges build a triangle that does not exist
//	in the grid.

//	first we need all vertices that are connected with end-points of e.
	vector<Vertex*> vVertices1;
	vector<Vertex*> vVertices2;

	CollectNeighbors(vVertices1, grid, e->vertex(0)); // e->vertex(0) is not contained in vVertices1!
	CollectNeighbors(vVertices2, grid, e->vertex(1)); // e->vertex(1) is not contained in vVertices2!

//	we need access to the triangles connected with e.
	vector<Face*> vFaces;
	CollectFaces(vFaces, grid, e);

//	this face descriptor will be needed during the algorithm.
	FaceDescriptor fd(3);
	fd.set_vertex(0, e->vertex(0));
	fd.set_vertex(1, e->vertex(1));

//	now check for each vertex in vVertices1 if it also exists in vVertices2.
	for(uint i = 0; i < vVertices1.size(); ++i)
	{
		Vertex* v1 = vVertices1[i];

		if(v1 != e->vertex(1))
		{
			for(uint j = 0; j < vVertices2.size(); ++j)
			{
				Vertex* v2 = vVertices2[j];

				if(v1 == v2)
				{
				//	v1 and v2 exist in both arrays.
				//	check if a triangle exists that connects e with v1.
				//	it is sufficient to search in vFaces.
					bool bGotOne = false;
					fd.set_vertex(2, v1);
					for(uint k = 0; k < vFaces.size(); ++k)
					{
						if(CompareVertices(vFaces[k], &fd))
						{
							bGotOne = true;
							break;
						}
					}

				//	if there was none, the collapse would be illegal.
					if(!bGotOne)
						return false;
				}
			}
		}
	}

//	everything is fine.
	return true;
}

////////////////////////////////////////////////////////////////////////
bool CreateEdgeSplitGeometry(Grid& destGrid, Grid& srcGrid, Edge* e,
							Vertex* newVertex, AVertex* paAssociatedVertices)
{

	if((paAssociatedVertices == NULL) && (&destGrid != &srcGrid))
		return false;

	vector<Vertex*> vVrts;

//	If paAssociatedVertices is specified, we'll have to find the vertices
//	in destGrid for each element that is adjacent to e. We then have
//	to store these vertices in vVrts and pass them to the
//	split-routine of the element.

//	the attachment-accessor
	Grid::VertexAttachmentAccessor<AVertex> aaAssociatedVertices;

	if(paAssociatedVertices != NULL)
	{
		AVertex& aAssociatedVertices = *paAssociatedVertices;

	//	check if aVertex is properly attached.
		if(!srcGrid.has_vertex_attachment(aAssociatedVertices))
		//	attach it and initialize its values.
			srcGrid.attach_to_vertices_dv(aAssociatedVertices, NULL, false);

	//	initialize the attachment-accessor
		aaAssociatedVertices.access(srcGrid, aAssociatedVertices);
	}

//	we will store the substitute-vertices in this vector - if they are needed.
	vector<Vertex*> vSubstituteVertices;

//	split the edge
	{
	//	simply create two new edges
		Edge* parent = e;
	//	the grids do not match then we can't pass e as a parent
		if(&srcGrid != &destGrid)
			parent = NULL;
			
		if(paAssociatedVertices){
			destGrid.create<RegularEdge>(EdgeDescriptor(aaAssociatedVertices[e->vertex(0)], newVertex), parent);
			destGrid.create<RegularEdge>(EdgeDescriptor(newVertex, aaAssociatedVertices[e->vertex(1)]), parent);
		}
		else{
			destGrid.create<RegularEdge>(EdgeDescriptor(e->vertex(0), newVertex), parent);
			destGrid.create<RegularEdge>(EdgeDescriptor(newVertex, e->vertex(1)), parent);
		}
	}

//	split faces
	{
	//	collect all faces which are associated with e
		vector<Face*> vOldFaces;
		CollectFaces(vOldFaces, srcGrid, e, false);

	//	we will collect new faces in this vector
		vector<Face*> vNewFaces;

	//	iterate through those faces and split each.
	//	If vertices are missing in destGrid, create them first.
		for(vector<Face*>::iterator oldFaceIter = vOldFaces.begin();
			oldFaceIter != vOldFaces.end(); ++oldFaceIter)
		{
			Face* oldFace = *oldFaceIter;
			uint numVrts = oldFace->num_vertices();

		//	get the index of e in oldFace
			int edgeIndex = GetEdgeIndex(oldFace, e);

		//	clear the new faces-vec
			vNewFaces.clear();

		//	get the substitute-vertices if they are required
			if(paAssociatedVertices != NULL)
			{
				if(numVrts > vSubstituteVertices.size())
					vSubstituteVertices.resize(numVrts);

			//	check for each vertex in oldFace, if an associated vertex exists in destGrid.
			//	if not create a new one by cloning the associated one in srcGrid.
			//	store the vertices in vSubstituteVertices
				{
					for(uint i = 0; i < numVrts; ++i)
					{
						if(aaAssociatedVertices[oldFace->vertex(i)] == NULL)
							aaAssociatedVertices[oldFace->vertex(i)] = *destGrid.create_by_cloning(oldFace->vertex(i));
						vSubstituteVertices[i] = aaAssociatedVertices[oldFace->vertex(i)];
					}
				}

			//	create the new faces by splitting the old face. use substitutes.
				oldFace->create_faces_by_edge_split(edgeIndex, newVertex, vNewFaces, &vSubstituteVertices.front());
			}
			else
			{
			//	create the new faces by splitting the old face.
			//	no substitutes required
				oldFace->create_faces_by_edge_split(edgeIndex, newVertex, vNewFaces);
			}

		//	register all new faces at destGrid
			{
				Face* pParent = NULL;
				if(&srcGrid == &destGrid)
					pParent = oldFace;

				for(vector<Face*>::iterator iter = vNewFaces.begin();
					iter != vNewFaces.end(); ++iter)
				{
					destGrid.register_element(*iter, pParent);
					if(pParent)
						destGrid.pass_on_values(pParent, *iter);
				}
			}
		}
	}
	
//	split volumes
	if(srcGrid.num<Volume>() > 0)
	{
	//	this vector will be used to specify on which edge a vertex has to be inserted
		vector<Vertex*> edgeVrts;

	//	collect all volumes associated with the edge
		vector<Volume*> vols, newVols;
		CollectVolumes(vols, srcGrid, e);
		
		for(size_t i_vols = 0; i_vols < vols.size(); ++i_vols)
		{
			Volume* oldVol = vols[i_vols];
			uint numVrts = oldVol->num_vertices();

			newVols.clear();
			
		//	get the index of e in oldFace and fill the edgeVrts array
			edgeVrts.clear();
			for(size_t i_edge = 0; i_edge < oldVol->num_edges(); ++i_edge)
			{
				if(srcGrid.get_edge(oldVol, i_edge) == e)
					edgeVrts.push_back(newVertex);
				else
					edgeVrts.push_back(NULL);
			}
		
		//	if refine creates a new vertex in the center of the volume,
		//	it will be stored in this var.
			Vertex* newVolVrt = NULL;
			
		//	get the substitute-vertices if they are required
			if(paAssociatedVertices != NULL)
			{
				if(numVrts > vSubstituteVertices.size())
					vSubstituteVertices.resize(numVrts);

			//	check for each vertex in oldFace, if an associated vertex exists in destGrid.
			//	if not create a new one by cloning the associated one in srcGrid.
			//	store the vertices in vSubstituteVertices
				{
					for(uint i = 0; i < numVrts; ++i)
					{
						if(aaAssociatedVertices[oldVol->vertex(i)] == NULL)
							aaAssociatedVertices[oldVol->vertex(i)] = *destGrid.create_by_cloning(oldVol->vertex(i));
						vSubstituteVertices[i] = aaAssociatedVertices[oldVol->vertex(i)];
					}
				}

			//	create the new faces by splitting the old face. use substitutes.
				oldVol->refine(newVols, &newVolVrt, &edgeVrts.front(), NULL, NULL,
							   RegularVertex(), &vSubstituteVertices.front());
			}
			else
			{
			//	create the new faces by splitting the old face.
			//	no substitutes required
				oldVol->refine(newVols, &newVolVrt, &edgeVrts.front(), NULL, NULL,
							   RegularVertex(), NULL);
			}

		//	register all new vertices and volumes at destGrid
			{
				Volume* pParent = NULL;
				if(&srcGrid == &destGrid)
					pParent = oldVol;
				
				if(newVolVrt)
					destGrid.register_element(newVolVrt, pParent);
					
				for(vector<Volume*>::iterator iter = newVols.begin();
					iter != newVols.end(); ++iter)
				{
					destGrid.register_element(*iter, pParent);
					if(pParent)
						destGrid.pass_on_values(pParent, *iter);
				}
			}
		}
	}
	return true;
}

Edge* SwapEdge(Grid& grid, Edge* e)
{
//	get the associated faces.
//	Only 2 associated faces are allowed.
	Face* f[2];
	if(GetAssociatedFaces(f, grid, e, 2) != 2){
		UG_LOG("Swap Failed: #neighbor-faces != 2.\n");
		return NULL;
	}

//	make sure that both faces are triangles
	if((f[0]->num_vertices() != 3) || (f[1]->num_vertices() != 3)){
		UG_LOG("Swap Failed: At least one neighbor-face is not a triangle.\n");
		return NULL;
	}

//	begin marking
	grid.begin_marking();

//	mark the end-points of the edge
	grid.mark(e->vertex(0));
	grid.mark(e->vertex(1));	

//	get the two vertices that will be connected by the new edge
	Vertex* v[2];
	int vrtInd[2];
	for(int j = 0; j < 2; ++j){
		v[j] = NULL;
		for(int i = 0; i < 3; ++i){
			Vertex* vrt = f[j]->vertex(i);
			if(!grid.is_marked(vrt)){
				vrtInd[j] = i;
				v[j] = vrt;
				break;
			}
		}
	}

//	we're done with marking
	grid.end_marking();

//	make sure that both vertices have been found.
	if(!(v[0] && v[1])){
		UG_LOG("Swap Failed: connected vertices were not found correctly.\n");
		return NULL;
	}

//	make sure that no edge exists between v[0] and v[1]
	if(grid.get_edge(v[0], v[1])){
		UG_LOG("Swap Failed: New edge already exists in the grid.\n");
		return NULL;
	}

//	the indices of the marked points in the first triangle
	int ind1 = (vrtInd[0] + 1) % 3;
	int ind2 = (vrtInd[0] + 2) % 3;

//	create the new edge
	Edge* nEdge = *grid.create_by_cloning(e, EdgeDescriptor(v[0], v[1]), e);

//	create the new faces
	grid.create<Triangle>(TriangleDescriptor(v[0], f[0]->vertex(ind1), v[1]), f[0]);
	grid.create<Triangle>(TriangleDescriptor(f[0]->vertex(ind2), v[0], v[1]), f[1]);

//	erase the old faces
	grid.erase(f[0]);
	grid.erase(f[1]);

//	erase the old edge
	grid.erase(e);
	
	return nEdge;
}

////////////////////////////////////////////////////////////////////////
bool CutEdgesWithPlane(Selector& sel, const vector3& p, const vector3& n,
						APosition& aPos)
{
	if(!sel.grid()){
		UG_LOG("ERROR in CutEdgesWithPlane: sel has to be assigned to a grid.\n");
		return false;
	}
	
	Grid& grid = *sel.grid();
	
	if(!grid.has_vertex_attachment(aPos)){
		UG_LOG("ERROR in CutEdgesWithPlane: aPos has to be attached to the vertices of the grid.\n");
		return false;
	}
	
	Grid::VertexAttachmentAccessor<APosition> aaPos(grid, aPos);
	
//	used for plane-intersection
	number t;
	vector3 v;
	
//	iterate through all edges and deselect all that do not intersect the plane
//	deselect all vertices, faces and volumes, too.
	sel.clear<Vertex>();
	sel.clear<Face>();
	sel.clear<Volume>();
		
	EdgeIterator iter = sel.begin<Edge>();
	while(iter != sel.end<Edge>()){
		Edge* e = *iter;
		iter++;
	
	//	check whether the edge intersects the plane
		vector3 rayDir;
		VecSubtract(rayDir, aaPos[e->vertex(1)], aaPos[e->vertex(0)]);
		
		bool bIntersect = RayPlaneIntersection(v, t, aaPos[e->vertex(0)],
												rayDir, p, n);
		if(!bIntersect || t < SMALL || t > 1. - SMALL)
		{
		//	it doesn't. Remove it from the selector
			sel.deselect(e);
		}
	}
	
//	refine all selected edges. RefinementCallbackEdgePlaneCut will insert
//	new vertices on the plane.
	PlaneCutProjector planeCutProjector(MakeGeometry3d(grid, aPos), p, n);
	const bool success = Refine(grid, sel, &planeCutProjector);

//	deselect all elements and edges which do not connect two selected vertices
	if(success){
	//	deselect all vertices which are not very close to the plane
		VertexIterator vrtIter = sel.begin<Vertex>();
		while(vrtIter != sel.end<Vertex>()){
			Vertex* vrt = *vrtIter;
			++vrtIter;
			if(DistancePointToPlane(aaPos[vrt], p, n) > SMALL)
				sel.deselect(vrt);
		}

		EdgeIterator iter = sel.begin<Edge>();
		while(iter != sel.end<Edge>()){
			Edge* e = *iter;
			iter++;
			if(!(sel.is_selected(e->vertex(0)) && sel.is_selected(e->vertex(1)))){
				sel.deselect(e);
			}
		}

		sel.clear<Face>();
		sel.clear<Volume>();
	}

	return success;
}

}//	end of namespace

/*
 * Copyright (c) 2014-2015:  G-CSC, Goethe University Frankfurt
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

#include <fstream>
#include <queue>
#include "lib_grid/lg_base.h"
#include "common/profiler/profiler.h"
#include "simple_grid.h"
#include "edge_length_adjustment_extended.h"
#include "lib_grid/refinement/regular_refinement.h"
#include "common/node_tree/node_tree.h"
#include "lib_grid/algorithms/trees/octree.h"
#include "lib_grid/callbacks/callbacks.h"
#include "../trees/octree.h"

using namespace std;

namespace ug
{


AdjustEdgeLengthDesc::
AdjustEdgeLengthDesc() :
	minEdgeLen(1),
	maxEdgeLen(2),
	approximation(0.9),
	triQuality(0.9),
	// adaptivity(0.5),
	projectPoints(true)
{}


///	only for debugging purposes!!!
/**	Output value pairs to gnuplot...
 * \{ */
//#define EDGE_LENGTH_ADJUSTMENT__GPLOT_ENABLED
#ifdef EDGE_LENGTH_ADJUSTMENT__GPLOT_ENABLED
	typedef vector<pair<number, number> > GnuplotData;
	static GnuplotData gplotLengthFac;
	static GnuplotData gplotMinCurvature;
	static GnuplotData gplotAverageCurvature;

	void WriteGnuplotData(const char* filename, const GnuplotData& data)
	{
		ofstream out(filename);
		if(!out)
			return;

		for(size_t i = 0; i < data.size(); ++i)
			out << data[i].first << " " << data[i].second << endl;

		out.close();
	}

	#define GPLOTPOINT(dataName, x, y) dataName.push_back(make_pair<number, number>((x), (y)));
	#define GPLOTSAVE()	{WriteGnuplotData("length_fac.gplot", gplotLengthFac);\
						WriteGnuplotData("min_curvature.gplot", gplotMinCurvature);\
						WriteGnuplotData("average_curvature.gplot", gplotAverageCurvature);}
#else
//	do nothing if EDGE_LENGTH_ADJUSTMENT__GPLOT_ENABLED is false
	#define GPLOTPOINT(dataName, x, y)
	#define GPLOTSAVE()
#endif
/** \} */


/*
vector3 PNTrianglePos(const vector3& p0, const vector3& p1, const vector3& p2,
					  const vector3& n0, const vector3& n1, const vector3& n2);

vector3 PNTriangleNorm(const vector3& p0, const vector3& p1, const vector3& p2,
					   const vector3& n0, const vector3& n1, const vector3& n2);

vector3 PNCTrianglePos(const vector3& p0, const vector3& p1, const vector3& p2,
						const vector3& n0, const vector3& n1, const vector3& n2,
						const vector3& cn0, const vector3& cn1, const vector3& cn2);

vector3 PNCTriangleNorm(const vector3& p0, const vector3& p1, const vector3& p2,
						const vector3& n0, const vector3& n1, const vector3& n2,
						const vector3& cn0, const vector3& cn1, const vector3& cn2);


		vector3 pos(number bc0, number bc1);
		vector3 norm(number bc0, number bc1);
};

class ProjectedVertex{
	Vertex*	vertex();
	vector3 vertex_position();
	vector3 vertex_normal();
	vector3 surface_normal();
	vector3 surface_position();
	size_t num_barycentric_coords();
	number barycentric_coord(size_t index);
};

class SurfaceRepresentation{
	public:

};
*/

////////////////////////////////////////////////////////////////////////
static void AssignFixedVertices(Grid& grid, SubsetHandler& shMarks)
{	
	grid.begin_marking();
	
//	mark all vertices that are not regular crease-vertices as fixed
	for(EdgeIterator iter = shMarks.begin<Edge>(REM_CREASE);
		iter != shMarks.end<Edge>(REM_CREASE); ++iter)
	{
		Edge* e = *iter;
		for(size_t i = 0; i < 2; ++i){
			Vertex* vrt = e->vertex(i);
			if(!grid.is_marked(vrt)){
				grid.mark(vrt);
				int counter = 0;
				for(Grid::AssociatedEdgeIterator nbIter = grid.associated_edges_begin(vrt);
					nbIter != grid.associated_edges_end(vrt); ++nbIter)
				{
					if(shMarks.get_subset_index(*nbIter) != REM_NONE)
						++counter;
				}
				
				if(counter != 2)
					shMarks.assign_subset(vrt, REM_FIXED);
			}
		}
	}
	
	grid.end_marking();
	
//	mark all vertices that lie on a fixed edge as fixed vertex
	for(EdgeIterator iter = shMarks.begin<Edge>(REM_FIXED);
		iter != shMarks.end<Edge>(REM_FIXED); ++iter)
	{
		Edge* e = *iter;
		shMarks.assign_subset(e->vertex(0), REM_FIXED);
		shMarks.assign_subset(e->vertex(1), REM_FIXED);
	}
}

static void AssignCreaseVertices(Grid& grid, SubsetHandler& shMarks)
{
//	mark all vertices that lie on a crease and which are not fixed
//	as crease vertices.
	if(shMarks.num_subsets() <= REM_CREASE)
		return;

	for(EdgeIterator iter = shMarks.begin<Edge>(REM_CREASE);
		iter != shMarks.end<Edge>(REM_CREASE); ++iter)
	{
		Edge* e = *iter;
		for(uint i = 0; i < 2; ++i)
			if(shMarks.get_subset_index(e->vertex(i)) != REM_FIXED)
				shMarks.assign_subset(e->vertex(i), REM_CREASE);
	}
}


////////////////////////////////////////////////////////////////////////
template <class TVertexPositionAccessor>
static number CalculateNormalDot(TriangleDescriptor& td1, TriangleDescriptor& td2,
						  TVertexPositionAccessor& aaPos)
{
	vector3 n1;
	CalculateTriangleNormal(n1, aaPos[td1.vertex(0)],
							aaPos[td1.vertex(1)], aaPos[td1.vertex(2)]);
	vector3 n2;
	CalculateTriangleNormal(n2, aaPos[td2.vertex(0)],
							aaPos[td2.vertex(1)], aaPos[td2.vertex(2)]);
	return VecDot(n1, n2);
}


// ////////////////////////////////////////////////////////////////////////
// //	CalculateCurvature
// template <class TAAPosVRT>
// number CalculateMinCurvature(Grid& grid, SubsetHandler& shMarks,
// 							Vertex* vrt, TAAPosVRT& aaPos)
// {
// //TODO:	check whether static vNormals brings any benefits.
// //TODO:	special cases for crease vertices

// //	face normals
// 	static vector<vector3>	vNormals;
// //	vertex normal (mean face normal)
// 	vector3 n(0, 0, 0);
	
// //	calculate the normals of associated faces
// 	vNormals.clear();
// 	Grid::AssociatedFaceIterator iterEnd = grid.associated_faces_end(vrt);
// 	for(Grid::AssociatedFaceIterator iter = grid.associated_faces_begin(vrt);
// 		iter != iterEnd; ++iter)
// 	{
// 		vector3 nTmp;
// 		CalculateNormal(nTmp, *iter, aaPos);
// 		vNormals.push_back(nTmp);
// 		VecAdd(n, n, nTmp);
// 	}

// //	the vertex normal
// 	VecNormalize(n, n);
	
// //	get the min dot-product of the vertex normal with associated faces-normals
// 	number minDot = 1;
// 	for(size_t i = 0; i < vNormals.size(); ++i)
// 		minDot = std::min(minDot, VecDot(n, vNormals[i]));

// //todo:	Think about converting to radiants.
// 	//minDot = acos(minDot) / (0.5 * PI);
// 	//...
// //	done
// 	GPLOTPOINT(gplotMinCurvature, 0, minDot);
// 	return minDot;	
// }							

// ////////////////////////////////////////////////////////////////////////
// template <class TAAPosVRT>
// number CalculateAverageCurvature(Grid& grid, SubsetHandler& shMarks,
// 								Edge* e, TAAPosVRT& aaPos)
// {
// 	number avCurv = 0.5 * (CalculateMinCurvature(grid, shMarks, e->vertex(0), aaPos)
// 				+ CalculateMinCurvature(grid, shMarks, e->vertex(1), aaPos));
// 	GPLOTPOINT(gplotAverageCurvature, 0.5, avCurv);
// 	return avCurv;
// }

// ////////////////////////////////////////////////////////////////////////
// template <class TAAPosVRT>
// number CalculateLengthFac(Grid& grid, SubsetHandler& shMarks,
// 								Edge* e, TAAPosVRT& aaPos)
// {
// 	number lenFac = CalculateAverageCurvature(grid, shMarks, e, aaPos);
// 	lenFac = (lenFac - 0.95) / 0.05;
// 	lenFac = max(number(0.25), lenFac);
// 	GPLOTPOINT(gplotLengthFac, 0.5, lenFac);
// 	return lenFac;
// }

// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// template <class TAAPosVRT, class TAANormVRT, class TAAIntVRT>
// bool TrySwap(const AdjustEdgeLengthDesc& desc, Grid& grid, Edge* e,
// 			TAAPosVRT& aaPos, TAANormVRT& aaNorm,
// 			TAAIntVRT& aaInt, SubsetHandler* pshMarks = NULL,
// 			EdgeSelector* pCandidates = NULL)
// {
// //	swaps are neither allowed for crease edges nor for fixed edges
// 	if(pshMarks){
// 		if(pshMarks->get_subset_index(e) == REM_FIXED ||
// 			pshMarks->get_subset_index(e) == REM_CREASE)
// 			return false;
// 	}

// //	get the associated faces. we need two of them
// 	Face* f[2];
// 	if(GetAssociatedFaces(f, grid, e, 2) != 2)
// 		return false;

// //	make sure both faces are triangles
// 	if((f[0]->num_vertices() != 3) || (f[1]->num_vertices() != 3))
// 		return false;

// //	create a simple grid
// 	SimpleGrid sg;
// 	if(!ObtainSimpleGrid(sg, grid, e->vertex(0), e->vertex(1), 0,
// 						aaPos, aaNorm, aaInt))
// 	{
// 		LOG("ObtainSimpleGrid failed. ignoring edge...\n");
// 		return false;
// 	}

// //	calculate geometric-approximation-degree and triangle quality
// 	number approxDeg = GeometricApproximationDegree(sg);
// 	number shapeDeg = ShapeQualityDegree(sg);
// 	// number smoothDeg = VecDot(sg.triangleNormals[0], sg.triangleNormals[1]);

// //	this is a new test. the idea is that each edge should be orthogonal to
// //	the normals of its endpoints - at least in a perfectly smooth surface.

// 	// number approxDeg;
// 	// number newApproxDeg;
// 	// {
// 	// 	vector3 v;
// 	// 	VecSubtract(v, sg.vertices[1], sg.vertices[0]);
// 	// 	VecNormalize(v, v);
// 	// 	approxDeg = 1. - 0.5*(fabs(VecDot(v, sg.vertexNormals[0])) +
// 	// 						fabs(VecDot(v, sg.vertexNormals[1])));
						
// 	// 	VecSubtract(v, sg.vertices[3], sg.vertices[2]);
// 	// 	VecNormalize(v, v);
// 	// 	newApproxDeg = 1. - 0.5*(fabs(VecDot(v, sg.vertexNormals[2])) +
// 	// 							fabs(VecDot(v, sg.vertexNormals[3])));
// 	// }

// //	perform a swap on the simple grid
// 	if(!SwapEdge(sg))
// 	{
// 		LOG("swap edge failed...\n");
// 		return false;
// 	}

// //	calculate new geometric-approximation-degree and triangle quality
// 	number newApproxDeg = GeometricApproximationDegree(sg);
// 	number newShapeDeg = ShapeQualityDegree(sg);
// 	// number newSmoothDeg = VecDot(sg.triangleNormals[0], sg.triangleNormals[1]);

// // //	neither the shapeDeg nor the approxDeg may get too bad.
// // 	if((newApproxDeg < 0.5 * approxDeg) || (newShapeDeg < 0.5 * shapeDeg))
// // 		return false;
	
// // //	make sure that the swap does not destroy the smoothness
// // 	if(newSmoothDeg < 0.1 * smoothDeg)
// // 		return false;

// 	// if(0.2 * (newApproxDeg - approxDeg) + 0.8 * (newShapeDeg - shapeDeg) > 0)
// 	//if(newShapeDeg > shapeDeg)
// 	//if(newApproxDeg > approxDeg)


// 	if((newApproxDeg > desc.approximation * approxDeg) ||
// 		(newApproxDeg >= desc.approximation && newShapeDeg > shapeDeg))
// 	{
// 	//	swap the edge
// 		e = SwapEdge(grid, e);

// 		if(e){
// 		//	swap was successful
// 		//	if pCandidates was specified then add new candidates
// 			if(pCandidates){
// 				for(int i = 0; i < 2; ++i)
// 					pCandidates->select(grid.associated_edges_begin(e->vertex(i)),
// 										grid.associated_edges_end(e->vertex(i)));
// 			//	e was selected but is not really a candidate
// 				pCandidates->deselect(e);
// 			}
			
// 			return true;
// 		}
// 	}
	
// 	return false;
// }

// ////////////////////////////////////////////////////////////////////////
// template <class TAAPosVRT, class TAANormVRT, class TAAIntVRT>
// bool PerformSwaps(const AdjustEdgeLengthDesc& desc, Grid& grid,
// 				SubsetHandler& shMarks, EdgeSelector& esel,
// 				TAAPosVRT& aaPos, TAANormVRT& aaNorm, TAAIntVRT& aaInt)
// {	
// 	PROFILE_FUNC();
// 	LOG("  performing swaps\n");
// 	int numSwaps = 0;
// 	int maxNumSwaps = esel.num<Edge>() * 2;
		
// 	while(!esel.empty())
// 	{
// 		Edge* e = *esel.begin<Edge>();
// 		esel.deselect(e);

// 		if(TrySwap(desc, grid, e, aaPos, aaNorm, aaInt, &shMarks, &esel)){
// 			++numSwaps;
// 			if(numSwaps > maxNumSwaps){
// 				UG_LOG("  aborting since maxNumSwaps was reached...");
// 				esel.clear();
// 			}
// 		}
// 	}
// 	LOG("  swaps performed: " << numSwaps << endl);
	
// 	return true;
// }

/**	returns the resulting vertex or NULL, if no collapse was performed.*/
template <class TAAPosVRT, class TAANormVRT, class TAAIntVRT>
static Vertex* TryCollapse(const AdjustEdgeLengthDesc& desc, Grid& grid, Edge* e,
				TAAPosVRT& aaPos, TAANormVRT& aaNorm, 
				TAAIntVRT& aaInt, SubsetHandler* pshMarks = NULL,
				EdgeSelector* pCandidates = NULL)
{
	if(pshMarks)
	{
		SubsetHandler& shMarks = *pshMarks;
	//	collapses are not allowed for fixed edges
		if(shMarks.get_subset_index(e) == REM_FIXED)
			return NULL;
			
	//	if both endpoints of are fixed vertices then
	//	we may not collapse
		int vrtSI[2];
		vrtSI[0] = shMarks.get_subset_index(e->vertex(0));
		vrtSI[1] = shMarks.get_subset_index(e->vertex(1));

		if((vrtSI[0] == REM_FIXED) && (vrtSI[1] == REM_FIXED))
			return NULL;

	//	if both endpoints are somehow marked, e has to be a
	//	crease edge
		if((vrtSI[0] != REM_NONE) && (vrtSI[1] != REM_NONE)
			&&	(shMarks.get_subset_index(e) != REM_CREASE))
			return NULL;
	}

//	check whether the edge can be collapsed
	if(EdgeCollapseIsValid(grid, e))
	{
	//	test the collapse using a simple-grid
		SimpleGrid sgSrc;
		if(!ObtainSimpleGrid(sgSrc, grid, e->vertex(0), e->vertex(1), 1,
							aaPos, aaNorm, aaInt))
		{
			LOG("ObtainSimpleGrid failed. ignoring edge...\n");
			return NULL;
		}
		
	//	calculate geometric-approximation-degree and triangle quality
		number approxDeg = GeometricApproximationDegree(sgSrc);
		number minApproxDeg = desc.approximation * approxDeg;
		number shapeDeg = ShapeQualityDegree(sgSrc);

	//	perform a collapse on the simple grid
		SimpleGrid sgDest;
		if(!ObtainSimpleGrid_CollapseEdge(sgDest, grid, e,
									1, aaPos, aaNorm, aaInt))
		{
			LOG("collapse edge failed...\n");
			return NULL;
		}

	//	get the positions of the old endpoints
		static const int numTestPositions = 3;
		int newInd = 0;
		vector3 v[numTestPositions];
		v[0] = aaPos[e->vertex(0)];
		v[1] = aaPos[e->vertex(1)];
		v[2] = sgDest.vertices[newInd];

		vector3 n[numTestPositions];
		n[0] = aaNorm[e->vertex(0)];
		n[1] = aaNorm[e->vertex(1)];
		n[2] = sgDest.vertexNormals[newInd];
		
	//	we'll compare 3 approximation degrees and three shape degrees
		number newApproxDeg[numTestPositions];
		number newShapeDeg[numTestPositions];
		
	//	check which position is the best
		int bestIndex = -1;
	//	the vertex subset index is used to support marks (crease and fixed vertices)
		int vrtSI[2];
		vrtSI[0] = vrtSI[1] = REM_NONE;

		if(pshMarks)
		{
			vrtSI[0] = pshMarks->get_subset_index(e->vertex(0));
			vrtSI[1] = pshMarks->get_subset_index(e->vertex(1));

			if((vrtSI[0] == REM_FIXED) || ((vrtSI[0] != REM_NONE) && (vrtSI[1] == REM_NONE))){
				bestIndex = 0;
				sgDest.vertices[newInd] = v[0];
				sgDest.vertexNormals[newInd] = n[0];
				newApproxDeg[0] = GeometricApproximationDegree(sgDest);
				newShapeDeg[0] = ShapeQualityDegree(sgDest);
			}
			else if((vrtSI[1] == REM_FIXED) || ((vrtSI[1] != REM_NONE) && (vrtSI[0] == REM_NONE))){
				bestIndex = 1;
				sgDest.vertices[newInd] = v[1];
				sgDest.vertexNormals[newInd] = n[1];
				newApproxDeg[1] = GeometricApproximationDegree(sgDest);
				newShapeDeg[1] = ShapeQualityDegree(sgDest);
			}
		}

		if(bestIndex == -1){
		//	check all three approximation degrees
			for(int i = 0; i < numTestPositions; ++i){
			//	we'll compute all qualities with the averaged normal
				sgDest.vertices[newInd] = v[i];
				sgDest.vertexNormals[newInd] = n[i];
				CalculateTriangleNormals(sgDest);
				newApproxDeg[i] = GeometricApproximationDegree(sgDest);
				newShapeDeg[i] = ShapeQualityDegree(sgDest);
			}
		//	get the best one
			bestIndex = -1;

			for(int i = 0; i < numTestPositions; ++i){
				if(newApproxDeg[i] >= minApproxDeg){
					if(bestIndex == -1)
						bestIndex = i;
				 	else if(newShapeDeg[i] > newShapeDeg[bestIndex])
				 		bestIndex = i;
				}
			}

			// for(int i = 1; i < numTestPositions; ++i){
			// 	if(newShapeDeg[i] > newShapeDeg[bestIndex])
			// 		bestIndex = i;
			// }
		}

		if(bestIndex == -1)
			return NULL;

	//	if the shape-degree of the collapsed region is too bad, we'll skip the collapse
		if(newShapeDeg[bestIndex] < desc.triQuality * shapeDeg)
			return NULL;
/*
	//	the approximation degree is only interesting if both endpoints of the
	//	edge are regular surface vertices
		bool regularNeighbourhood = IsRegularSurfaceVertex(grid, e->vertex(0)) &&	
									IsRegularSurfaceVertex(grid, e->vertex(1));
*/

	//	if the best approximation degree is not too bad, we'll perform the collapse
		if(/*!regularNeighbourhood || */(newApproxDeg[bestIndex] >= minApproxDeg))
		{						
		//	pick one of the endpoints to be the one that resides
		//	This has to be done with care, since the residing vertex
		//	determines which edges will be removed and which will remain
		//	(this is important, since we want crease edges to remain!).
		//	If only one of the endpoints lies on a crease or is fixed, the
		//	decision is easy (the marked one has to remain).
		//	If both are marked, we have to investigate the triangles which
		//	are adjacent to the collapsed edge (quadrilaterals don't make
		//	problems here).
		//	- If all three corner vertices are marked we have to distinguish:
		//		(a) all three edges are marked -> abort
		//		(b) two edges are marked -> the vertex connecting both remains.
		//		(c) one edge is marked -> the chosen vertex doesn't affect edge-marks.
		//	- If only the two are marked they don't affect the edge-marks.
		//	
		//	Even more care has to be taken if a fixed vertex is involved.
		//	If case (b) applies and the creas-vertex is has to remain, it
		//	has to be moved to the position of the fixed-vertex and has to
		//	be marked fixed itself.
					
		//	choose the vertex that shall remain.
			Vertex* vrt = e->vertex(0);

			if(vrtSI[0] != REM_FIXED && vrtSI[1] != REM_NONE)
			{
				vrt = e->vertex(1);
			}

			if(vrtSI[0] != REM_NONE && vrtSI[1] != REM_NONE){
			//	both are marked. Things are getting complicated now!
			//	get adjacent faces of e
				vector<Face*> vFaces;
				vFaces.reserve(2);
				CollectFaces(vFaces, grid, e);
			
				vector<Edge*> vEdges;
				vEdges.reserve(4);
				for(size_t i = 0; i < vFaces.size(); ++i){
					Face* f = vFaces[i];
				//	only triangles are interesting
					if(f->num_edges() == 3){
					//	get the number of marked edges
						CollectEdges(vEdges, grid, f);
					//	count the marked edges
						int numMarked = 0;
						for(size_t j = 0; j < vEdges.size(); ++j){
						//	note that pshMarks exists since vrtSI != REM_NONE
							if(pshMarks->get_subset_index(vEdges[j]) != REM_NONE)
								++numMarked;
						}
					
						if(numMarked == 3){
							return NULL;	//	case (a) applies
						}
						else if(numMarked == 2){
						//	check which of the vrts is connected to two
						//	marked edges of the triangle
							for(size_t j = 0; j < 2; ++j){
								int numMarked = 0;
								for(size_t k = 0; k < vEdges.size(); ++k){
									if(pshMarks->get_subset_index(vEdges[k]) != REM_NONE){
										if(EdgeContains(vEdges[k], e->vertex(j)))
											++numMarked;
									}
								}
							//	if numMarked == 2 we found it
								if(numMarked == 2){
								//	the connected edge has to be marked as a crease
									Edge* ce = GetConnectedEdge(grid, e->vertex(j), f);
									if(ce)
										pshMarks->assign_subset(ce, REM_CREASE);
								//	we're done. break
									break;
								}
							}
						}
						else{
						}
					}
				}
			}

		//	collapse the edge
			CollapseEdge(grid, e, vrt);
			
		//	assign best position
			aaPos[vrt] = v[bestIndex];
		//	assign the normal
			aaNorm[vrt] = n[bestIndex];
/*
			if(pCandidates){
//TODO: all edges that belong to associated faces are new candidates.						
			//	associated edges of vrt are possible new candidates
				pCandidates->select(grid.associated_edges_begin(vrt),
							grid.associated_edges_end(vrt));
			}*/

			return vrt;
		}
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
template <class TAAPosVRT, class TAANormVRT, class TAAIntVRT>
static bool PerformCollapses(const AdjustEdgeLengthDesc& desc, Grid& grid,
					  SubsetHandler& shMarks, EdgeSelector& esel,
					  TAAPosVRT& aaPos, TAANormVRT& aaNorm, TAAIntVRT& aaInt)
{	
	PROFILE_FUNC();
	LOG("  performing collapses\n");
	vector<Edge*>	edges;
	int numCollapses = 0;
//	compare squares
	number minEdgeLenSq = sq(desc.minEdgeLen);

	while(!esel.empty())
	{
		Edge* e = *esel.begin<Edge>();
		esel.deselect(e);
		
	//	the higher the curvature the larger minEdgeLen (lenFac is the inverse).
		// number dot = max(SMALL, VecDot(aaNorm[e->vertex(0)], aaNorm[e->vertex(1)]));
		// number lenFacSq = sq(desc.adaptivity * dot + (1. - desc.adaptivity));
	//	adaptivity doesn't work at the moment...
		number lenFacSq = 1;

	//	check whether the edge is short enough
		if(lenFacSq * VecDistanceSq(aaPos[e->vertex(0)], aaPos[e->vertex(1)]) < minEdgeLenSq)
		{
			Vertex* vrt = TryCollapse(desc, grid, e, aaPos, aaNorm, aaInt, &shMarks, &esel);
			if(vrt){
				++numCollapses;

			//	we'll deselect associated edges of vrt, to avoid cascade-collapses
				CollectAssociated(edges, grid, vrt);
				esel.deselect(edges.begin(), edges.end());
			}
		}
	}
	LOG("  collapses performed: " << numCollapses << endl);
	
	return true;
}

////////////////////////////////////////////////////////////////////////
template <class TAAPosVRT, class TAANormVRT>
static bool TrySplit(Grid& grid, Edge* e, TAAPosVRT& aaPos, TAANormVRT& aaNorm,
			  EdgeSelector* pCandidates = NULL, SubsetHandler* pshMarks = NULL)
{
	// bool bCreaseEdge = false;
//	splits are not allowed for fixed edges
	if(pshMarks){
		if(pshMarks->get_subset_index(e) == REM_FIXED)
			return false;
		// else if(pshMarks->get_subset_index(e) == REM_CREASE)
		// 	bCreaseEdge = true;
	}

//	get the center of the edges
	vector3 vCenter = CalculateCenter(e, aaPos);

//	the new normal
	vector3 n;
	VecAdd(n, aaNorm[e->vertex(0)], aaNorm[e->vertex(1)]);
	VecNormalize(n, n);
	
	// if(bCreaseEdge){		
	// //	interpolating the normal can cause severe problems at highly
	// //	irregular vertices or if one vertecs lies on a very
	// //	sharp edge (the normals of the endpoints thus point
	// //	in different directions.)
	// 	// VecAdd(n, aaNorm[e->vertex(0)], aaNorm[e->vertex(1)]);
	// 	// VecNormalize(n, n);
	// 	CalculateNormal(n, grid, e, aaPos);
	// }

//	split the edge
	RegularVertex* vrt = SplitEdge<RegularVertex>(grid, e, false);

//	assign the new position
	aaPos[vrt] = vCenter;

//	assign the new normal. calculate it if required
	// if(!bCreaseEdge)
	// 	CalculateVertexNormal(n, grid, vrt, aaPos);			

	aaNorm[vrt] = n;

//	associated edges of vrt are candidates again
	// if(pCandidates)
	// 	pCandidates->select(grid.associated_edges_begin(vrt),
	// 						grid.associated_edges_end(vrt));

	return true;
}

////////////////////////////////////////////////////////////////////////
template <class TAAPosVRT, class TAANormVRT>
static bool PerformSplits(const AdjustEdgeLengthDesc& desc, Grid& grid,
					SubsetHandler& shMarks, EdgeSelector& esel,
					TAAPosVRT& aaPos, TAANormVRT& aaNorm)
{
	PROFILE_FUNC();
//	compare squares
	number maxEdgeLenSq = sq(desc.maxEdgeLen);

	LOG("  performing splits\n");
	int numSplits = 0;

	while(!esel.empty())
	{
	//	get an edge
		Edge* e = *esel.begin<Edge>();
		esel.deselect(e);
		
	//	the higher the curvature the smaller maxEdgeLen (lenFac is the inverse).
	//	adaptivity doesn't work currently...
		// number dot = max(SMALL, VecDot(aaNorm[e->vertex(0)], aaNorm[e->vertex(1)]));
		// number lenFacSq = sq(desc.adaptivity * dot + (1. - desc.adaptivity));
		number lenFacSq = 1;

	//	check whether the edge should be splitted
		if(VecDistanceSq(aaPos[e->vertex(0)], aaPos[e->vertex(1)]) > lenFacSq * maxEdgeLenSq)
		{
			if(TrySplit(grid, e, aaPos, aaNorm, &esel, &shMarks))
				++numSplits;
		}
	}

	LOG("  splits performed: " << numSplits << endl);
	return true;
}

////////////////////////////////////////////////////////////////////////
//	relocate point by smoothing
// static void RelocatePointBySmoothing(vector3& vOut, const vector3&v,
// 						  std::vector<vector3>& vNodes,
// 						  size_t numIterations, number stepSize,
// 						  std::vector<number>* pvWeights = NULL)
// {
// 	if(vNodes.empty())
// 		return;

// //TODO:	incorporate weights
// //	iterate through all nodes, calculate the direction
// //	from v to each node, and add the scaled sum to v.
// 	vector3 t, vOld;
// 	vOut = v;
// 	stepSize /= (number)vNodes.size();

// 	for(size_t j = 0; j < numIterations; ++j){
// 		vOld = vOut;
// 		for(size_t i = 0; i < vNodes.size(); ++i){
// 			VecSubtract(t, vNodes[i], vOld);
// 			VecScale(t, t, stepSize);
// 			VecAdd(vOut, vOut, t);
// 		}
// 	}
// }

////////////////////////////////////////////////////////////////////////
//	FixBadTriangles
// template <class TAAPosVRT, class TAANormVRT>
// static bool FixBadTriangles(Grid& grid, SubsetHandler& shMarks, EdgeSelector& esel,
// 					TAAPosVRT& aaPos, TAANormVRT& aaNorm,
// 					number qualityThreshold)
// {
// 	LOG("  fixing bad triangles... ");
// //	iterate through marked edges and check whether adjacent triangles are
// //	badly shaped. If this is the case, then split the non-marked
// //	edges of the triangle.
// //	store newly inserted vertices and smooth them at the end of the algo
// 	vector<Vertex*> vNewVrts;
// 	vector<Face*> vFaces;
// 	vector<Edge*> vEdges;
	
// //	we wont assign this selector to the grid until it is clear that
// //	we'll need it.
// 	Selector sel;
	
	
// 	EdgeIterator iter = esel.begin<Edge>();
// 	while(iter != esel.end<Edge>())
// 	{
// 	//	store edge and increase iterator immediatly
// 		Edge* e = *iter;
// 		++iter;
		
// 	//	get the adjacent faces
// 		CollectFaces(vFaces, grid, e);
		
// 	//	check whether one of them is degenerated.
// 		for(size_t i = 0; i < vFaces.size(); ++i){
// 			Face* f = vFaces[i];
// 			number q = FaceQuality(f, aaPos);
// 		//	if the quality is smaller than threshold, we have to
// 		//	do something
// 			if(q < qualityThreshold){
// 			//	make sure that the selector is connected to the grid
// 				if(sel.grid() == NULL)
// 					sel.assign_grid(grid);
					
// 			//	get associated edges and mark them for refinement
// 				CollectEdges(vEdges, grid, f);

// 				for(size_t j = 0; j < vEdges.size(); ++j){
// 					if(shMarks.get_subset_index(vEdges[j]) == REM_NONE)
// 						sel.select(vEdges[j]);
// 				}
// 			}
// 		}
// 	}
	
// //	if edges have been selected, we'll now call refine.
// 	if(sel.grid() != NULL){
// 		if(sel.num<Edge>() > 0){
// 			if(Refine(grid, sel)){
// 				LOG(sel.num<Vertex>() << " new vertices... ");
// 			//	retriangulate surface
// 				if(grid.num<Quadrilateral>() > 0)
// 					Triangulate(grid, grid.begin<Quadrilateral>(), grid.end<Quadrilateral>());
				
// 			//	calculate normals, then
// 			//	smooth new vertices (all vertices selected in sel).
// 				vector<Vertex*> vNeighbours;
// 				vector<vector3> vNodes;
				
// 			//	calculate normals
// 				for(VertexIterator iter = sel.begin<Vertex>();
// 						iter != sel.end<Vertex>(); ++iter)
// 				{
// 				//	collect neighbour nodes
// 					Vertex* vrt = *iter;
// 					CollectNeighbors(vNeighbours, grid, vrt);
					
// 				//	sum their normals and interpolate it
// 				//	make sure to only add valid normals
// 					vector3 n(0, 0, 0);
// 					for(size_t i = 0; i < vNeighbours.size(); ++i){
// 						if(!sel.is_selected(vNeighbours[i]))
// 							VecAdd(n, n, aaNorm[vNeighbours[i]]);
// 					}
// 					VecNormalize(aaNorm[vrt], n);
// 				}
				
// 			//	repeat smoothing.
// 				for(size_t i = 0; i < 5; ++i){
// 					for(VertexIterator iter = sel.begin<Vertex>();
// 						iter != sel.end<Vertex>(); ++iter)
// 					{
// 						Vertex* vrt = *iter;

// 					//	collect the neighbours and project them to the plane
// 					//	that is defined by vrt and its normal
// 						vector3 v = aaPos[vrt];
// 						vector3 n = aaNorm[vrt];

// 						CollectNeighbors(vNeighbours, grid, vrt);
// 						vNodes.resize(vNeighbours.size());
						
// 						for(size_t j = 0; j < vNodes.size(); ++j)
// 							ProjectPointToPlane(vNodes[j], aaPos[vNeighbours[j]], v, n);
						
// 					//	perform point relocation
// 						RelocatePointBySmoothing(aaPos[vrt], v, vNodes, 5, 0.05);
// 					}
// 				}
// 			}
// 			else{
// 				LOG("refine failed!\n");
// 				return false;
// 			}
// 		}
// 	}
	
// 	LOG("done\n");
// 	return true;
// }

////////////////////////////////////////////////////////////////////////
//	PerformSmoothing
// template <class TAAPosVRT, class TAANormVRT>
// static void PerformSmoothing(Grid& grid, SubsetHandler& shMarks,
// 					TAAPosVRT& aaPos, TAANormVRT& aaNorm,
// 					size_t numIterations, number stepSize)
// {
// 	vector<vector3> vNodes;
// 	vector<Vertex*> vNeighbours;
// 	for(size_t i = 0; i < numIterations; ++i){
// 		CalculateVertexNormals(grid, aaPos, aaNorm);
// 		for(VertexIterator iter = grid.begin<Vertex>();
// 			iter != grid.end<Vertex>(); ++iter)
// 		{
// 			Vertex* vrt = *iter;
// 		//	if the vertex is fixed then leave it where it is.
// 			if(shMarks.get_subset_index(vrt) == REM_FIXED)
// 				continue;
// /*
// if(shMarks.get_subset_index(vrt) == REM_CREASE)
// 	continue;
// */
// 		//	collect the neighbours and project them to the plane
// 		//	that is defined by vrt and its normal
// 			vector3 v = aaPos[vrt];
// 			vector3 n = aaNorm[vrt];

// 			bool bProjectPointsToPlane = true;
			
// 			if(shMarks.get_subset_index(vrt) == REM_CREASE){
// 				CollectNeighbors(vNeighbours, grid, vrt, NHT_EDGE_NEIGHBORS,
// 									IsInSubset(shMarks, REM_CREASE));

// 			//	we have to choose a special normal
// 				if(vNeighbours.size() != 2){
// 					UG_LOG("n"<<vNeighbours.size());
// 					continue;
// 				}
// 				else{
// 					vector3 v1, v2;
// 					VecSubtract(v1, v, aaPos[vNeighbours[0]]);
// 					VecSubtract(v2, v, aaPos[vNeighbours[1]]);
// 					VecNormalize(v1, v1);
// 					VecNormalize(v2, v2);
// 					VecAdd(n, v1, v2);
// 					if(VecLengthSq(n) > SMALL)
// 						VecNormalize(n, n);
// 					else {
// 					//	both edges have the same direction.
// 					//	don't project normals
// 						bProjectPointsToPlane = false;
// 					}
// 				}
// 			}
// 			else{
// 				CollectNeighbors(vNeighbours, grid, vrt);
// 			}
			
// 			vNodes.resize(vNeighbours.size());

// 			if(bProjectPointsToPlane){
// 				for(size_t j = 0; j < vNodes.size(); ++j)
// 					ProjectPointToPlane(vNodes[j], aaPos[vNeighbours[j]], v, n);
// 			}
// 			else{
// 				for(size_t j = 0; j < vNodes.size(); ++j)
// 					vNodes[j] = aaPos[vNeighbours[j]];
// 			}
// 		//	perform point relocation
// 			RelocatePointBySmoothing(aaPos[vrt], v, vNodes, 5, stepSize);
// 		}
// 	}
// }

/**	Make sure that elements in gridOut directly correspond to
 *	elements in gridIn*/
template <class TGeomObj>
static void CopySelectionStatus(Selector& selOut, Grid& gridOut,
						 Selector& selIn, Grid& gridIn)
{
	typedef typename geometry_traits<TGeomObj>::iterator GeomObjIter;
	GeomObjIter endOut = gridOut.end<TGeomObj>();
	GeomObjIter endIn = gridIn.end<TGeomObj>();
	GeomObjIter iterOut = gridOut.begin<TGeomObj>();
	GeomObjIter iterIn = gridIn.begin<TGeomObj>();
	
	for(; (iterOut != endOut) && (iterIn != endIn); ++iterOut, ++iterIn)
	{
		if(selIn.is_selected(*iterIn))
			selOut.select(*iterOut);
	}
}

////////////////////////////////////////////////////////////////////////
bool AdjustEdgeLength(Grid& grid, SubsetHandler& shMarks,
					  const AdjustEdgeLengthDesc& desc, int numIterations)
{
	PROFILE_FUNC();

//TODO:	replace this by a template parameter
	APosition aPos = aPosition;

//	we have to make sure that the mesh consist of triangles only,
//	since the algorithm would produce bad results if not.
	if(grid.num<Quadrilateral>() > 0)
	{
		UG_LOG("  INFO: grid contains quadrilaterals. Converting to triangles...\n");
				
//TODO:	not gridIn but a copy of gridIn (pRefGrid) should be triangulated.
		Triangulate(grid, grid.begin<Quadrilateral>(),
					grid.end<Quadrilateral>());
	}
		
//	make sure that grid and pRefGrid have position-attachments
	if(!grid.has_vertex_attachment(aPos)){
		UG_LOG("  vertex-position-attachment missing in AdjustEdgeLength. Aborting.\n");
		return false;
	}
	
//	make sure that faces create associated edges
	if(!grid.option_is_enabled(FACEOPT_AUTOGENERATE_EDGES))
	{
		LOG("  INFO: auto-enabling FACEOPT_AUTOGENERATE_EDGES in AdjustEdgeLength.\n");
		grid.enable_options(FACEOPT_AUTOGENERATE_EDGES);
	}

//	position attachment
	Grid::VertexAttachmentAccessor<APosition> aaPos(grid, aPos);

//	we need an integer attachment (a helper for ObtainSimpleGrid)
	AInt aInt;
	grid.attach_to<Vertex>(aInt);
	Grid::AttachmentAccessor<Vertex, AInt> aaInt(grid, aInt);

//	TODO: normals shouldn't be created here but instead be passed to the method.
//	attach the vertex normals.
	ANormal aNorm;
	grid.attach_to<Vertex>(aNorm);
	Grid::AttachmentAccessor<Vertex, ANormal> aaNorm(grid, aNorm);
	CalculateVertexNormals(grid, aPos, aNorm);

//	assign vertex marks
	AssignFixedVertices(grid, shMarks);
	AssignCreaseVertices(grid, shMarks);

//	we need an selector that holds all edges that are candidates for a operation
	EdgeSelector esel(grid);
	esel.enable_selection_inheritance(false);
	
//	sort the triangles of pRefGrid into a octree to speed-up projection performance
	SPOctree octree;
	node_tree::Traverser_ProjectPoint pojectionTraverser;
	if(desc.projectPoints){
		//PROFILE_BEGIN(octree_construction);
		octree = CreateOctree(grid, grid.begin<Triangle>(),
									grid.end<Triangle>(),
									10, 30, false, aPos);

		//PROFILE_END();
		if(!octree.valid()){
			UG_LOG("  Octree creation failed in AdjustEdgeLength. Aborting.\n");
			return false;
		}
	}
	
//	start the main iteration
	for(int iteration = 0; iteration < numIterations; ++iteration)
	{
	//	perform splits
		esel.select(grid.begin<Edge>(), grid.end<Edge>());
		if(!PerformSplits(desc, grid, shMarks, esel, aaPos, aaNorm))
			return false;

	// //	perform collapses
		esel.select(grid.begin<Edge>(), grid.end<Edge>());
		if(!PerformCollapses(desc, grid, shMarks, esel, aaPos, aaNorm, aaInt))
			return false;

	//	perform swaps
		// esel.select(grid.begin<Edge>(), grid.end<Edge>());
		// if(!PerformSwaps(desc, grid, shMarks, esel, aaPos, aaNorm, aaInt))
		// 	return false;

/*
//	This is commented out, since it didn't help with the problems encountered
//	in the geometries at that time.
//	The algorithm however works and may prove useful in the future.
	//	fix bad triangles
	//	adjacent to crease-edges badly shaped triangles may occur,
	//	which have to be treated somehow.
		esel.clear();
		esel.select(shMarks.begin<Edge>(REM_CREASE), shMarks.end<Edge>(REM_CREASE));
		esel.select(shMarks.begin<Edge>(REM_FIXED), shMarks.end<Edge>(REM_FIXED));
		FixBadTriangles(grid, shMarks, esel, aaPos, aaNorm, 0.1);
*/
	//	relocate points
		/*LOG("  smoothing points...");
		PerformSmoothing(grid, shMarks, aaPos, aaNorm, 10, 0.1);
		LOG(" done\n");*/

		// LOG("  updating normals...\n");
		// CalculateVertexNormals(grid, aPos, aNorm);

	//	project points back on the surface
		if(desc.projectPoints)
		{
			LOG("  projecting points...");
			//PROFILE_BEGIN(projecting_points);
			for(VertexIterator iter = grid.vertices_begin();
				iter != grid.vertices_end(); ++iter)
			{
//TODO:	project crease vertices onto creases only! Don't project fixed vertices
				if(shMarks.get_subset_index(*iter) != REM_FIXED){
					vector3 vNew;
					if(pojectionTraverser.project(aaPos[*iter], octree/*, &aaNorm[*iter]*/)){
						aaPos[*iter] = pojectionTraverser.get_closest_point();
					}
					else{
						LOG("f");
					}
				}
			}
			//PROFILE_END();
			LOG(" done\n");
		}

		// if(iteration < numIterations - 1){
		// 	LOG("  updating normals...");
		// 	CalculateVertexNormals(grid, aPos, aNorm);
		// }
	}


//	detach
	grid.detach_from<Vertex>(aInt);
	grid.detach_from<Vertex>(aNorm);

	GPLOTSAVE();
	return true;
}




/*
////////////////////////////////////////////////////////////////////////
//	This is an alternative version for PerformSplits.
//	It uses the Refine method.
//	It is not yet optimized for maximum speed.
//	While it performs a little less splits, overall runtime of
//	AdjustEdgeLength is not better than with the original
//	PerformSplits method.
template <class TAAPosVRT, class TAANormVRT>
bool PerformSplits(Grid& grid, SubsetHandler& shMarks, EdgeSelector& esel,
					  number maxEdgeLen, TAAPosVRT& aaPos, TAANormVRT& aaNorm)
{
	AInt aInt;
	grid.attach_to_edges(aInt);

//	compare squares
	maxEdgeLen *= maxEdgeLen;

	Selector sel(grid);
	sel.enable_autoselection(true);
	sel.select(esel.begin<Edge>(), esel.end<Edge>());

	LOG("  performing splits\n");
	int numSplits = 0;

	while(!sel.empty()){
	//	deselect all vertices and faces
		sel.clear_selection<Vertex>();
		sel.clear_selection<Face>();
		sel.clear_selection<Volume>();

	//	deselect all edges that shall not be splitted
		EdgeIterator iter = sel.begin<Edge>();
		while(iter != sel.end<Edge>()){
			Edge* e = *iter;
			++iter;

		//	the higher the curvature the smaller the maxEdgeLen.
		//	minimal lenFac is 0.1
			number lenFac = CalculateLengthFac(grid, shMarks, e, aaPos);

		//	fixed edges will not be refined
			if(shMarks.get_subset_index(e) == REM_FIXED)
				sel.deselect(e);
			else if(VecDistanceSq(aaPos[e->vertex(0)], aaPos[e->vertex(1)]) < lenFac * maxEdgeLen)
				sel.deselect(e);
		}

	//	refine the grid
		Refine(grid, sel, aInt);

	//	new vertices are selected
		numSplits += sel.num<Vertex>();

	//	re-triangulate
		Triangulate(grid, &aaPos);

	//	calculate normal for new vertices
//TODO:	be careful with crease edges
		for(VertexIterator iter = sel.begin<Vertex>();
			iter != sel.end<Vertex>(); ++iter)
		{
			CalculateVertexNormal(aaNorm[*iter], grid, *iter, aaPos);
		}

		sel.clear_selection<Vertex>();
		sel.clear_selection<Face>();
		sel.clear_selection<Volume>();
	}

	grid.detach_from_edges(aInt);
	LOG("  splits performed: " << numSplits << endl);
	return true;
}
*/

}//	end of namespace

//	created by Sebastian Reiter
// 	extended by mbreit
//	s.b.reiter@googlemail.com
//	y10 m09 d09

////////////////////////////////////////////////////////////////////////
//	This sample project is primarily intended to demonstrate how a
//	unit-test project can be set up.
//	It also shows how the profiler that comes with ug (Shiny Profiler)
//	can be used.
//	Used profiler instructions are:
//		PROFILE_FUNC();
//		PROFILE_BEGIN(sectionName);
//		PROFILE_END();
//		PROFILE_CODE(SaveGridToFile(grid, "sample_triangle.obj"));
//		PROFILER_UPDATE();
//		PROFILER_OUTPUT();
//
//	Code that is executed during PROFILE_CODE will be executed even
//	if the profiler is disabled via SHINY_PROFILER = FALSE.
//	For more information about the profiler take a look at
//	ugbase/common/profiler/
////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "ug.h"
#include "common/common.h"
#include "lib_grid/lib_grid.h"
#include <common/profiler/profiler.h>

using namespace std;
using namespace ug;
using namespace boost::unit_test;

////////////////////////////////////////////////////////////////////////
//	build_geometry
void build_geometry(Grid& grid) {
	//	enables profiling for the whole method.
	PROFILE_FUNC();

	//	create vertices
	RegularVertex* v1 = *grid.create<RegularVertex> ();
	RegularVertex* v2 = *grid.create<RegularVertex> ();
	RegularVertex* v3 = *grid.create<RegularVertex> ();

	//	attach position data
	grid.attach_to_vertices(aPosition);
	Grid::VertexAttachmentAccessor<APosition> aaPos(grid, aPosition);

	//	assign coordinates
	aaPos[v1] = vector3(-1, -1, -1);
	aaPos[v2] = vector3(1, -1, -1);
	aaPos[v3] = vector3(0, 1, 1);

	//	create the triangle
	grid.create<Triangle> (TriangleDescriptor(v1, v2, v3));
}


/// test function for point containment in triangles
template <typename vector_t>
size_t test_points_in_triangles
(
	bool (*contains_fct) (const vector_t&, const vector_t&, const vector_t&, const vector_t&),
	size_t nTri,
	size_t nPts,
	number scale
)
{
	// init RNG
	std::srand(0);

	vector_t v0, v1, v2;
	vector_t p;

	size_t hits = 0;

	// test for nTri random triangles
	for (size_t i = 0; i < nTri; ++i)
	{
		// create three point coords within [0, scale]^d
		size_t sz = v0.size();
		for (size_t j = 0; j < sz; ++j)
		{
			v0[j] = (scale * std::rand()) / RAND_MAX;
			v1[j] = (scale * std::rand()) / RAND_MAX;
			v2[j] = (scale * std::rand()) / RAND_MAX;
		}

		// test points
		for (size_t j = 0; j < nPts; ++j)
		{
			// create test point within [0, scale]^d
			for (size_t k = 0; k < sz; ++k)
				p[k] = (scale * std::rand()) / RAND_MAX;

			if (contains_fct(p, v0, v1, v2)) ++hits;
		}
	}

	return hits;
}

/// test function for point containment in quadrilaterals
template <typename vector_t>
size_t test_points_in_quadrilaterals
(
	bool (*contains_fct) (const vector_t&, const vector_t&, const vector_t&, const vector_t&, const vector_t&),
	size_t nQuad,
	size_t nPts,
	number scale
)
{
	// init RNG
	std::srand(0);

	vector_t v0, v1, v2, v3;
	vector_t p;
	number dummy1, dummy2;

	size_t hits = 0;

	// test for nTri random triangles
	for (size_t i = 0; i < nQuad; ++i)
	{
		// create three point coords within [0, scale]^d
		size_t sz = v0.size();

		do
		{
			for (size_t j = 0; j < sz; ++j)
			{
				v0[j] = (scale * std::rand()) / RAND_MAX;
				v1[j] = (scale * std::rand()) / RAND_MAX;
				v2[j] = (scale * std::rand()) / RAND_MAX;
				v3[j] = (scale * std::rand()) / RAND_MAX;
			}
		}
		// accept only convex quadrilaterals
		while(!LineLineIntersection2d(p, dummy1, dummy2, v0, v2, v1, v3));

		// test points
		for (size_t j = 0; j < nPts; ++j)
		{
			// create test point within [0, scale]^d
			for (size_t k = 0; k < sz; ++k)
				p[k] = (scale * std::rand()) / RAND_MAX;

			if (contains_fct(p, v0, v1, v2, v3)) ++hits;
		}
	}

	return hits;
}

/// test function for point containment in tetrahedra
template <typename vector_t>
size_t test_points_in_tetrahedra
(
	bool (*contains_fct) (const vector_t&, const vector_t&, const vector_t&, const vector_t&, const vector_t&),
	size_t nTet,
	size_t nPts,
	number scale
)
{
	// init RNG
	std::srand(0);

	vector_t v0, v1, v2, v3;
	vector_t p;

	size_t hits = 0;

	// test for nTri random triangles
	for (size_t i = 0; i < nTet; ++i)
	{
		// create three point coords within [0, scale]^d
		size_t sz = v0.size();
		for (size_t j = 0; j < sz; ++j)
		{
			v0[j] = (scale * std::rand()) / RAND_MAX;
			v1[j] = (scale * std::rand()) / RAND_MAX;
			v2[j] = (scale * std::rand()) / RAND_MAX;
			v3[j] = (scale * std::rand()) / RAND_MAX;
		}

		// test points
		for (size_t j = 0; j < nPts; ++j)
		{
			// create test point within [0, scale]^d
			for (size_t k = 0; k < sz; ++k)
				p[k] = (scale * std::rand()) / RAND_MAX;

			if (contains_fct(p, v0, v1, v2, v3)) ++hits;
		}
	}

	return hits;
}


// register the ug fixture with the ug sample test suite
BOOST_AUTO_TEST_SUITE(testGridSuiteNumProc1);

/**
 *
 */
BOOST_AUTO_TEST_CASE(gridTest) {
	//	begin profiling for the main section.
	//	main_section is an arbitrary name.
	//PROFILE_BEGIN(main_section);

	//	create a grid.
	Grid grid;

	//	build geometry
	build_geometry(grid);

	BOOST_CHECK(grid.num_vertices() == 3);
	BOOST_CHECK(grid.num_faces() == 1);

	//	end profiling of this section
	//PROFILE_END();
}


BOOST_AUTO_TEST_CASE(pointInTriangleTest)
{
	static const size_t nScales = 30;

	number scale = 1.0;
	size_t hits_0 = test_points_in_triangles(&PointIsInsideTriangle<MathVector<2> >, 1000, 10000, scale);

	BOOST_MESSAGE("---------------\nHITS statistic\n---------------");
	for (size_t i = 0; i <= nScales; ++i)
	{
		scale = std::pow(10.0, (int)i-(int)nScales/2);
		size_t hits = test_points_in_triangles(&PointIsInsideTriangle<MathVector<2> >, 1000, 10000, scale);
		BOOST_MESSAGE("scale "<< std::pow(10.0, (int)i-(int)nScales/2) << ":   " << hits << " / " << 10000000);
		BOOST_CHECK_MESSAGE(hits == hits_0, "There need to be the same number of hits for every scale, "
							"but #hits is " << hits << " for scale " << scale << " and " << hits_0
							<< " for scale 1.0");
	}
}


BOOST_AUTO_TEST_CASE(pointInQuadrilateralTest)
{
	static const size_t nScales = 30;

	number scale = 1.0;
	size_t hits_0 = test_points_in_quadrilaterals(&PointIsInsideQuadrilateral<MathVector<2> >, 1000, 10000, scale);

	BOOST_MESSAGE("---------------\nHITS statistic\n---------------");
	for (size_t i = 0; i <= nScales; ++i)
	{
		number scale = std::pow(10.0, (int)i-(int)nScales/2);
		size_t hits = test_points_in_quadrilaterals(&PointIsInsideQuadrilateral<MathVector<2> >, 1000, 10000, scale);
		BOOST_MESSAGE("scale "<< std::pow(10.0, (int)i-(int)nScales/2) << ":   " << hits << " / " << 10000000);
		BOOST_CHECK_MESSAGE(hits == hits_0, "There need to be the same number of hits for every scale, "
							"but #hits is " << hits << " for scale " << scale << " and " << hits_0
							<< " for scale 1.0");
	}
}


BOOST_AUTO_TEST_CASE(pointInTetrahedronTest)
{
	static const size_t nScales = 30;

	number scale = 1.0;
	size_t hits_0 = test_points_in_tetrahedra(&PointIsInsideTetrahedron<MathVector<3> >, 1000, 10000, scale);

	BOOST_MESSAGE("---------------\nHITS statistic\n---------------");
	for (size_t i = 0; i <= nScales; ++i)
	{
		number scale = std::pow(10.0, (int)i-(int)nScales/2);
		size_t hits = test_points_in_tetrahedra(&PointIsInsideTetrahedron<MathVector<3> >, 1000, 10000, scale);
		BOOST_MESSAGE("scale "<< std::pow(10.0, (int)i-(int)nScales/2) << ":   " << hits << " / " << 10000000);
		BOOST_CHECK_MESSAGE(hits == hits_0, "There need to be the same number of hits for every scale, "
							"but #hits is " << hits << " for scale " << scale << " and " << hits_0
							<< " for scale 1.0");
	}
}



// end of ug_master suite
BOOST_AUTO_TEST_SUITE_END();

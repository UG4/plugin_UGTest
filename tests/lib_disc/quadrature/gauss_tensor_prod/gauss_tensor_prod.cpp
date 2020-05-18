/*
 * Copyright (c) 2013-2015:  G-CSC, Goethe University Frankfurt
 * Author: Lisa Grau
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


#include "../quadrature.h"
#include "common/util/provider.h"
#include "gauss_tensor_prod.h"
#include "../gauss_legendre/gauss_legendre.h"
#include "../gauss_jacobi/gauss_jacobi10.h"
#include "../gauss_jacobi/gauss_jacobi20.h"
#include "lib_disc/reference_element/reference_mapping_provider.h"

namespace ug {

GaussQuadratureTriangle::GaussQuadratureTriangle(size_t order)
{
	GaussLegendre quadRule = GaussLegendre(order);
	GaussJacobi10 quadRule10 = GaussJacobi10(order);

	m_order = std::min(quadRule.order(), quadRule10.order());
	m_numPoints = quadRule10.size() * quadRule.size();
	position_type* pvPoint = new position_type[m_numPoints];
	weight_type* pvWeight = new weight_type[m_numPoints];

	size_t cnt = 0;
	for(size_t i = 0; i < quadRule.size(); i++){
		for(size_t j = 0; j < quadRule10.size(); j++, cnt++){
			pvPoint[cnt][0] = quadRule10.point(j)[0];
			pvPoint[cnt][1] = (1.0 - quadRule10.point(j)[0] ) * quadRule.point(i)[0];
			pvWeight[cnt] = quadRule.weight(i) * quadRule10.weight(j);
		}
	}
	m_pvPoint = pvPoint;
	m_pvWeight = pvWeight;
};

GaussQuadratureTriangle::~GaussQuadratureTriangle()
{
	delete[] m_pvPoint;
	delete[] m_pvWeight;
}

GaussQuadratureQuadrilateral::GaussQuadratureQuadrilateral(size_t order)
{
	GaussLegendre quadRule = GaussLegendre(order);

	m_order = std::min(quadRule.order(), quadRule.order());
	m_numPoints = quadRule.size() * quadRule.size();
	position_type* pvPoint = new position_type[m_numPoints];
	weight_type* pvWeight = new weight_type[m_numPoints];

	size_t cnt  = 0;
	for(size_t i = 0; i < quadRule.size(); i ++) {
		for(size_t j = 0; j < quadRule.size(); j++, cnt++) {
			pvPoint[cnt][0] = quadRule.point(i)[0];
			pvPoint[cnt][1] = quadRule.point(j)[0];
			pvWeight[cnt] = quadRule.weight(i) * quadRule.weight(j);
		}
	}
	m_pvPoint = pvPoint;
	m_pvWeight = pvWeight;
};

GaussQuadratureQuadrilateral::~GaussQuadratureQuadrilateral()
{
	delete[] m_pvPoint;
	delete[] m_pvWeight;
}

GaussQuadratureHexahedron::GaussQuadratureHexahedron(size_t order)
{
	GaussLegendre quadRule = GaussLegendre(order);

	m_order = std::min(quadRule.order(), std::min(quadRule.order(), quadRule.order()));
	m_numPoints = quadRule.size() * quadRule.size() * quadRule.size();
	position_type* pvPoint = new position_type[m_numPoints];
	weight_type* pvWeight = new weight_type[m_numPoints];

	size_t cnt  = 0;
	for(size_t i = 0; i < quadRule.size(); i ++) {
		for(size_t j = 0; j < quadRule.size(); j++) {
			for(size_t k = 0; k < quadRule.size(); k++, cnt++) {
				pvPoint[cnt][0] = quadRule.point(i)[0];
				pvPoint[cnt][1] = quadRule.point(j)[0];
				pvPoint[cnt][2] = quadRule.point(k)[0];
				pvWeight[cnt] = quadRule.weight(i) * quadRule.weight(j) * quadRule.weight(k);
			}
		}
	}
	m_pvPoint = pvPoint;
	m_pvWeight = pvWeight;
};

GaussQuadratureHexahedron::~GaussQuadratureHexahedron()
{
	delete[] m_pvPoint;
	delete[] m_pvWeight;
}

GaussQuadratureTetrahedron::GaussQuadratureTetrahedron(size_t order)
{
	GaussLegendre quadRule = GaussLegendre(order);
	GaussJacobi10 quadRule10 = GaussJacobi10(order);
	GaussJacobi20 quadRule20 = GaussJacobi20(order);

	m_order = std::min(quadRule.order(), std::min(quadRule10.order(), quadRule20.order()));
	m_numPoints = quadRule.size() * quadRule10.size() * quadRule20.size();
	position_type* pvPoint = new position_type[m_numPoints];
	weight_type* pvWeight = new weight_type[m_numPoints];

	size_t cnt = 0;
	for(size_t i = 0; i < quadRule20.size(); i++) {
		for(size_t j = 0; j < quadRule10.size(); j++) {
			for(size_t k = 0; k < quadRule.size(); k++, cnt++) {
				pvPoint[cnt][0] = quadRule20.point(i)[0];
				pvPoint[cnt][1] = (1.0 - quadRule20.point(i)[0] ) * quadRule10.point(j)[0];
				pvPoint[cnt][2] = (1.0 - quadRule20.point(i)[0]) * (1.0 - quadRule10.point(j)[0]) * quadRule.point(k)[0];
				pvWeight[cnt] = quadRule20.weight(i) * quadRule10.weight(j) * quadRule.weight(k);
			}
		}
	}
	m_pvPoint = pvPoint;
	m_pvWeight = pvWeight;
};

GaussQuadratureTetrahedron::~GaussQuadratureTetrahedron()
{
	delete[] m_pvPoint;
	delete[] m_pvWeight;
}

GaussQuadraturePrism::GaussQuadraturePrism(size_t order)
{
	GaussLegendre quadRule = GaussLegendre(order);
	GaussJacobi10 quadRule10 = GaussJacobi10(order);

	m_order = std::min(quadRule.order(), quadRule10.order());
	m_numPoints = quadRule10.size() * quadRule.size() * quadRule.size();
	position_type* pvPoint = new position_type[m_numPoints];
	weight_type* pvWeight = new weight_type[m_numPoints];

	size_t cnt = 0;
	for(size_t i = 0; i < quadRule10.size(); i++) {
		for(size_t j = 0; j < quadRule.size(); j++) {
			for(size_t k = 0; k < quadRule.size(); k++, cnt++) {
				pvPoint[cnt][0] = quadRule10.point(i)[0];
				pvPoint[cnt][1] = (1.0 - quadRule10.point(i)[0]) * quadRule.point(j)[0];
				pvPoint[cnt][2] = quadRule.point(k)[0];
				pvWeight[cnt] = quadRule10.weight(i) * quadRule.weight(j) * quadRule.weight(k);
			}
		}
	}
	m_pvPoint = pvPoint;
	m_pvWeight = pvWeight;
};

GaussQuadraturePrism::~GaussQuadraturePrism()
{
	delete[] m_pvPoint;
	delete[] m_pvWeight;
}

GaussQuadraturePyramid::GaussQuadraturePyramid(size_t order)
{
	GaussQuadratureTetrahedron quadRule = GaussQuadratureTetrahedron(order);

	m_order = quadRule.order();
	m_numPoints = quadRule.size() * 2;
	position_type* pvPoint = new position_type[m_numPoints];
	weight_type* pvWeight = new weight_type[m_numPoints];

	MathVector<3> Tet1Co[4];
	Tet1Co[0] = MathVector<3>(0,0,0);
	Tet1Co[1] = MathVector<3>(1,1,0);
	Tet1Co[2] = MathVector<3>(0,0,1);
	Tet1Co[3] = MathVector<3>(0,1,0);

	MathVector<3> Tet2Co[4];
	Tet2Co[0] = MathVector<3>(0,0,0);
	Tet2Co[1] = MathVector<3>(1,0,0);
	Tet2Co[2] = MathVector<3>(1,1,0);
	Tet2Co[3] = MathVector<3>(0,0,1);

	DimReferenceMapping<3, 3>& map1 =
			ReferenceMappingProvider::get<3,3>(ROID_TETRAHEDRON, Tet1Co);

	size_t cnt = 0;
	for(size_t i = 0; i < quadRule.size(); i++, cnt++) {
		map1.local_to_global(pvPoint[cnt], quadRule.point(i));
		pvWeight[cnt] = quadRule.weight(i) * map1.sqrt_gram_det(quadRule.point(i));
	}


	DimReferenceMapping<3, 3>& map2 =
			ReferenceMappingProvider::get<3,3>(ROID_TETRAHEDRON, Tet2Co);

	for(size_t j = 0; j < quadRule.size(); j++, cnt++) {
		map2.local_to_global(pvPoint[cnt], quadRule.point(j));
		pvWeight[cnt] = quadRule.weight(j) * map2.sqrt_gram_det(quadRule.point(j));
	}
	m_pvPoint = pvPoint;
	m_pvWeight = pvWeight;
};

GaussQuadraturePyramid::~GaussQuadraturePyramid()
{
	delete[] m_pvPoint;
	delete[] m_pvWeight;
}

GaussQuadratureOctahedron::GaussQuadratureOctahedron(size_t order)
{
	GaussQuadratureTetrahedron quadRule = GaussQuadratureTetrahedron(order);

	m_order = quadRule.order();
	m_numPoints = quadRule.size() * 4;
	position_type* pvPoint = new position_type[m_numPoints];
	weight_type* pvWeight = new weight_type[m_numPoints];

	MathVector<3> Tet1Co[4];
	//Tet1Co[0] = MathVector<3>(0,0,0);
	//Tet1Co[1] = MathVector<3>(1,1,0);
	//Tet1Co[2] = MathVector<3>(0,0,1);
	//Tet1Co[3] = MathVector<3>(0,1,0);
	Tet1Co[0] = MathVector<3>(0,0,0);
	Tet1Co[1] = MathVector<3>(1,0,0);
	Tet1Co[2] = MathVector<3>(1,1,0);
	Tet1Co[3] = MathVector<3>(0,0,1);


	MathVector<3> Tet2Co[4];
//	Tet2Co[0] = MathVector<3>(0,0,0);
//	Tet2Co[1] = MathVector<3>(1,0,0);
//	Tet2Co[2] = MathVector<3>(1,1,0);
//	Tet2Co[3] = MathVector<3>(0,0,1);
	Tet2Co[0] = MathVector<3>(0,0,0);
	Tet2Co[1] = MathVector<3>(1,1,0);
	Tet2Co[2] = MathVector<3>(0,1,0);
	Tet2Co[3] = MathVector<3>(0,0,1);

	MathVector<3> Tet3Co[4];
//	Tet3Co[0] = MathVector<3>(0,0,0);
//	Tet3Co[1] = MathVector<3>(1,1,0);
//	Tet3Co[2] = MathVector<3>(0,0,-1);
//	Tet3Co[3] = MathVector<3>(0,1,0);
	Tet3Co[0] = MathVector<3>(0,0,0);
	Tet3Co[1] = MathVector<3>(1,1,0);
	Tet3Co[2] = MathVector<3>(1,0,0);
	Tet3Co[3] = MathVector<3>(0,0,-1);

	MathVector<3> Tet4Co[4];
//	Tet4Co[0] = MathVector<3>(0,0,0);
//	Tet4Co[1] = MathVector<3>(1,0,0);
//	Tet4Co[2] = MathVector<3>(1,1,0);
//	Tet4Co[3] = MathVector<3>(0,0,-1);
	Tet4Co[0] = MathVector<3>(0,0,0);
	Tet4Co[1] = MathVector<3>(0,1,0);
	Tet4Co[2] = MathVector<3>(1,1,0);
	Tet4Co[3] = MathVector<3>(0,0,-1);


	DimReferenceMapping<3, 3>& map1 =
			ReferenceMappingProvider::get<3,3>(ROID_TETRAHEDRON, Tet1Co);

	size_t cnt = 0;
	for(size_t i = 0; i < quadRule.size(); i++, cnt++) {
		map1.local_to_global(pvPoint[cnt], quadRule.point(i));
		pvWeight[cnt] = quadRule.weight(i) * map1.sqrt_gram_det(quadRule.point(i));
	}


	DimReferenceMapping<3, 3>& map2 =
			ReferenceMappingProvider::get<3,3>(ROID_TETRAHEDRON, Tet2Co);

	for(size_t j = 0; j < quadRule.size(); j++, cnt++) {
		map2.local_to_global(pvPoint[cnt], quadRule.point(j));
		pvWeight[cnt] = quadRule.weight(j) * map2.sqrt_gram_det(quadRule.point(j));
	}


	DimReferenceMapping<3, 3>& map3 =
			ReferenceMappingProvider::get<3,3>(ROID_TETRAHEDRON, Tet3Co);

	for(size_t k = 0; k < quadRule.size(); k++, cnt++) {
		map3.local_to_global(pvPoint[cnt], quadRule.point(k));
		pvWeight[cnt] = quadRule.weight(k) * map3.sqrt_gram_det(quadRule.point(k));
	}


	DimReferenceMapping<3, 3>& map4 =
			ReferenceMappingProvider::get<3,3>(ROID_TETRAHEDRON, Tet4Co);

	for(size_t l = 0; l < quadRule.size(); l++, cnt++) {
		map4.local_to_global(pvPoint[cnt], quadRule.point(l));
		pvWeight[cnt] = quadRule.weight(l) * map4.sqrt_gram_det(quadRule.point(l));
	}


	m_pvPoint = pvPoint;
	m_pvWeight = pvWeight;
};

GaussQuadratureOctahedron::~GaussQuadratureOctahedron()
{
	delete[] m_pvPoint;
	delete[] m_pvWeight;
}
}


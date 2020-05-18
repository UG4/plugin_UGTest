/*
 * Copyright (c) 2013-2015:  G-CSC, Goethe University Frankfurt
 * Author: Andreas Vogel
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

#include "neumann_boundary_fv.h"
#include "lib_disc/spatial_disc/disc_util/fvho_geom.h"
#include "lib_disc/spatial_disc/disc_util/geom_provider.h"

namespace ug{

////////////////////////////////////////////////////////////////////////////////
//	Constructor
////////////////////////////////////////////////////////////////////////////////

template<typename TDomain>
NeumannBoundaryFV<TDomain>::NeumannBoundaryFV(const char* function)
 :NeumannBoundaryBase<TDomain>(function),
  m_order(1), m_lfeID(LFEID::LAGRANGE, TDomain::dim, m_order)
{
	register_all_funcs(m_order);
}

template<typename TDomain>
void NeumannBoundaryFV<TDomain>::
prepare_setting(const std::vector<LFEID>& vLfeID, bool bNonRegularGrid)
{
	if(bNonRegularGrid)
		UG_THROW("NeumannBoundary: Hanging Nodes not implemented.");

//	check number
	if(vLfeID.size() != 1)
		UG_THROW("NeumannBoundaryFV: Need exactly 1 function.");

	if(vLfeID[0].type() != LFEID::LAGRANGE)
		UG_THROW("NeumannBoundary: FV Scheme only implemented for Lagrange-type elements.");

//	check that not ADAPTIVE
	if(vLfeID[0].order() < 1)
		UG_THROW("NeumannBoundary: Adaptive order not implemented.");

//	set order
	m_lfeID = vLfeID[0];
	m_order = vLfeID[0].order();

	register_all_funcs(m_order);
}

template<typename TDomain>
void NeumannBoundaryFV<TDomain>::
add(SmartPtr<CplUserData<number, dim> > data, const char* BndSubsets, const char* InnerSubsets)
{
	m_vNumberData.push_back(NumberData(data, BndSubsets, InnerSubsets, this));
	this->add_inner_subsets(InnerSubsets);
}

template<typename TDomain>
void NeumannBoundaryFV<TDomain>::
add(SmartPtr<CplUserData<number, dim, bool> > user, const char* BndSubsets, const char* InnerSubsets)
{
	m_vBNDNumberData.push_back(BNDNumberData(user, BndSubsets, InnerSubsets));
	this->add_inner_subsets(InnerSubsets);
}

template<typename TDomain>
void NeumannBoundaryFV<TDomain>::
add(SmartPtr<CplUserData<MathVector<dim>, dim> > user, const char* BndSubsets, const char* InnerSubsets)
{
	m_vVectorData.push_back(VectorData(user, BndSubsets, InnerSubsets));
	this->add_inner_subsets(InnerSubsets);
}

template<typename TDomain>
void NeumannBoundaryFV<TDomain>::update_subset_groups()
{
	for(size_t i = 0; i < m_vNumberData.size(); ++i)
		base_type::update_subset_groups(m_vNumberData[i]);
	for(size_t i = 0; i < m_vBNDNumberData.size(); ++i)
		base_type::update_subset_groups(m_vBNDNumberData[i]);
	for(size_t i = 0; i < m_vVectorData.size(); ++i)
		base_type::update_subset_groups(m_vVectorData[i]);
}


////////////////////////////////////////////////////////////////////////////////
//	assembling functions
////////////////////////////////////////////////////////////////////////////////

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void NeumannBoundaryFV<TDomain>::
prep_elem_loop(const ReferenceObjectID roid, const int si)
{
	update_subset_groups();
	m_si = si;

//	register subsetIndex at Geometry
	TFVGeom& geo = GeomProvider<TFVGeom>::get(m_lfeID,m_order);

	try{
		geo.update_local(roid, m_lfeID);
	}
	UG_CATCH_THROW("NeumannBoundaryFV::prep_elem_loop:"
						" Cannot update Finite Volume Geometry.");

//	request subset indices as boundary subset. This will force the
//	creation of boundary subsets when calling geo.update

	for(size_t i = 0; i < m_vNumberData.size(); ++i){
		if(!m_vNumberData[i].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vNumberData[i].BndSSGrp.size(); ++s){
			const int si = m_vNumberData[i].BndSSGrp[s];
			geo.add_boundary_subset(si);
		}
	}
	for(size_t i = 0; i < m_vBNDNumberData.size(); ++i){
		if(!m_vBNDNumberData[i].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vBNDNumberData[i].BndSSGrp.size(); ++s){
			const int si = m_vBNDNumberData[i].BndSSGrp[s];
			geo.add_boundary_subset(si);
		}
	}
	for(size_t i = 0; i < m_vVectorData.size(); ++i){
		if(!m_vVectorData[i].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vVectorData[i].BndSSGrp.size(); ++s){
			const int si = m_vVectorData[i].BndSSGrp[s];
			geo.add_boundary_subset(si);
		}
	}

//	clear imports, since we will set them afterwards
	this->clear_imports();

	ReferenceObjectID id = geometry_traits<TElem>::REFERENCE_OBJECT_ID;

//	set lin defect fct for imports
	for(size_t data = 0; data < m_vNumberData.size(); ++data)
	{
		if(!m_vNumberData[data].InnerSSGrp.contains(m_si)) continue;
		m_vNumberData[data].import.set_fct(id,
		                                   &m_vNumberData[data],
		                                   &NumberData::template lin_def<TElem, TFVGeom>);

		this->register_import(m_vNumberData[data].import);
		m_vNumberData[data].import.set_rhs_part();
	}
}

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void NeumannBoundaryFV<TDomain>::
prep_elem(const LocalVector& u, GridObject* elem, const ReferenceObjectID roid, const MathVector<dim> vCornerCoords[])
{
//  update Geometry for this element
	TFVGeom& geo = GeomProvider<TFVGeom>::get(m_lfeID,m_order);
	try{
		geo.update(elem,vCornerCoords,&(this->subset_handler()));
	}
	UG_CATCH_THROW("NeumannBoundaryFV::prep_elem: "
						"Cannot update Finite Volume Geometry.");

	for(size_t i = 0; i < m_vNumberData.size(); ++i)
		if(m_vNumberData[i].InnerSSGrp.contains(m_si))
			m_vNumberData[i].template extract_bip<TElem, TFVGeom>(geo);
}

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void NeumannBoundaryFV<TDomain>::
add_rhs_elem(LocalVector& d, GridObject* elem, const MathVector<dim> vCornerCoords[])
{
	TFVGeom& geo = GeomProvider<TFVGeom>::get(m_lfeID,m_order);
	typedef typename TFVGeom::BF BF;

//	Number Data
	for(size_t data = 0; data < m_vNumberData.size(); ++data){
		if(!m_vNumberData[data].InnerSSGrp.contains(m_si)) continue;
		size_t ip = 0;
		for(size_t s = 0; s < m_vNumberData[data].BndSSGrp.size(); ++s){
			const int si = m_vNumberData[data].BndSSGrp[s];
			const std::vector<BF>& vBF = geo.bf(si);

			for(size_t b = 0; b < vBF.size(); ++b){
				const int co = vBF[b].node_id();

				for(size_t i = 0; i < vBF[b].num_ip(); ++i, ++ip){
					d(_C_, co) -= m_vNumberData[data].import[ip]
					             * vBF[b].volume() * vBF[b].weight(i);
				}
			}
		}
	}

//	conditional Number Data
	for(size_t data = 0; data < m_vBNDNumberData.size(); ++data){
		if(!m_vBNDNumberData[data].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vBNDNumberData[data].BndSSGrp.size(); ++s)	{
			const int si = m_vBNDNumberData[data].BndSSGrp[s];
			const std::vector<BF>& vBF = geo.bf(si);

			for(size_t b = 0; b < vBF.size(); ++b){
				number val = 0.0;
				const int co = vBF[b].node_id();

				for(size_t i = 0; i < vBF[b].num_ip(); ++i){
					if(!(*m_vBNDNumberData[data].functor)(val, vBF[b].global_ip(i), this->time(), si))
						continue;

					d(_C_, co) -= val * vBF[b].volume() * vBF[b].weight(i);
				}
			}
		}
	}

// 	vector data
	for(size_t data = 0; data < m_vVectorData.size(); ++data){
		if(!m_vVectorData[data].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vVectorData[data].BndSSGrp.size(); ++s){
			const int si = m_vVectorData[data].BndSSGrp[s];
			const std::vector<BF>& vBF = geo.bf(si);

			for(size_t b = 0; b < vBF.size(); ++b){
				MathVector<dim> val;
				const int co = vBF[b].node_id();

				for(size_t i = 0; i < vBF[b].num_ip(); ++i){
					(*m_vVectorData[data].functor)(val, vBF[b].global_ip(i), this->time(), si);

					d(_C_, co) -= vBF[b].weight(i) * VecDot(val, vBF[b].normal());
				}
			}
		}
	}
}

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void NeumannBoundaryFV<TDomain>::
finish_elem_loop()
{
//	remove subsetIndex from Geometry
	TFVGeom& geo = GeomProvider<TFVGeom>::get(m_lfeID,m_order);

//	unrequest subset indices as boundary subset. This will force the
//	creation of boundary subsets when calling geo.update

	for(size_t i = 0; i < m_vNumberData.size(); ++i){
		if(!m_vNumberData[i].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vNumberData[i].BndSSGrp.size(); ++s){
			const int si = m_vNumberData[i].BndSSGrp[s];
			geo.remove_boundary_subset(si);
			geo.reset_curr_elem();
		}
	}

	for(size_t i = 0; i < m_vBNDNumberData.size(); ++i){
		if(!m_vBNDNumberData[i].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vBNDNumberData[i].BndSSGrp.size(); ++s){
			const int si = m_vBNDNumberData[i].BndSSGrp[s];
			geo.remove_boundary_subset(si);
			geo.reset_curr_elem();
		}
	}

	for(size_t i = 0; i < m_vVectorData.size(); ++i){
		if(!m_vVectorData[i].InnerSSGrp.contains(m_si)) continue;
		for(size_t s = 0; s < m_vVectorData[i].BndSSGrp.size(); ++s){
			const int si = m_vVectorData[i].BndSSGrp[s];
			geo.remove_boundary_subset(si);
			geo.reset_curr_elem();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Number Data
////////////////////////////////////////////////////////////////////////////////

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void NeumannBoundaryFV<TDomain>::NumberData::
lin_def(const LocalVector& u,
            std::vector<std::vector<number> > vvvLinDef[],
            const size_t nip)
{
//  get finite volume geometry
	const TFVGeom& geo = GeomProvider<TFVGeom>::get(This->m_lfeID,This->m_order);
	typedef typename TFVGeom::BF BF;

	size_t ip = 0;
	for(size_t s = 0; s < this->BndSSGrp.size(); ++s)
	{
		const int si = this->BndSSGrp[s];
		const std::vector<BF>& vBF = geo.bf(si);
		for(size_t i = 0; i < vBF.size(); ++i){
			const int co = vBF[i].node_id();
			vvvLinDef[ip][_C_][co] -= vBF[i].volume();
		}
	}
}

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void NeumannBoundaryFV<TDomain>::NumberData::
extract_bip(const TFVGeom& geo)
{
	typedef typename TFVGeom::BF BF;
	static const int locDim = TElem::dim;

	std::vector<MathVector<locDim> >* vLocIP = local_ips<locDim>();

	vLocIP->clear();
	vGloIP.clear();
	for(size_t s = 0; s < this->BndSSGrp.size(); s++)
	{
		const int si = this->BndSSGrp[s];
		const std::vector<BF>& vBF = geo.bf(si);
		for(size_t i = 0; i < vBF.size(); ++i)
		{
			const BF& bf = vBF[i];
			for(size_t ip = 0; ip < bf.num_ip(); ++ip){
				vLocIP->push_back(bf.local_ip(ip));
				vGloIP.push_back(bf.global_ip(ip));
			}
		}
	}

	import.template set_local_ips<locDim>(&(*vLocIP)[0], vLocIP->size());
	import.set_global_ips(&vGloIP[0], vGloIP.size());
}

template<typename TDomain>
template <int refDim>
std::vector<MathVector<refDim> >* NeumannBoundaryFV<TDomain>::NumberData::local_ips()
{
	switch (refDim)
	{
		case 1: return (std::vector<MathVector<refDim> >*)(&vLocIP_dim1);
		case 2: return (std::vector<MathVector<refDim> >*)(&vLocIP_dim2);
		case 3: return (std::vector<MathVector<refDim> >*)(&vLocIP_dim3);
	}
}

////////////////////////////////////////////////////////////////////////////////
//	register assemble functions
////////////////////////////////////////////////////////////////////////////////

#ifdef UG_DIM_1
template<>
void NeumannBoundaryFV<Domain1d>::register_all_funcs(int order)
{
	// Edge
	register_func<RegularEdge, DimFVGeometry<dim, 1> >();
}
#endif

#ifdef UG_DIM_2
template<>
void NeumannBoundaryFV<Domain2d>::register_all_funcs(int order)
{
// Edge
	register_func<RegularEdge, DimFVGeometry<dim, 1> >();

//	Triangle
	switch(order)
	{
		case 1:	{typedef FVGeometry<1, Triangle, dim> FVGeom;
				 register_func<Triangle, FVGeom >(); break;}
		case 2:	{typedef FVGeometry<2, Triangle, dim> FVGeom;
				 register_func<Triangle, FVGeom >(); break;}
		case 3:	{typedef FVGeometry<3, Triangle, dim> FVGeom;
				 register_func<Triangle, FVGeom >(); break;}
		default: {typedef DimFVGeometry<dim, 2> FVGeom;
				 register_func<Triangle, FVGeom >(); break;}
	}

//	Quadrilateral
	switch(order) {
		case 1:	{typedef FVGeometry<1, Quadrilateral, dim> FVGeom;
				 register_func<Quadrilateral, FVGeom >(); break;}
		case 2:	{typedef FVGeometry<2, Quadrilateral, dim> FVGeom;
				 register_func<Quadrilateral, FVGeom >(); break;}
		case 3:	{typedef FVGeometry<3, Quadrilateral, dim> FVGeom;
				 register_func<Quadrilateral, FVGeom >(); break;}
		default: {typedef DimFVGeometry<dim, 2> FVGeom;
				  register_func<Quadrilateral, FVGeom >(); break;}
	}
}
#endif

#ifdef UG_DIM_3
template<>
void NeumannBoundaryFV<Domain3d>::register_all_funcs(int order)
{
// Edge
	register_func<RegularEdge, DimFVGeometry<dim, 1> >();

//	Tetrahedron
	switch(order)
	{
		case 1:	{typedef FVGeometry<1, Tetrahedron, dim> FVGeom;
				 register_func<Tetrahedron, FVGeom >(); break;}
		case 2:	{typedef FVGeometry<2, Tetrahedron, dim> FVGeom;
				 register_func<Tetrahedron, FVGeom >(); break;}
		case 3:	{typedef FVGeometry<3, Tetrahedron, dim> FVGeom;
				 register_func<Tetrahedron, FVGeom >(); break;}
		default: {typedef DimFVGeometry<dim, 3> FVGeom;
				  register_func<Tetrahedron, FVGeom >(); break;}
	}

//	Prism
	switch(order) {
		case 1:	{typedef FVGeometry<1, Prism, dim> FVGeom;
				 register_func<Prism, FVGeom >(); break;}
		default: {typedef DimFVGeometry<dim, 3> FVGeom;
				  register_func<Prism, FVGeom >(); break;}
	}

//	Hexahedron
	switch(order)
	{
		case 1:	{typedef FVGeometry<1, Hexahedron, dim> FVGeom;
				 register_func<Hexahedron, FVGeom >(); break;}
		case 2:	{typedef FVGeometry<2, Hexahedron, dim> FVGeom;
				 register_func<Hexahedron, FVGeom >(); break;}
		case 3:	{typedef FVGeometry<3, Hexahedron, dim> FVGeom;
				 register_func<Hexahedron, FVGeom >(); break;}
		default: {typedef DimFVGeometry<dim, 3> FVGeom;
				  register_func<Hexahedron, FVGeom >(); break;}
	}
}
#endif

template<typename TDomain>
template<typename TElem, typename TFVGeom>
void NeumannBoundaryFV<TDomain>::register_func()
{
	ReferenceObjectID id = geometry_traits<TElem>::REFERENCE_OBJECT_ID;
	typedef this_type T;

	this->clear_add_fct(id);

	this->set_prep_elem_loop_fct(id, &T::template prep_elem_loop<TElem, TFVGeom>);
	this->set_prep_elem_fct(	 id, &T::template prep_elem<TElem, TFVGeom>);
	this->set_add_rhs_elem_fct(	 id, &T::template add_rhs_elem<TElem, TFVGeom>);
	this->set_fsh_elem_loop_fct( id, &T::template finish_elem_loop<TElem, TFVGeom>);

	this->set_add_jac_A_elem_fct(	 id, &T::template add_jac_A_elem<TElem, TFVGeom>);
	this->set_add_jac_M_elem_fct(	 id, &T::template add_jac_M_elem<TElem, TFVGeom>);
	this->set_add_def_A_elem_fct(	 id, &T::template add_def_A_elem<TElem, TFVGeom>);
	this->set_add_def_M_elem_fct(	 id, &T::template add_def_M_elem<TElem, TFVGeom>);
}

////////////////////////////////////////////////////////////////////////////////
//	explicit template instantiations
////////////////////////////////////////////////////////////////////////////////

#ifdef UG_DIM_1
template class NeumannBoundaryFV<Domain1d>;
#endif
#ifdef UG_DIM_2
template class NeumannBoundaryFV<Domain2d>;
#endif
#ifdef UG_DIM_3
template class NeumannBoundaryFV<Domain3d>;
#endif

} // namespace ug


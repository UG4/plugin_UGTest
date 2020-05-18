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

#include "adaption_surface_grid_function.h"
#include "lib_disc/dof_manager/orientation.h"
#include "lib_disc/function_spaces/local_transfer_interface.h"

namespace ug{

template <typename TDomain>
AdaptionSurfaceGridFunction<TDomain>::
AdaptionSurfaceGridFunction(SmartPtr<TDomain> spDomain, bool bObserveStorage)
	:	m_spDomain(spDomain),
	 	m_spGrid(spDomain->grid()),
	 	m_bObserveStorage(bObserveStorage),
	 	m_aValue(true)
{}

template <typename TDomain>
void AdaptionSurfaceGridFunction<TDomain>::
prolongate(const GridMessage_Adaption& msg)
{
//	\todo: this temporary - handle more flexible
	m_vpProlong.clear();
	for(size_t fct = 0; fct < m_spDDInfo->num_fct(); ++fct)
		m_vpProlong.push_back(GetStandardElementProlongation<TDomain>(m_spDDInfo->lfeid(fct)));

	const GridObjectCollection& goc = msg.affected_elements();
	for(size_t lvl = 0; lvl < goc.num_levels(); ++lvl)
	{
		prolongate<Vertex>(msg, lvl);
		prolongate<Edge>(msg, lvl);
		prolongate<Face>(msg, lvl);
		prolongate<Volume>(msg, lvl);
	}
}

template <typename TDomain>
template <typename TBaseElem>
void AdaptionSurfaceGridFunction<TDomain>::
prolongate(const GridMessage_Adaption& msg, const size_t lvl)
{
	const GridBaseObjectId gbo = (GridBaseObjectId)TBaseElem::BASE_OBJECT_ID;

//	extract and init prolongations working on the base element type
	std::vector<SmartPtr<IElemProlongation<TDomain> > > vpProlong;
	for(size_t fct = 0; fct < m_vpProlong.size(); ++fct){

		if(!m_vpProlong[fct]->perform_prolongation_on(gbo)) continue;

		m_vpProlong[fct]->init(m_spDomain,
		                       make_sp(new ValueAccessor(*this, fct)),
		                       make_sp(new ValueAccessor(*this, fct)));

		vpProlong.push_back(m_vpProlong[fct]);
	}

//	check that something to do
	if(vpProlong.empty()) return;

//	iterators
	const GridObjectCollection& goc = msg.affected_elements();
	typedef typename GridObjectCollection::traits<TBaseElem>::const_iterator const_iterator;


	const_iterator iter = goc.begin<TBaseElem>(lvl);
	const_iterator iterEnd = goc.end<TBaseElem>(lvl);

//	loop base element type
	for( ; iter != iterEnd; ++iter)
	{
	//	get parent element, that has been refined
		TBaseElem* parent = *iter;

	//	call implementations
		for(size_t f = 0; f < vpProlong.size(); ++f){
			vpProlong[f]->prolongate(parent);
		}
	}
}

//template <typename TElem, typename TSubElem>
//static void select_associated(MGSelector& sel, TElem* elem)
//{
//	MultiGrid* mg = sel.multi_grid();
//
//	typename Grid::traits<TSubElem>::secure_container vSubElem;
//	mg->associated_elements(vSubElem, elem);
//	for(size_t i = 0; i < vSubElem.size(); ++i)
//		sel.select(vSubElem[i]);
//}
//
//template <typename TElem>
//static void select_associated(MGSelector& sel, TElem* elem)
//{
//	// add all subelements
//	switch(TElem::dim){
//		case 3: select_associated<TElem, Face>(sel, elem);
//		case 2: select_associated<TElem, Edge>(sel, elem);
//		case 1: select_associated<TElem, Vertex>(sel, elem);
//		default: ;
//	}
//}
//
//static void select_associated(MGSelector& sel, GridObject* elem)
//{
//	switch(elem->base_object_id()){
//		case VERTEX: return; // no sub elems
//		case EDGE: select_associated(sel, static_cast<Edge*>(elem)); return;
//		case FACE: select_associated(sel, static_cast<Face*>(elem)); return;
//		case VOLUME: select_associated(sel, static_cast<Volume*>(elem)); return;
//		default: UG_THROW("Dim not supported.");
//	}
//}


template <typename TDomain>
template <typename TBaseElem>
void AdaptionSurfaceGridFunction<TDomain>::
select_parents(MGSelector& sel, const GridMessage_Adaption& msg)
{
//	iterators
	const GridObjectCollection& goc = msg.affected_elements();
	typedef typename GridObjectCollection::traits<TBaseElem>::const_iterator const_iterator;

	for(size_t lvl = 0; lvl < goc.num_levels(); ++lvl)
	{
		const_iterator iter = goc.begin<TBaseElem>(lvl);
		const_iterator iterEnd = goc.end<TBaseElem>(lvl);

	//	loop base element type
		for( ; iter != iterEnd; ++iter)
		{
		//	get parent element, that is marked for coarsening
			TBaseElem* elem = *iter;

			// exclude if not in surface or no shadow
			if(m_aaValue[elem].size() != m_spDDInfo->num_fct())
				continue;

		//	get parent
			GridObject* parent = sel.multi_grid()->get_parent(elem);

		//	check whether a parent exist and if it is of the same type as the
		//	elements considered. Parents of a different type are handled in
		//	other calls to this method
			if(!parent || (parent->base_object_id() != TBaseElem::BASE_OBJECT_ID))
				continue;

		//	select parent
			sel.select(parent);
		}
	}
}

template <typename TDomain>
void AdaptionSurfaceGridFunction<TDomain>::
do_restrict(const GridMessage_Adaption& msg)
{
//	\todo: this temporary - handle more flexible
	m_vpRestrict.clear();
	for(size_t fct = 0; fct < m_spDDInfo->num_fct(); ++fct)
		m_vpRestrict.push_back(GetStandardElementRestriction<TDomain>(m_spDDInfo->lfeid(fct)));

	MGSelector sel(*m_spGrid);

	if(m_aaValue.is_valid_vertex_accessor())
		select_parents<Vertex>(sel, msg);
	if(m_aaValue.is_valid_edge_accessor())
		select_parents<Edge>(sel, msg);
	if(m_aaValue.is_valid_face_accessor())
		select_parents<Face>(sel, msg);
	if(m_aaValue.is_valid_volume_accessor())
		select_parents<Volume>(sel, msg);

	do_restrict<Vertex>(sel, msg);
	do_restrict<Edge>(sel, msg);
	do_restrict<Face>(sel, msg);
	do_restrict<Volume>(sel, msg);
}

template <typename TDomain>
template <typename TBaseElem>
void AdaptionSurfaceGridFunction<TDomain>::
do_restrict(const MGSelector& sel, const GridMessage_Adaption& msg)
{
	const GridBaseObjectId gbo = (GridBaseObjectId)TBaseElem::BASE_OBJECT_ID;

//	extract and init prolongations working on the base element type
	std::vector<SmartPtr<IElemRestriction<TDomain> > > vpRestrict;
	for(size_t fct = 0; fct < m_vpRestrict.size(); ++fct){

		if(!m_vpRestrict[fct]->perform_restriction_on(gbo)) continue;

		m_vpRestrict[fct]->init(m_spDomain,
		                        make_sp(new ValueAccessor(*this, fct)),
		                        make_sp(new ValueAccessor(*this, fct)));

		vpRestrict.push_back(m_vpRestrict[fct]);
	}

//	check that something to do
	if(vpRestrict.empty()) return;

//	check that correct attachments used
	if(m_spDDInfo->max_dofs(gbo) == 0) return;

//	iterators
	typedef typename Selector::traits<TBaseElem>::const_level_iterator const_iterator;

	for(int lvl = sel.num_levels() - 1; lvl >= 0; --lvl)
	{
		const_iterator iter = sel.begin<TBaseElem>(lvl);
		const_iterator iterEnd = sel.end<TBaseElem>(lvl);

	//	loop base element type
		for( ; iter != iterEnd; ++iter)
		{
		//	get parent element, that has been refined
			TBaseElem* parent = *iter;

		//	add storage for parent
			obj_created(parent);

		//	call implementations
			for(size_t f = 0; f < vpRestrict.size(); ++f){
				vpRestrict[f]->do_restrict(parent);
			}
		}
	}
}


template <typename TDomain>
void AdaptionSurfaceGridFunction<TDomain>::
attach_entries(ConstSmartPtr<DoFDistributionInfo> spDDInfo)
{
	GFUNCADAPT_PROFILE_FUNC();
	if(m_spGrid.invalid()) UG_THROW("Grid missing")

	m_spDDInfo = spDDInfo;

//	get required elem types
	const bool vrt = (spDDInfo->max_dofs(VERTEX) > 0);
	const bool edge = (spDDInfo->max_dofs(EDGE) > 0);
	const bool face = (spDDInfo->max_dofs(FACE) > 0);
	const bool vol = (spDDInfo->max_dofs(VOLUME) > 0);

//	attach storage arrays
	if(vrt) m_spGrid->attach_to<Vertex>(m_aValue);
	if(edge) m_spGrid->attach_to<Edge>(m_aValue);
	if(face) m_spGrid->attach_to<Face>(m_aValue);
	if(vol) m_spGrid->attach_to<Volume>(m_aValue);

//	prepare access
	m_aaValue.access(*m_spGrid, m_aValue, vrt, edge, face, vol);

	if(m_bObserveStorage)
	{
	//	get type of observer
		int type = OT_GRID_OBSERVER;
		if(vrt) type |= OT_VERTEX_OBSERVER;
		if(edge) type |= OT_EDGE_OBSERVER;
		if(face) type |= OT_FACE_OBSERVER;
		if(vol) type |= OT_VOLUME_OBSERVER;

	//	register observer
		m_spGrid->register_observer(this, type);
	}
}

template <typename TDomain>
template <typename TElem>
void AdaptionSurfaceGridFunction<TDomain>::detach_entries()
{
	if(m_spGrid.invalid()) UG_THROW("Grid missing")

	if(m_spGrid->has_attachment<TElem>(m_aValue))
		m_spGrid->detach_from<TElem>(m_aValue);
}

template <typename TDomain>
void AdaptionSurfaceGridFunction<TDomain>::detach_entries()
{
	GFUNCADAPT_PROFILE_FUNC();
//	remove attachments
	if(m_aaValue.is_valid_vertex_accessor()) detach_entries<Vertex>();
	if(m_aaValue.is_valid_edge_accessor()) detach_entries<Edge>();
	if(m_aaValue.is_valid_face_accessor()) detach_entries<Face>();
	if(m_aaValue.is_valid_volume_accessor()) detach_entries<Volume>();
	m_aaValue = MultiElementAttachmentAccessor<AValues>();

//	stop observing grid
	if(m_bObserveStorage)
		m_spGrid->unregister_observer(this);
}

////////////////////////////////////////////////////////////////////////////////
// Value accessor
////////////////////////////////////////////////////////////////////////////////

template <typename TDomain>
AdaptionSurfaceGridFunction<TDomain>::ValueAccessor::
ValueAccessor(AdaptionSurfaceGridFunction<TDomain>& rASGF,
			  size_t fct)
	: m_rASGF(rASGF), m_fct(fct)
{
	for(int g = 0; g < NUM_GEOMETRIC_BASE_OBJECTS; ++g)
	{
		const GridBaseObjectId gbo = (GridBaseObjectId)g;
		m_HasDoFs[gbo] = (m_rASGF.m_spDDInfo->max_fct_dofs(fct, gbo) > 0);
	}
}

template <typename TDomain>
void
AdaptionSurfaceGridFunction<TDomain>::ValueAccessor::
access_inner(GridObject* elem)
{
	UG_ASSERT(m_fct < m_rASGF.m_aaValue[elem].size(), "Only storage for "
	          <<m_rASGF.m_aaValue[elem].size()<<" fcts, but fct-cmp "
	          <<m_fct<<" requested on "<< ElementDebugInfo(*m_rASGF.m_spDomain->grid(), elem))
	std::vector<number>& vVal = m_rASGF.m_aaValue[elem][m_fct];
	m_Val.resize(vVal.size());
	for(size_t i = 0; i < vVal.size(); ++i)
		m_Val[i] = &vVal[i];
}

template <typename TDomain>
template <typename TBaseElem, typename TSubBaseElem>
void
AdaptionSurfaceGridFunction<TDomain>::ValueAccessor::
access_closure(TBaseElem* elem)
{

	typename Grid::traits<TSubBaseElem>::secure_container vSubElem;
	m_rASGF.m_spGrid->associated_elements_sorted(vSubElem, elem);

	std::vector<size_t> vOrientOffset;

	for(size_t i = 0; i < vSubElem.size(); ++i)
	{
	//	get subelement
		TSubBaseElem* subElem = vSubElem[i];
		UG_ASSERT(m_fct < m_rASGF.m_aaValue[subElem].size(), "Only storage for "
		          <<m_rASGF.m_aaValue[subElem].size()<<" fcts, but fct-cmp "
		          <<m_fct<<" requested on "<<ElementDebugInfo(*m_rASGF.m_spDomain->grid(), subElem))
		std::vector<number>& vVal = m_rASGF.m_aaValue[subElem][m_fct];

	//	get the orientation for this subelement
		ComputeOrientationOffset(vOrientOffset, elem, subElem, i,
								 m_rASGF.m_spDDInfo->lfeid(m_fct));

		UG_ASSERT(vOrientOffset.size() == vVal.size() ||
				  vOrientOffset.empty(), "Orientation wrong");

	//	cache access
		if(vOrientOffset.empty()){
			for(size_t j = 0; j < vVal.size(); ++j)
				m_Val.push_back(&vVal[ j ]);
		}else{
			for(size_t j = 0; j < vVal.size(); ++j)
				m_Val.push_back(&vVal[ vOrientOffset[j] ]);
		}
	}
}

template <typename TDomain>
template <typename TBaseElem>
void
AdaptionSurfaceGridFunction<TDomain>::ValueAccessor::
access_closure(TBaseElem* elem)
{
	m_Val.clear();
	if(m_HasDoFs[VERTEX]) access_closure<TBaseElem, Vertex>(elem);
	if(m_HasDoFs[EDGE]) access_closure<TBaseElem, Edge>(elem);
	if(m_HasDoFs[FACE]) access_closure<TBaseElem, Face>(elem);
	if(m_HasDoFs[VOLUME]) access_closure<TBaseElem, Volume>(elem);
}

template <typename TDomain>
void
AdaptionSurfaceGridFunction<TDomain>::ValueAccessor::
access_closure(GridObject* elem)
{
	switch(elem->base_object_id()){
		case VERTEX: access_closure(static_cast<Vertex*>(elem)); return;
		case EDGE: access_closure(static_cast<Edge*>(elem)); return;
		case FACE: access_closure(static_cast<Face*>(elem)); return;
		case VOLUME: access_closure(static_cast<Volume*>(elem)); return;
		default: UG_THROW("Base object id not found.")
	}
}


#ifdef UG_DIM_1
template class AdaptionSurfaceGridFunction<Domain1d>;
#endif
#ifdef UG_DIM_2
template class AdaptionSurfaceGridFunction<Domain2d>;
#endif
#ifdef UG_DIM_3
template class AdaptionSurfaceGridFunction<Domain3d>;
#endif

} // end namespace ug

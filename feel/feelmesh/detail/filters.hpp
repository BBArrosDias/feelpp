/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*-

 This file is part of the Feel++ library

 Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
 Date: 29 juil. 2015

 Copyright (C) 2015 Feel++ Consortium

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef FEELPP_FEELMESH_DETAIL_FILTERS_HPP
#define FEELPP_FEELMESH_DETAIL_FILTERS_HPP 1

#include <feel/feelcore/unwrapptr.hpp>

namespace Feel {

#pragma GCC visibility push(hidden)
/// \cond detail
namespace detail
{

template <typename RangeType>
struct submeshrangetype
{
    typedef typename mpl::if_< boost::is_std_list<RangeType>,
                               mpl::identity<RangeType>,
                               mpl::identity<std::list<RangeType> > >::type::type::value_type type;
};



template<typename MeshType>
boost::tuple<mpl::size_t<MESH_ELEMENTS>,
             typename MeshTraits<MeshType>::element_const_iterator,
             typename MeshTraits<MeshType>::element_const_iterator>
allelements( MeshType const& mesh )
{
    return boost::make_tuple( mpl::size_t<MESH_ELEMENTS>(),
                              Feel::unwrap_ptr( mesh ).beginElement(),
                              Feel::unwrap_ptr( mesh ).endElement() );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_ELEMENTS>,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::elements_reference_wrapper_ptrtype >
elements( MeshType const& mesh, rank_type pid )
{
    auto rangeElements = Feel::unwrap_ptr( mesh ).elementsWithProcessId( pid );
    return boost::make_tuple( mpl::size_t<MESH_ELEMENTS>(),
                              std::get<0>( rangeElements ),
                              std::get<1>( rangeElements ),
                              std::get<2>( rangeElements ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_ELEMENTS>,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::elements_reference_wrapper_ptrtype >
boundaryelements( MeshType const& mesh, uint16_type entity_min_dim, uint16_type entity_max_dim, rank_type pid )
{
    auto rangeElements = Feel::unwrap_ptr( mesh ).boundaryElements( entity_min_dim,entity_max_dim,pid );
    return boost::make_tuple( mpl::size_t<MESH_ELEMENTS>(),
                              std::get<0>( rangeElements ),
                              std::get<1>( rangeElements ),
                              std::get<2>( rangeElements ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_ELEMENTS>,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::elements_reference_wrapper_ptrtype >
internalelements( MeshType const& mesh, rank_type pid )
{
    auto rangeElements = Feel::unwrap_ptr( mesh ).internalElements( pid );
    return boost::make_tuple( mpl::size_t<MESH_ELEMENTS>(),
                              std::get<0>( rangeElements ),
                              std::get<1>( rangeElements ),
                              std::get<2>( rangeElements ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_ELEMENTS>,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::elements_reference_wrapper_ptrtype >
markedelements( MeshType const& mesh, uint16_type markerType, std::set<flag_type> const& markersFlag, rank_type pid  )
{
    auto rangeElementsWithMarker = Feel::unwrap_ptr( mesh ).elementsWithMarkerByType( markerType, markersFlag, pid );
    return boost::make_tuple( mpl::size_t<MESH_ELEMENTS>(),
                              std::get<0>( rangeElementsWithMarker ),
                              std::get<1>( rangeElementsWithMarker ),
                              std::get<2>( rangeElementsWithMarker ) );
}
template<typename MeshType>
boost::tuple<mpl::size_t<MESH_ELEMENTS>,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::elements_reference_wrapper_ptrtype >
markedelements( MeshType const& mesh, uint16_type markerType, rank_type pid  )
{
    auto rangeElementsWithMarker = Feel::unwrap_ptr( mesh ).elementsWithMarkerByType( markerType, pid );
    return boost::make_tuple( mpl::size_t<MESH_ELEMENTS>(),
                              std::get<0>( rangeElementsWithMarker ),
                              std::get<1>( rangeElementsWithMarker ),
                              std::get<2>( rangeElementsWithMarker ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_ELEMENTS>,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::element_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::elements_reference_wrapper_ptrtype >
idedelements( MeshType const& mesh, size_type id )
{
    auto rangeElementsWithId = Feel::unwrap_ptr( mesh ).elementsWithId( id );
    return boost::make_tuple( mpl::size_t<MESH_ELEMENTS>(),
                              std::get<0>( rangeElementsWithId ),
                              std::get<1>( rangeElementsWithId ),
                              std::get<2>( rangeElementsWithId ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_FACES>,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::faces_reference_wrapper_ptrtype >
faces( MeshType const& mesh, rank_type __pid )
{
    auto rangeFaces = Feel::unwrap_ptr( mesh ).facesWithProcessId( __pid );
    return boost::make_tuple( mpl::size_t<MESH_FACES>(),
                              std::get<0>( rangeFaces ),
                              std::get<1>( rangeFaces ),
                              std::get<2>( rangeFaces ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_FACES>,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::faces_reference_wrapper_ptrtype >
idedfaces( MeshType const& mesh, size_type id )
{
    auto rangeFaces = Feel::unwrap_ptr( mesh ).facesWithId( id );
    return boost::make_tuple( mpl::size_t<MESH_FACES>(),
                              std::get<0>( rangeFaces ),
                              std::get<1>( rangeFaces ),
                              std::get<2>( rangeFaces ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_FACES>,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::faces_reference_wrapper_ptrtype >
markedfaces( MeshType const& mesh, uint16_type markerType, rank_type pid )
{
    auto rangeMarkedFaces = Feel::unwrap_ptr( mesh ).facesWithMarkerByType( markerType, pid );
    return boost::make_tuple( mpl::size_t<MESH_FACES>(),
                              std::get<0>( rangeMarkedFaces ),
                              std::get<1>( rangeMarkedFaces ),
                              std::get<2>( rangeMarkedFaces ) );

}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_FACES>,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::faces_reference_wrapper_ptrtype >
markedfaces( MeshType const& mesh, uint16_type markerType, std::set<flag_type> const& markersFlag, rank_type pid )
{
    auto rangeMarkedFaces = Feel::unwrap_ptr( mesh ).facesWithMarkerByType( markerType, markersFlag, pid );
    return boost::make_tuple( mpl::size_t<MESH_FACES>(),
                              std::get<0>( rangeMarkedFaces ),
                              std::get<1>( rangeMarkedFaces ),
                              std::get<2>( rangeMarkedFaces ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_FACES>,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::faces_reference_wrapper_ptrtype >
boundaryfaces( MeshType const& mesh, rank_type __pid  )
{
    auto rangeBoundaryFaces = Feel::unwrap_ptr( mesh ).facesOnBoundary( __pid );
    return boost::make_tuple( mpl::size_t<MESH_FACES>(),
                              std::get<0>( rangeBoundaryFaces ),
                              std::get<1>( rangeBoundaryFaces ),
                              std::get<2>( rangeBoundaryFaces ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_FACES>,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::faces_reference_wrapper_ptrtype >
internalfaces( MeshType const& mesh, rank_type __pid )
{
    auto rangeInternalFaces = Feel::unwrap_ptr( mesh ).internalFaces( __pid );
    return boost::make_tuple( mpl::size_t<MESH_FACES>(),
                              std::get<0>( rangeInternalFaces ),
                              std::get<1>( rangeInternalFaces ),
                              std::get<2>( rangeInternalFaces ) );

}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_FACES>,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::face_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::faces_reference_wrapper_ptrtype >
interprocessfaces( MeshType const& mesh, rank_type neighbor_pid )
{
    auto rangeInterprocessFaces = Feel::unwrap_ptr( mesh ).interProcessFaces( neighbor_pid );
    return boost::make_tuple( mpl::size_t<MESH_FACES>(),
                              std::get<0>( rangeInterprocessFaces ),
                              std::get<1>( rangeInterprocessFaces ),
                              std::get<2>( rangeInterprocessFaces ) );
}

template<typename MeshType>
decltype(auto)
edges( MeshType const& mesh, rank_type __pid )
{
    auto rangeMarkedEdges = Feel::unwrap_ptr( mesh ).edgesWithProcessId( __pid );
    return boost::make_tuple( mpl::size_t<MESH_EDGES>(),
                              std::get<0>( rangeMarkedEdges ),
                              std::get<1>( rangeMarkedEdges ),
                              std::get<2>( rangeMarkedEdges ) );
}


template<typename MeshType>
boost::tuple<mpl::size_t<MESH_EDGES>,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edges_reference_wrapper_ptrtype >
markededges( MeshType const& mesh, uint16_type markerType, rank_type pid )
{
    auto rangeMarkedEdges = Feel::unwrap_ptr( mesh ).edgesWithMarkerByType( markerType, pid );
    return boost::make_tuple( mpl::size_t<MESH_EDGES>(),
                              std::get<0>( rangeMarkedEdges ),
                              std::get<1>( rangeMarkedEdges ),
                              std::get<2>( rangeMarkedEdges ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_EDGES>,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edges_reference_wrapper_ptrtype >
markededges( MeshType const& mesh, uint16_type markerType, std::set<flag_type> const& markersFlag, rank_type pid )
{
    auto rangeMarkedEdges = Feel::unwrap_ptr( mesh ).edgesWithMarkerByType( markerType, markersFlag, pid );
    return boost::make_tuple( mpl::size_t<MESH_EDGES>(),
                              std::get<0>( rangeMarkedEdges ),
                              std::get<1>( rangeMarkedEdges ),
                              std::get<2>( rangeMarkedEdges ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_EDGES>,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edges_reference_wrapper_ptrtype >
boundaryedges( MeshType const& mesh )
{
    auto rangeBoundaryEdges = Feel::unwrap_ptr( mesh ).edgesOnBoundary();
    return boost::make_tuple( mpl::size_t<MESH_EDGES>(),
                              std::get<0>( rangeBoundaryEdges ),
                              std::get<1>( rangeBoundaryEdges ),
                              std::get<2>( rangeBoundaryEdges ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_EDGES>,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edge_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::edges_reference_wrapper_ptrtype >
internaledges( MeshType const& mesh )
{
    auto rangeInternalEdges = Feel::unwrap_ptr( mesh ).internalEdges();
    return boost::make_tuple( mpl::size_t<MESH_EDGES>(),
                              std::get<0>( rangeInternalEdges ),
                              std::get<1>( rangeInternalEdges ),
                              std::get<2>( rangeInternalEdges ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_POINTS>,
             typename MeshTraits<MeshType>::point_const_iterator,
             typename MeshTraits<MeshType>::point_const_iterator>
allpoints( MeshType const& mesh )
{
    return boost::make_tuple( mpl::size_t<MESH_POINTS>(),
                              Feel::unwrap_ptr( mesh ).beginPoint(),
                              Feel::unwrap_ptr( mesh ).endPoint() );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_POINTS>,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::points_reference_wrapper_ptrtype >
points( MeshType const& mesh )
{
    auto rangePoints = Feel::unwrap_ptr( mesh ).pointsWithProcessId();
    return boost::make_tuple( mpl::size_t<MESH_POINTS>(),
                              std::get<0>( rangePoints ),
                              std::get<1>( rangePoints ),
                              std::get<2>( rangePoints ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_POINTS>,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::points_reference_wrapper_ptrtype >
markedpoints( MeshType const& mesh, uint16_type markerType, rank_type pid )
{
    auto rangePointsWithMarker = Feel::unwrap_ptr( mesh ).pointsWithMarkerByType( markerType, pid );
    return boost::make_tuple( mpl::size_t<MESH_POINTS>(),
                              std::get<0>( rangePointsWithMarker ),
                              std::get<1>( rangePointsWithMarker ),
                              std::get<2>( rangePointsWithMarker ) );
}
template<typename MeshType>
boost::tuple<mpl::size_t<MESH_POINTS>,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::points_reference_wrapper_ptrtype >
markedpoints( MeshType const& mesh, uint16_type markerType, std::set<flag_type> const& markersFlag, rank_type pid )
{
    auto rangePointsWithMarker = Feel::unwrap_ptr( mesh ).pointsWithMarkerByType( markerType, markersFlag, pid );
    return boost::make_tuple( mpl::size_t<MESH_POINTS>(),
                              std::get<0>( rangePointsWithMarker ),
                              std::get<1>( rangePointsWithMarker ),
                              std::get<2>( rangePointsWithMarker ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_POINTS>,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::points_reference_wrapper_ptrtype >
boundarypoints( MeshType const& mesh )
{
    auto rangeBoundaryPoints = Feel::unwrap_ptr( mesh ).boundaryPoints();
    return boost::make_tuple( mpl::size_t<MESH_POINTS>(),
                              std::get<0>( rangeBoundaryPoints ),
                              std::get<1>( rangeBoundaryPoints ),
                              std::get<2>( rangeBoundaryPoints ) );
}

template<typename MeshType>
boost::tuple<mpl::size_t<MESH_POINTS>,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::point_reference_wrapper_const_iterator,
             typename MeshTraits<MeshType>::points_reference_wrapper_ptrtype >
internalpoints( MeshType const& mesh )
{
    auto rangeInternalPoints = Feel::unwrap_ptr( mesh ).internalPoints();
    return boost::make_tuple( mpl::size_t<MESH_POINTS>(),
                              std::get<0>( rangeInternalPoints ),
                              std::get<1>( rangeInternalPoints ),
                              std::get<2>( rangeInternalPoints ) );
}

} // detail
/// \endcond
#pragma GCC visibility pop

}
#endif

/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-09-03

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2007 Universit� Joseph Fourier (Grenoble I)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
   \file elements.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-09-03
 */
#ifndef __elements_H
#define __elements_H 1


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <life/lifemesh/geoelement.hpp>

namespace Life
{
namespace multi_index = boost::multi_index;
/// \cond detail
/*!
  \class Elements
  \brief Elements container class

  @author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
  @see
*/
template<typename ElementType>
class Elements
{
public:


    /** @name Typedefs
     */
    //@{

    /**
     * Element type depending on the dimension, @see geoelement.hpp
     * \note Elements have their topological dimension equal to the
     * dimension of the geometric space.
     */
    typedef typename mpl::if_<mpl::equal_to<mpl::int_<ElementType::nDim>, mpl::int_<3> >,
                              mpl::identity<GeoElement3D<ElementType::nRealDim, ElementType> >,
                              typename mpl::if_<mpl::equal_to<mpl::int_<ElementType::nDim>, mpl::int_<2> >,
                                                mpl::identity<GeoElement2D<ElementType::nRealDim, ElementType> >,
                                                mpl::identity<GeoElement1D<ElementType::nRealDim, ElementType> > >::type>::type::type element_type;

    /**
     * multi-indexed element container
     */
    typedef multi_index::multi_index_container<
        element_type,
        multi_index::indexed_by<

            // sort by less<int> on id() + pid()
            multi_index::ordered_unique<
                multi_index::composite_key<element_type,
                                           multi_index::const_mem_fun<element_type,
                                                                      uint16_type,
                                                                      &element_type::processId>,
                                           multi_index::const_mem_fun<element_type,
                                                                      size_type,
                                                                      &element_type::id> > >,
            // sort by less<int> on marker
            multi_index::ordered_non_unique<multi_index::tag<detail::by_marker>,
                                            multi_index::composite_key<element_type,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  Marker1 const&,
                                                                                                  &element_type::marker>,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  uint16_type,
                                                                                                  &element_type::processId> > >,
            // sort by less<int> on marker
            multi_index::ordered_non_unique<multi_index::tag<detail::by_marker2>,
                                            multi_index::composite_key<element_type,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  Marker2 const&,
                                                                                                  &element_type::marker2>,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  uint16_type,
                                                                                                  &element_type::processId> > >,

            // sort by less<int> on marker
            multi_index::ordered_non_unique<multi_index::tag<detail::by_marker3>,
                                            multi_index::composite_key<element_type,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  Marker3 const&,
                                                                                                  &element_type::marker3>,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  uint16_type,
                                                                                                  &element_type::processId> > >,

            // sort by less<int> on boundary
            multi_index::ordered_non_unique<multi_index::tag<detail::by_location>,
                                            multi_index::composite_key<element_type,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  bool,
                                                                                                  &element_type::isOnBoundary>,
                                                                       multi_index::const_mem_fun<element_type,
                                                                                                  uint16_type,
                                                                                                  &element_type::processId> > >,

            // sort by less<int> on processId
            multi_index::ordered_non_unique<multi_index::tag<detail::by_pid>,
                                            multi_index::const_mem_fun<element_type,
                                                                       uint16_type,
                                                                       &element_type::processId> >

            > > elements_type;


    typedef typename elements_type::iterator element_iterator;
    typedef typename elements_type::const_iterator element_const_iterator;

    // marker
    typedef typename elements_type::template index<detail::by_marker>::type marker_elements;
    typedef typename marker_elements::iterator marker_element_iterator;
    typedef typename marker_elements::const_iterator marker_element_const_iterator;

    // marker2
    typedef typename elements_type::template index<detail::by_marker2>::type marker2_elements;
    typedef typename marker2_elements::iterator marker2_element_iterator;
    typedef typename marker2_elements::const_iterator marker2_element_const_iterator;

    // marker3
    typedef typename elements_type::template index<detail::by_marker3>::type marker3_elements;
    typedef typename marker3_elements::iterator marker3_element_iterator;
    typedef typename marker3_elements::const_iterator marker3_element_const_iterator;

    typedef typename elements_type::template index<detail::by_pid>::type pid_elements;
    typedef typename pid_elements::iterator pid_element_iterator;
    typedef typename pid_elements::const_iterator pid_element_const_iterator;


    typedef typename elements_type::template index<detail::by_location>::type location_elements;
    typedef typename location_elements::iterator location_element_iterator;
    typedef typename location_elements::const_iterator location_element_const_iterator;

    typedef std::map<int, size_type> parts_map_type;
    typedef typename parts_map_type::const_iterator parts_const_iterator_type;

    /// \cond disabled
    struct update_element_neighbor_type
    {
        update_element_neighbor_type(uint16_type n, size_type id )
            :
            _M_pos_neigh( n ),
            _M_neigh_id( id )
        {}

        void operator()( element_type& e)
        {
            e.setNeighbor( _M_pos_neigh, _M_neigh_id );
        }

    private:
        uint16_type _M_pos_neigh;
        size_type _M_neigh_id;
    };

    /**
     * @class ElementUpdatePoint
     * @brief update point data structure in element
     *
     * this structure is to be used by \p modify from
     * \c boost::multi_index
     *
     */
    struct ElementUpdatePoint
    {
        /**
         * @param index local index of the point
         * @param pt point to update
         */
        ElementUpdatePoint( uint16_type index, typename element_type::PointType const& pt )
            :
            _M_index( index ),
            _M_pt( pt )
        {}

        /**
         * update the \p _M_index point info in element \p e using
         * \p _M_pt
         */
        void operator()( element_type& e )
        {
            e.setPoint( _M_index, _M_pt );
        }
    private:
        uint16_type _M_index;
        typename element_type::PointType const& _M_pt;

    };
    /// \endcond

    //@}

    /** @name Constructors, destructor
     */
    //@{

    Elements()
        :
        _M_elements()
    {}

    Elements( Elements const & f )
        :
        _M_elements( f._M_elements )
    {}

    virtual ~Elements()
    {}

    //@}

    /** @name Operator overloads
     */
    //@{

    /**
     * copy operator
     */
    Elements& operator=( Elements const& e )
    {
        if ( this != &e )
            {
                _M_elements = e._M_elements;
            }
        return *this;
    }

    //@}

    /** @name Accessors
     */
    //@{

    /**
     * \return the elements container
     */
    elements_type const& elements() const { return _M_elements; }

    /**
     * \return the elements container
     */
    elements_type &      elements()       { return _M_elements; }

    /**
     * \return \p true if the container is empty, \p false otherwise
     */
    virtual bool isEmpty() const { return _M_elements.empty(); }

    bool isBoundaryElement( element_type const & e ) const { return _M_elements.find( e )->isOnBoundary(); }
    bool isBoundaryElement( size_type const & id ) const { return _M_elements.find( element_type( id ) )->isOnBoundary(); }

    element_iterator elementIterator( size_type i ) const
        { return  _M_elements.template get<0>().find( boost::make_tuple( M_comm.rank(), i ) ); };

    element_iterator elementIterator( size_type i, size_type p ) const
        { return  _M_elements.template get<0>().find( boost::make_tuple( p, i ) ); };

    element_type const& element( size_type i ) const
        { return *_M_elements.template get<0>().find( boost::make_tuple( M_comm.rank(), i ) );  };

    element_type const& element( size_type i, size_type p ) const
        { return *_M_elements.template get<0>().find( boost::make_tuple( p, i ) );  };

    element_iterator beginElement() { return _M_elements.begin(); }
    element_const_iterator beginElement() const { return _M_elements.begin(); }
    element_iterator endElement() { return _M_elements.end(); }
    element_const_iterator endElement() const { return _M_elements.end(); }


    parts_const_iterator_type beginParts() const { return _M_parts.begin(); }
    parts_const_iterator_type endParts() const { return _M_parts.end(); }

    /**
     * \return the range of iterator \c (begin,end) over the elements
     * with marker \p m on processor \p p
     */
    std::pair<element_iterator, element_iterator>
    elementsRange()
    { return std::make_pair( _M_elements.begin(), _M_elements.end() ); }

    /**
     * \return the range of iterator \c (begin,end) over the elements
     * with marker \p m on processor \p p
     */
    std::pair<element_const_iterator, element_const_iterator>
    elementsRange() const
    { return std::make_pair( _M_elements.begin(), _M_elements.end() ); }

    element_iterator beginElementWithId( size_type m )
    { return _M_elements.template get<0>().lower_bound(boost::make_tuple( M_comm.rank(), m )); }
    element_const_iterator beginElementWithId( size_type m ) const
    { return _M_elements.template get<0>().lower_bound(boost::make_tuple( M_comm.rank(), m )); }
    element_iterator endElementWithId( size_type m )
    { return _M_elements.template get<0>().upper_bound(boost::make_tuple( M_comm.rank(), m )); }
    element_const_iterator endElementWithId( size_type m ) const
    { return _M_elements.template get<0>().upper_bound(boost::make_tuple( M_comm.rank(), m )); }

    /**
     * \return the iterator \c begin over the elements with \c Marker1 \p m
     */
    marker_element_const_iterator beginElementWithMarker( size_type m ) const
    { return _M_elements.template get<detail::by_marker>().lower_bound(boost::make_tuple( Marker1(m), M_comm.rank() )); }

    /**
     * \return the iterator \c begin over the elements with \c Marker2 \p m
     */
    marker2_element_const_iterator beginElementWithMarker2( size_type m ) const
    { return _M_elements.template get<detail::by_marker2>().lower_bound(boost::make_tuple( Marker2(m), M_comm.rank() )); }

    /**
     * \return the iterator \c begin over the elements with \c Marker3 \p m
     */
    marker3_element_const_iterator beginElementWithMarker3( size_type m ) const
    { return _M_elements.template get<detail::by_marker3>().lower_bound(boost::make_tuple( Marker3(m), M_comm.rank() )); }

    /**
     * \return the iterator \c end over the elements with \c Marker1 \p m
     */
    marker_element_const_iterator endElementWithMarker( size_type m ) const
    { return _M_elements.template get<detail::by_marker>().upper_bound(boost::make_tuple( Marker1(m), M_comm.rank() )); }

    /**
     * \return the iterator \c end over the elements with \c Marker2 \p m
     */
    marker2_element_const_iterator endElementWithMarker2( size_type m ) const
    { return _M_elements.template get<detail::by_marker2>().upper_bound(boost::make_tuple( Marker2(m), M_comm.rank() )); }

    /**
     * \return the iterator \c end over the elements with \c Marker3 \p m
     */
    marker3_element_const_iterator endElementWithMarker3( size_type m ) const
    { return _M_elements.template get<detail::by_marker3>().upper_bound(boost::make_tuple( Marker3(m), M_comm.rank() )); }

    /**
     * \return the range of iterator \c (begin,end) over the elements
     * with \c Marker1 \p m on processor \p p
     */
    std::pair<marker_element_const_iterator, marker_element_const_iterator>
    elementsWithMarker( size_type m, size_type p ) const
    { return _M_elements.template get<detail::by_marker>().equal_range(boost::make_tuple( Marker1(m), M_comm.rank() )); }


    /**
     * \return the range of iterator \c (begin,end) over the elements
     * with \c Marker2 \p m on processor \p p
     */
    std::pair<marker2_element_const_iterator, marker2_element_const_iterator>
    elementsWithMarker2( size_type m, size_type p ) const
    { return _M_elements.template get<detail::by_marker2>().equal_range(boost::make_tuple( Marker2(m), M_comm.rank() )); }

    /**
     * \return the range of iterator \c (begin,end) over the elements
     * with \c Marker3 \p m on processor \p p
     */
    std::pair<marker3_element_const_iterator, marker3_element_const_iterator>
    elementsWithMarker3( size_type m, size_type p ) const
    { return _M_elements.template get<detail::by_marker3>().equal_range(boost::make_tuple( Marker3(m), M_comm.rank() ));}

    element_iterator beginElementWithProcessId( size_type m )
    {
        return _M_elements.template get<0>().lower_bound(boost::make_tuple(m));
    }
    element_const_iterator beginElementWithProcessId( size_type m ) const
    {
        return _M_elements.template get<0>().lower_bound(boost::make_tuple(m));
    }
    element_iterator endElementWithProcessId( size_type m )
    {
        return _M_elements.template get<0>().upper_bound(boost::make_tuple(m));
    }
    element_const_iterator endElementWithProcessId( size_type m ) const
    {
        return _M_elements.template get<0>().upper_bound(boost::make_tuple(m));
    }

    std::pair<element_const_iterator, element_const_iterator>
    elementsWithProcessId( size_type m ) const
    { return _M_elements.template get<0>().equal_range(boost::make_tuple( m )); }

    std::pair<element_iterator, element_iterator>
    elementsWithProcessId( size_type m )
    { return _M_elements.template get<0>().equal_range(boost::make_tuple( m )); }

    /**
     * get the elements container by id
     *
     *
     * @return the element container by id
     */
    typename elements_type::template nth_index<0>::type &
    elementsById()
    {
        return _M_elements.template get<0>();
    }

    /**
     * get the elements container by id
     *
     *
     * @return the element container by id
     */
    typename elements_type::template nth_index<0>::type const&
    elementsById() const
    {
        return _M_elements.template get<0>();
    }

    /**
     * get the elements container using the \c Marker1 view
     *
     *
     * @return the element container using \c Marker1 view
     */
    marker_elements &
    elementsByMarker()
    {
        return _M_elements.template get<detail::by_marker>();
    }

    /**
     * get the elements container using the \c Marker2 view
     *
     *
     * @return the element container using \c Marker2 view
     */
    marker2_elements &
    elementsByMarker2()
    {
        return _M_elements.template get<detail::by_marker2>();
    }

    /**
     * get the elements container using the \c Marker3 view
     *
     *
     * @return the element container using \c Marker3 view
     */
    marker3_elements &
    elementsByMarker3()
    {
        return _M_elements.template get<detail::by_marker3>();
    }

    /**
     * get the elements container using the \c Marker1 view
     *
     *
     * @return the element container using \c Marker1 view
     */
    marker_elements const&
    elementsByMarker() const
    {
        return _M_elements.template get<detail::by_marker>();
    }

    /**
     * get the elements container using the \c Marker2 view
     *
     *
     * @return the element container using \c Marker2 view
     */
    marker2_elements const&
    elementsByMarker2() const
    {
        return _M_elements.template get<detail::by_marker2>();
    }

    /**
     * get the elements container using the \c Marker3 view
     *
     *
     * @return the element container using \c Marker3 view
     */
    marker3_elements const&
    elementsByMarker3() const
    {
        return _M_elements.template get<detail::by_marker3>();
    }

    /**
     * get the elements container using the process id view
     *
     *
     * @return the element container using process id view
     */
    pid_elements &
    elementsByProcessId()
    {
        return _M_elements.template get<detail::by_pid>();
    }

    /**
     * get the elements container using the process id view
     *
     *
     * @return the element container using marker view
     */
    pid_elements const&
    elementsByProcessId() const
    {
        return _M_elements.template get<detail::by_pid>();
    }

    /**
     * \return the range of iterator \c (begin,end) over the boundary
     *  element on processor \p p
     */
    std::pair<location_element_const_iterator, location_element_const_iterator>
    boundaryElements( size_type p  ) const
    { return _M_elements.template get<detail::by_location>().equal_range(boost::make_tuple( ON_BOUNDARY, M_comm.rank() )); }

    /**
     * \return the range of iterator \c (begin,end) over the internal
     *  element on processor \p p
     */
    std::pair<location_element_const_iterator, location_element_const_iterator>
    internalElements( size_type p  ) const
    { return _M_elements.template get<detail::by_location>().equal_range(boost::make_tuple( INTERNAL, M_comm.rank() )); }


    /**
     * get the elements container using the location view
     *
     *
     * @return the element container using location view
     */
    location_elements &
    elementsByLocation()
    {
        return _M_elements.template get<detail::by_location>();
    }

    /**
     * get the elements container using the location view
     *
     *
     * @return the element container using location view
     */
    location_elements const&
    elementsByLocation() const
    {
        return _M_elements.template get<detail::by_location>();
    }

    /**
     * get the begin() iterator on all the internal elements
     *
     * @return the begin() iterator on all the internal elements
     */
    location_element_iterator beginInternalElement()
    {
        return _M_elements.template get<detail::by_location>().lower_bound(INTERNAL);
    }
    /**
     * get the end() iterator on all the internal elements
     *
     * @return the end() iterator on all the internal elements
     */
    location_element_iterator endInternalElement()
    {
        return _M_elements.template get<detail::by_location>().upper_bound(INTERNAL);
    }

    /**
     * get the begin() iterator on all the internal elements
     *
     * @return the begin() iterator on all the internal elements
     */
    location_element_const_iterator beginInternalElement() const
    {
        return _M_elements.template get<detail::by_location>().lower_bound(INTERNAL);
    }

    /**
     * get the end() iterator on all the internal elements
     *
     * @return the end() iterator on all the internal elements
     */
    location_element_const_iterator endInternalElement() const
    {
        return _M_elements.template get<detail::by_location>().upper_bound(INTERNAL);
    }

    /**
     * get the begin() iterator on all the boundary elements
     *
     * @return the begin() iterator on all the boundary elements
     */
    location_element_iterator beginElementOnBoundary()
    {
        return _M_elements.template get<detail::by_location>().lower_bound(ON_BOUNDARY);
    }
    /**
     * get the end() iterator on all the boundary elements
     *
     * @return the end() iterator on all the boundary elements
     */
    location_element_iterator endElementOnBoundary()
    {
        return _M_elements.template get<detail::by_location>().upper_bound(ON_BOUNDARY);
    }

    /**
     * get the begin() iterator on all the boundary elements
     *
     * @return the begin() iterator on all the boundary elements
     */
    location_element_const_iterator beginElementOnBoundary() const
    {
        return _M_elements.template get<detail::by_location>().lower_bound(ON_BOUNDARY);
    }

    /**
     * get the end() iterator on all the boundary elements
     *
     * @return the end() iterator on all the boundary elements
     */
    location_element_const_iterator endElementOnBoundary() const
    {
        return _M_elements.template get<detail::by_location>().upper_bound(ON_BOUNDARY);
    }

    //@}

    /** @name  Mutators
     */
    //@{


    //@}

    /** @name  Methods
     */
    //@{

    /**
     * add a new element in the mesh
     * @param f a new point
     * @return the new point from the list
     */
    element_type const& addElement( element_type& f )
    {
        _M_parts[f.marker().value()]++;
        f.setId( _M_elements.size() );
        return *_M_elements.insert( f ).first;
    }

    /**
     * erase element at position \p position
     *
     * @param position \p position is a valid dereferenceable iterator of the index.
     *
     * @return An iterator pointing to the element immediately
     * following the one that was deleted, or \c end() if no such element
     * exists.
     */
    element_iterator eraseElement( element_iterator position )
    {
        return _M_elements.erase( position );
    }

    template<typename ElementVecType>
    void updateMarker2( ElementVecType const& evec )
    {
        UpdaterMarker2<ElementVecType> updaterMarker2( evec );

        element_iterator it;
        element_iterator en;
        boost::tie( it, en ) = elementsWithProcessId( M_comm.rank() );

        for( ; it != en; ++it )
            _M_elements.modify( it, updaterMarker2 );
    }

    template<typename ElementVecType>
    void updateMarker3( ElementVecType const& evec )
    {
        UpdaterMarker3<ElementVecType> updaterMarker3( evec );

        element_iterator it;
        element_iterator en;
        boost::tie( it, en ) = elementsWithProcessId( M_comm.rank() );

        for( ; it != en; ++it )
            _M_elements.modify( it, updaterMarker3 );
    }
    //@}

private:

    template<typename ElementVecType>
    struct UpdaterMarker2
    {
        UpdaterMarker2( ElementVecType const& evec ) : M_evec(evec) {}
        inline void operator() ( element_type& e ) { e.setMarker2( flag_type( M_evec.localToGlobal( e.id(), size_type(0), 0 ) ) ); }
        ElementVecType const& M_evec;
    };

    template<typename ElementVecType>
    struct UpdaterMarker3
    {
        UpdaterMarker3( ElementVecType const& evec ) : M_evec(evec) {}
        inline void operator() ( element_type& e ) { e.setMarker3( flag_type( M_evec.localToGlobal( e.id(), size_type(0), 0 ) ) ); }
        ElementVecType const& M_evec;
    };

private:
    mpi::communicator M_comm;
    elements_type _M_elements;
    parts_map_type _M_parts;
};
/// \endcond
} // Life
#endif /* __elements_H */

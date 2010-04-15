/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-09-03

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2007 Universit� Joseph Fourier

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
   \file faces.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-09-03
 */
#ifndef __faces_H
#define __faces_H 1


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
/**
 * \class Faces
 * \brief Faces container class
 *
 * @author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
 * @see Elements, Edges, Points
 */
template<typename EntityType, typename ElementType>
class Faces
{
public:


    /** @name Typedefs
     */
    //@{
    typedef typename mpl::if_<mpl::equal_to<mpl::int_<EntityType::nDim>, mpl::int_<0> >,
                              mpl::identity<GeoElement0D<EntityType::nRealDim, SubFaceOf<ElementType> > >,
                              typename mpl::if_<mpl::equal_to<mpl::int_<EntityType::nDim>, mpl::int_<1> >,
                                                mpl::identity<GeoElement1D<EntityType::nRealDim, EntityType,  SubFaceOf<ElementType> > >,
                                                mpl::identity<GeoElement2D<EntityType::nRealDim, EntityType,  SubFaceOf<ElementType> > >
                                               >::type
                             >::type::type face_type;

    typedef multi_index::multi_index_container<
                                               face_type,
                                               multi_index::indexed_by<
                                               // sort by employee::operator<
                                               multi_index::ordered_unique<multi_index::identity<face_type> >,

                                               // sort by less<int> on marker
                                               multi_index::ordered_non_unique<multi_index::tag<detail::by_marker>,
                                                                               multi_index::composite_key<
                                                   face_type,
                                                   multi_index::const_mem_fun<face_type,
                                                                              Marker1 const&,
                                                                              &face_type::marker>,
                                                   multi_index::const_mem_fun<face_type,
                                                                              uint16_type,
                                                                              &face_type::processId>
                                                   > >,
                                               // sort by less<int> on processId
                                               multi_index::ordered_non_unique<multi_index::tag<detail::by_pid>,
                                                                               multi_index::const_mem_fun<face_type,
                                                                                                          uint16_type,
                                                                                                          &face_type::processId> >,


                                              // sort by less<int> on boundary
                                               multi_index::ordered_non_unique<multi_index::tag<detail::by_interprocessdomain>,
                                                                               multi_index::composite_key<
                                                   face_type,
                                                   multi_index::const_mem_fun<face_type,
                                                                              bool,
                                                                              &face_type::isInterProcessDomain>,
                                                   multi_index::const_mem_fun<face_type,
                                                                              uint16_type,
                                                                              &face_type::processId>
                                                   >
                                               >,
                                              // sort by less<int> on boundary
                                               multi_index::ordered_non_unique<multi_index::tag<detail::by_location>,
                                                                               multi_index::composite_key<
                                                   face_type,
                                                   multi_index::const_mem_fun<face_type,
                                                                              bool,
                                                                              &face_type::isOnBoundary>,
                                                   multi_index::const_mem_fun<face_type,
                                                                              uint16_type,
                                                                              &face_type::processId>
                                                   >
                                               > >
                                              > faces_type;


    typedef typename faces_type::iterator face_iterator;
    typedef typename faces_type::const_iterator face_const_iterator;
    typedef typename faces_type::template index<detail::by_marker>::type marker_faces;

    typedef typename marker_faces::iterator marker_face_iterator;
    typedef typename marker_faces::const_iterator marker_face_const_iterator;
    typedef typename faces_type::template index<detail::by_location>::type location_faces;
    typedef typename location_faces::iterator location_face_iterator;
    typedef typename location_faces::const_iterator location_face_const_iterator;

    typedef typename faces_type::template index<detail::by_pid>::type pid_faces;
    typedef typename pid_faces::iterator pid_face_iterator;
    typedef typename pid_faces::const_iterator pid_face_const_iterator;

    typedef typename faces_type::template index<detail::by_interprocessdomain>::type interprocess_faces;
    typedef typename interprocess_faces::iterator interprocess_face_iterator;
    typedef typename interprocess_faces::const_iterator interprocess_face_const_iterator;
    //@}

    /**
     * @class FaceUpdatePoint
     * @brief update point data structure in face
     *
     * this structure is to be used by \p modify from
     * \c boost::multi_index
     *
     */
    struct FaceUpdatePoint
    {
        /**
         * @param index local index of the point
         * @param pt point to update
         */
        FaceUpdatePoint( uint16_type index, typename face_type::point_type const& pt )
            :
            _M_index( index ),
            _M_pt( pt )
        {}

        /**
         * update the \p _M_index point info in element \p e using
         * \p _M_pt
         */
        void operator()( face_type& e )
        {
            //Debug() << "FaceUpdatePoint] �pdate point index " << _M_index << " with "<< _M_pt.id() << "\n";
            e.setPoint( _M_index, _M_pt );
            //Debug() << "FaceUpdatePoint] �pdate point "<< e.point(_M_index).id() << "\n";
        }
    private:
        uint16_type _M_index;
        typename face_type::point_type const& _M_pt;

    };

    /** @name Constructors, destructor
     */
    //@{

    Faces()
        :
        _M_faces()
    {}

    Faces( Faces const & f )
        :
        _M_faces( f._M_faces )
    {}

    virtual ~Faces()
    {}

    //@}

    /** @name Operator overloads
     */
    //@{

    Faces& operator=( Faces const& e )
    {
        if ( this != &e )
        {
            _M_faces = e._M_faces;
        }
        return *this;
    }

    //@}

    /** @name Accessors
     */
    //@{

    /**
     * \return the points container
     */
    faces_type & faces() { return _M_faces; }

    /**
     * \return the faces container
     */
    faces_type const& faces() const { return _M_faces; }


    virtual bool isEmpty() const { return _M_faces.empty(); }
    bool isBoundaryFace( face_type const & e ) const { return _M_faces.find( e )->isOnBoundary(); }
    bool isBoundaryFace( size_type const & id ) const { return _M_faces.find( face_type( id ) )->isOnBoundary(); }

    face_type const& face( size_type i ) const { return *_M_faces.find( face_type( i ) ); };

    face_iterator beginFace() { return _M_faces.begin(); }
    face_const_iterator beginFace() const { return _M_faces.begin(); }
    face_iterator endFace() { return _M_faces.end(); }
    face_const_iterator endFace() const { return _M_faces.end(); }


    marker_face_iterator beginFaceWithMarker( size_type m ) { return _M_faces.template get<detail::by_marker>().lower_bound(Marker1(m)); }
    marker_face_const_iterator beginFaceWithMarker( size_type m ) const { return _M_faces.template get<detail::by_marker>().lower_bound(Marker1(m)); }
    marker_face_iterator endFaceWithMarker( size_type m ) { return _M_faces.template get<detail::by_marker>().upper_bound(Marker1(m)); }
    marker_face_const_iterator endFaceWithMarker( size_type m ) const { return _M_faces.template get<detail::by_marker>().upper_bound(Marker1(m)); }

    face_iterator beginFaceWithId( size_type m ) { return _M_faces.lower_bound( face_type(m) ); }
    face_const_iterator beginFaceWithId( size_type m ) const { return _M_faces.lower_bound( face_type(m) ); }
    face_iterator endFaceWithId( size_type m ) { return _M_faces.upper_bound( face_type(m) ); }
    face_const_iterator endFaceWithId( size_type m ) const { return _M_faces.upper_bound( face_type(m)); }

    /**
     * \return the range of iterator \c (begin,end) over the faces
     * with marker \p m on processor \p p
     */
    std::pair<marker_face_iterator, marker_face_iterator>
    facesWithMarker( size_type m, size_type p ) const
    { return _M_faces.template get<detail::by_marker>().equal_range(boost::make_tuple( Marker1(m), p )); }

    /**
     * \return the range of iterator \c (begin,end) over the faces
     * with marker \p m on processor \p p
     */
    std::pair<location_face_iterator, location_face_iterator>
    facesOnBoundary() const
    { return _M_faces.template get<detail::by_location>().equal_range(boost::make_tuple( ON_BOUNDARY )); }

    /**
     * \return the range of iterator \c (begin,end) over the boundary
     *  faces on processor \p p
     */
    std::pair<location_face_iterator, location_face_iterator>
    facesOnBoundary( size_type p  ) const
    { return _M_faces.template get<detail::by_location>().equal_range(boost::make_tuple( ON_BOUNDARY, p )); }

    /**
     * \return the range of iterator \c (begin,end) over the internal faces
     * on processor \p p
     */
    std::pair<location_face_iterator, location_face_iterator>
    internalFaces() const
    { return _M_faces.template get<detail::by_location>().equal_range(boost::make_tuple( INTERNAL, M_comm.rank() )); }

    /**
     * \return the range of iterator \c (begin,end) over the inter-process domain faces
     * on processor \p p
     */
    std::pair<interprocess_face_iterator, interprocess_face_iterator>
    interProcessFaces() const
    { return _M_faces.template get<detail::by_interprocessdomain>().equal_range(boost::make_tuple( true, M_comm.rank() )); }

#if 0
    /**
     * \return the range of iterator \c (begin,end) over the intra-process domain faces
     * on processor \p p
     */
    std::pair<interprocess_face_iterator, interprocess_face_iterator>
    intraProcessFaces() const
    { return _M_faces.template get<detail::by_interprocessdomain>().equal_range(boost::make_tuple( false, M_comm.rank() )); }
#endif

    /**
     * \return the range of iterator \c (begin,end) over the elements
     * on processor \p p
     */
    std::pair<pid_face_iterator, pid_face_iterator>
    facesWithProcessId( ) const
        { return _M_faces.template get<detail::by_pid>().equal_range( M_comm.rank() ); }

    /**
     * get the faces container by id
     *
     *
     * @return the face container by id
     */
    typename faces_type::template nth_index<0>::type &
    facesById()
        {
            return _M_faces.template get<0>();
        }

    /**
     * get the faces container by id
     *
     *
     * @return the face container by id
     */
    typename faces_type::template nth_index<0>::type const&
    facesById() const
        {
            return _M_faces.template get<0>();
        }

    /**
     * get the faces container using the marker view
     *
     *
     * @return the face container using marker view
     */
    marker_faces &
    facesByMarker()
        {
            return _M_faces.template get<detail::by_marker>();
        }

    /**
     * get the faces container using the marker view
     *
     *
     * @return the face container using marker view
     */
    marker_faces const&
    facesByMarker() const
        {
            return _M_faces.template get<detail::by_marker>();
        }
    /**
     * get the faces container using the location view
     *
     *
     * @return the face container using location view
     */
    location_faces &
    facesByLocation()
        {
            return _M_faces.template get<detail::by_location>();
        }

    /**
     * get the faces container using the location view
     *
     *
     * @return the face container using location view
     */
    location_faces const&
    facesByLocation() const
        {
            return _M_faces.template get<detail::by_location>();
        }

    /**
     * get the begin() iterator on all the internal faces
     *
     * @return the begin() iterator on all the internal faces
     */
    location_face_iterator beginInternalFace()
        {
            return _M_faces.template get<detail::by_location>().lower_bound( boost::make_tuple( INTERNAL, M_comm.rank() ));
        }
    /**
     * get the end() iterator on all the internal faces
     *
     * @return the end() iterator on all the internal faces
     */
    location_face_iterator endInternalFace()
        {
            return _M_faces.template get<detail::by_location>().upper_bound( boost::make_tuple( INTERNAL, M_comm.rank() ));
        }

    /**
     * get the begin() iterator on all the internal faces
     *
     * @return the begin() iterator on all the internal faces
     */
    location_face_const_iterator beginInternalFace() const
        {
            return _M_faces.template get<detail::by_location>().lower_bound( boost::make_tuple( INTERNAL, M_comm.rank() ));
        }

    /**
     * get the end() iterator on all the internal faces
     *
     * @return the end() iterator on all the internal faces
     */
    location_face_const_iterator endInternalFace() const
        {
            return _M_faces.template get<detail::by_location>().upper_bound( boost::make_tuple( INTERNAL, M_comm.rank() ));
        }

    /**
     * get the begin() iterator on all the boundary faces
     *
     * @return the begin() iterator on all the boundary faces
     */
    location_face_iterator beginFaceOnBoundary()
        {
            return _M_faces.template get<detail::by_location>().lower_bound(boost::make_tuple( ON_BOUNDARY, M_comm.rank() ) );
        }
    /**
     * get the end() iterator on all the boundary faces
     *
     * @return the end() iterator on all the boundary faces
     */
    location_face_iterator endFaceOnBoundary()
        {
            return _M_faces.template get<detail::by_location>().upper_bound(boost::make_tuple( ON_BOUNDARY, M_comm.rank() ) );
        }

    /**
     * get the begin() iterator on all the boundary faces
     *
     * @return the begin() iterator on all the boundary faces
     */
    location_face_const_iterator beginFaceOnBoundary() const
        {
            return _M_faces.template get<detail::by_location>().lower_bound(boost::make_tuple( ON_BOUNDARY, M_comm.rank() ) );
        }

    /**
     * get the end() iterator on all the boundary faces
     *
     * @return the end() iterator on all the boundary faces
     */
    location_face_const_iterator endFaceOnBoundary() const
        {
            return _M_faces.template get<detail::by_location>().upper_bound(boost::make_tuple(ON_BOUNDARY, M_comm.rank() ) );
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
     * add a new face in the mesh
     * @param f a new point
     * @return the new point from the list
     */
    std::pair<face_iterator,bool> addFace( face_type& f )
    {
        std::pair<face_iterator,bool> ret =  _M_faces.insert( f );
        LIFE_ASSERT( ret.second )( ret.second )( ret.first->id() )( f.id() ).warn("face not added to container");
        return ret;
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
    face_iterator eraseFace( face_iterator position )
    {
        return _M_faces.erase( position );
    }

    //@}

private:
    mpi::communicator M_comm;
    faces_type _M_faces;
};
/// \endcond
} // Life
#endif /* __faces_H */

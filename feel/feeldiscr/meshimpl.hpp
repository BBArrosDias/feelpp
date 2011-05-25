/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=tcl:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2007-08-07

  Copyright (C) 2007-2010 Université Joseph Fourier (Grenoble I)

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
   \file meshimpl.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2007-08-07
 */
#ifndef __MESHIMPL_HPP
#define __MESHIMPL_HPP 1

#include <boost/preprocessor/comparison/greater_equal.hpp>
#include <feel/feelmesh/geoentity.hpp>
#include <feel/feelmesh/simplex.hpp>
#include <feel/feelmesh/hypercube.hpp>

#include <feel/feeldiscr/mesh.hpp>
#include <feel/feeldiscr/partitioner.hpp>

namespace Feel
{
template<typename Shape, typename T>
Mesh<Shape, T>::Mesh( std::string partitioner )
    :
    super(),
    _M_gm( new gm_type ),
    _M_gm1( new gm1_type ),
    M_meas( 0 ),
    M_measbdy( 0 ),
    M_part(),
    M_tool_localization(new Localization())
{
    Debug( 4015 ) << "[Mesh::Mesh] setting partitioner to " << partitioner << "\n";
    this->setPartitioner( partitioner );
    Debug( 4015 ) << "[Mesh::Mesh] setting partitioner to " << partitioner << " done\n";
}
template<typename Shape, typename T>
void
Mesh<Shape, T>::partition ( const uint16_type n_parts )
{
    M_part->partition( *this, n_parts );
}

template<typename Shape, typename T>
void
Mesh<Shape, T>::updateForUse()
{
    Debug( 4015 ) << "component     MESH_RENUMBER: " <<  this->components().test( MESH_RENUMBER ) << "\n";
    Debug( 4015 ) << "component MESH_UPDATE_EDGES: " <<  this->components().test( MESH_UPDATE_EDGES ) << "\n";
    Debug( 4015 ) << "component MESH_UPDATE_FACES: " <<  this->components().test( MESH_UPDATE_FACES ) << "\n";
    Debug( 4015 ) << "component    MESH_PARTITION: " <<  this->components().test( MESH_PARTITION ) << "\n";

    if ( this->numElements() == 0)
        return;

    boost::timer ti;
    if ( !this->isUpdatedForUse() )
        {
            if ( this->components().test( MESH_RENUMBER ) )
                {

                    this->renumber();
                    Debug( 4015 ) << "[Mesh::updateForUse] renumber : " << ti.elapsed() << "\n";
                }
            //
            // compute the Adjacency graph
            //
            ti.restart();
            Debug( 4015 ) << "Compute adjacency graph\n";
            element_iterator iv,  en;
            std::map<size_type,boost::tuple<size_type, uint16_type, size_type> > f2e;

            _M_e2f.resize( boost::extents[this->numElements()][this->numLocalFaces()] );
            std::map<std::set<int>, size_type > _faces;
            typename std::map<std::set<int>, size_type >::iterator _faceit;
            int next_face = 0;

            boost::tie( iv, en ) = this->elementsRange();
            for ( ;iv != en; ++iv )
                {
                    element_type const& __element = *iv;

                    //MakeBareEntity<element_type,nDim> baremaker( __element );
                    for ( size_type j = 0; j < this->numLocalFaces(); j++ )
                    {
                        std::set<int> s;
                        for( int f = 0; f < face_type::numVertices; ++f )
                        {
                            if ( nDim == 1 )
                                s.insert( iv->point( j ).id() );
                            else
                                s.insert( iv->point( iv->fToP( j, f ) ).id() );
                        }
                        bool faceinserted = false;
                        boost::tie( _faceit, faceinserted ) = _faces.insert( std::make_pair( s, next_face ) );
                        if ( faceinserted )
                            ++next_face;

#if !defined ( NDEBUG )
                        Debug( 4015 ) << "------------------------------------------------------------\n";
                        Debug( 4015 ) << "Element id: " << iv->id() << " local face id: " << j << "\n";
#endif
                        //e = _be.addIfNotThere( baremaker( j ) );


                        if ( faceinserted )
                        {
#if !defined ( NDEBUG )
                            Debug( 4015 ) << " new face " << _faceit->second << " is now in store with elt " << f2e[_faceit->second].template get<0>()  << " and local face id " <<  f2e[_faceit->second].template get<1>()  << "\n";
#endif

                            f2e[_faceit->second].template get<0>() = iv->id();
                            f2e[_faceit->second].template get<1>() = j;
                            _M_e2f[iv->id()][j]=boost::make_tuple( _faceit->second, invalid_size_type_value );
                        }
                        else // already stored
                        {
#if !defined ( NDEBUG )
                            Debug( 4015 ) << "old face " << _faceit->second << " was already in store with elt " << f2e[_faceit->second].template get<0>() << " and local face id " <<  f2e[_faceit->second].template get<1>() << "\n";
#endif

                            f2e[_faceit->second].template get<2>() = iv->id();
                            _M_e2f[iv->id()][j]=boost::make_tuple( _faceit->second, f2e[_faceit->second].template get<0>() );
                            _M_e2f[f2e[_faceit->second].template get<0>()] [f2e[_faceit->second].template get<1>()] =
                                boost::make_tuple( _faceit->second, iv->id() );
                        }


                    } // local face
                } // element loop
            Debug( 4015 ) << "Compute adjacency graph done in " << ti.elapsed() << "\n";
            // partition mesh
            if ( this->components().test( MESH_PARTITION ) || ( M_comm.size() > 1 ) )
                {
                    boost::timer ti1;
                    this->partition();
                    Debug( 4015 ) << "[Mesh::updateForUse] partition time : " << ti1.elapsed() << "\n";
                }

            if ( this->components().test( MESH_UPDATE_FACES ) )
                {
                    ti.restart();
                    // update connectivities of entities of co dimension 1
                    this->updateEntitiesCoDimensionOne();
                    // update permutation of entities of co-dimension 1
                    this->updateEntitiesCoDimensionOnePermutation();
                    Debug( 4015 ) << "[Mesh::updateForUse] update entities of codimension 1 : " << ti.elapsed() << "\n";
                }
            if ( this->components().test( MESH_UPDATE_EDGES ) )
                {
                    ti.restart();
                    // update connectivities of entities of co dimension 2
                    // (edges in 3D)
                    this->updateEntitiesCoDimensionTwo();
                    Debug( 4015 ) << "[Mesh::updateForUse] update edges : " << ti.elapsed() << "\n";
                }

            {
                element_iterator iv,  en;
                boost::tie( iv, en ) = this->elementsRange();
                for ( ;iv != en; ++iv )
                {
                    this->elements().modify( iv, typename super_elements::ElementConnectPointToElement() );
                }
                boost::tie( iv, en ) = this->elementsRange();
                auto pc = _M_gm->preCompute( _M_gm, _M_gm->referenceConvex().vertices() );
                auto pcf =  _M_gm->preComputeOnFaces( _M_gm, _M_gm->referenceConvex().barycenterFaces() );
                M_meas = 0;
                M_measbdy = 0;
                for ( ;iv != en; ++iv )
                    {
                        this->elements().modify( iv,
                                                 lambda::bind( &element_type::setMeshAndGm,
                                                               lambda::_1,
                                                               this, _M_gm, _M_gm1 ) );

                        this->elements().modify( iv,
                                                 lambda::bind( &element_type::updateWithPc,
                                                               lambda::_1, pc, boost::ref(pcf) ) );
                        M_meas += iv->measure();
                        auto _faces = iv->faces();
                        for( ; _faces.first != _faces.second; ++_faces.first )
                            if ( (*_faces.first) && (*_faces.first)->isOnBoundary() )
                                M_measbdy += (*_faces.first)->measure();
                    }
                // now that all elements have been updated, build inter element
                // data such as the measure of point element neighbors
                boost::tie( iv, en ) = this->elementsRange();
                for ( ;iv != en; ++iv )
                {
                    value_type meas = 0;
                    BOOST_FOREACH( auto _elt, iv->pointElementNeighborIds() )
                    {
                        if ( this->hasElement( _elt ) )
                            meas += this->element(_elt).measure();
                    }
                    this->elements().modify( iv,
                                             lambda::bind( &element_type::setMeasurePointElementNeighbors,
                                                           lambda::_1, meas ) );
                }
                typedef typename super::face_const_iterator face_const_iterator;
                face_iterator itf = this->beginFace();
                face_iterator ite = this->endFace();
                for( ; itf != ite; ++ itf )
                    {
                        this->faces().modify( itf,
                                              lambda::bind( &face_type::setMesh,
                                                            lambda::_1,
                                                            this ) );
                    }
            }

            // check mesh connectivity
            this->check();
            this->setUpdatedForUse( true );

            _M_gm->initCache( this );
            _M_gm1->initCache( this );

            M_tool_localization->setMesh( this->shared_from_this(),false );

        }
    Debug( 4015 ) << "[Mesh::updateForUse] total time : " << ti.elapsed() << "\n";
}



template<typename Shape, typename T>
void
Mesh<Shape, T>::renumber(mpl::bool_<true>)
{
    size_type next_free_node = 0;

    // map old/new ids
    std::vector<size_type> node_map( this->numPoints(), invalid_size_type_value );
    //std::map<size_type> node_map( this->numPoints(), invalid_size_type_value );
    //std::vector<point_type> pt_map( this->numPoints() );
    typedef std::map<size_type,point_type> ptmap_type;
    ptmap_type pt_map;

    typedef typename ptmap_type::iterator ptmap_iterator;
    typedef std::vector<size_type>::iterator nm_iterator;
    // first collect all point id that will be swapped in a dictionary
    for ( element_const_iterator elt = this->beginElement();
          elt != this->endElement(); ++elt )
        {
            element_type const& __element = *elt;
#if !defined( NDEBUG )
            Debug( 4015 ) << "mesh::renumber] element id " << __element.id() <<  " proc " << __element.processId() << "\n";
            for(int i = 0; i < __element.nPoints(); ++i )
                {
                    Debug( 4015 ) << "point id = " << __element.point( i ).id() << "\n";
                }
#endif
            // renumber the nodes of the element
            for(int i = 0; i < __element.nPoints(); ++i )
                {
                    point_type  __pt = __element.point( i );
                    size_type __true_id = __pt.id();
                    size_type __id = __true_id;
                    // did we already change the id? if yes get the
                    //  id it was changed to
                    if ( node_map[ __true_id ] != invalid_size_type_value )
                        {
                            __id = node_map[__true_id];
#if !defined( NDEBUG )
                            if ( __id >= next_free_node )
                                Debug( 4015 ) << "next_free_node = " << next_free_node
                                              << " point id = " << __id << "\n";
#endif
                        }

                    // don't renumber if already done
                    if ( __id == next_free_node )
                        {
                            node_map[ __true_id ] = next_free_node;
                            pt_map[next_free_node] = __pt;
                            pt_map[next_free_node].setId( next_free_node );
                            ++next_free_node;
                        }
                    else if ( __id > next_free_node )
                        {
                            // first check if next_free_node has been
                            // already taken by another point, if so
                            // then affect this point id to __id
                            nm_iterator nm_it = std::find( node_map.begin(),
                                                           node_map.end(),
                                                           next_free_node );
                            if ( nm_it != node_map.end() )
                                *nm_it = __id;
                            else
                                node_map[ next_free_node ] = __id;

                            // must use \p __true_id here as it is the
                            // original point id we started with
                            node_map[ __true_id ] = next_free_node;
                            pt_map[next_free_node] = __pt;
                            pt_map[next_free_node].setId( next_free_node );

#if !defined( NDEBUG )
                            Debug( 4015 ) << "next_free_node = " << next_free_node
                                          << " swapping point " << __true_id
                                          << " and point " << next_free_node << "\n";
#endif

                            // next free node
                            ++next_free_node;
                        } // if
                } // for
            //__element.setId(
        }
    Debug( 4015 ) << "[mesh::renumber] done collecting ids\n";
#if !defined(NDEBUG)
    std::vector<size_type> check_id( node_map );
    std::unique(check_id.begin(), check_id.end());

    FEEL_ASSERT( check_id.size() == node_map.size() )( node_map.size() )( check_id.size() ).error( "all ids must be unique");
    FEEL_ASSERT ( std::find( node_map.begin(), node_map.end(), invalid_size_type_value ) == node_map.end() ).warn("invalid size_type value found as id ");

#endif /* NDEBUG */

    // now we can replace the points id
    //for( size_type p = 0; p < pt_map.size(); ++p )
    ptmap_iterator ptmapit = pt_map.begin();
    ptmap_iterator ptmapen = pt_map.end();
    for( ; ptmapit != ptmapen; ++ptmapit )
        {
            point_iterator ptit = this->points().find( ptmapit->first );
#if !defined( NDEBUG )
            Debug( 4015 ) << "[mesh::replace] replacing point " << ptit->id()
                          <<  " with " << ptmapit->second.id() << "\n";
#endif
            bool __rep1 = this->points().replace( ptit, ptmapit->second );
            FEEL_ASSERT( __rep1 )( __rep1 )( ptit->id() )( ptmapit->second.id() ) .warn( "invalid point replacement");

        }
    Debug( 4015 ) << "[mesh::renumber] done replace point ids\n";
    for ( element_iterator elt = this->beginElement();
          elt != this->endElement(); ++elt )
        {
            element_type __element = *elt;
#if !defined( NDEBUG )
            Debug( 4015 ) << "mesh::renumber] element id " << __element.id() <<  " proc " << __element.processId() << "\n";
#endif
            // renumber the nodes of the element
            for(int i = 0; i < __element.nPoints(); ++i )
                {

                    size_type __true_id =__element.point( i ).id();
                    this->elements().modify( elt,
                                             typename super_elements::ElementUpdatePoint( i, this->point( node_map[__true_id] ) ) );
#if !defined( NDEBUG )
                    Debug( 4015 ) << "point id = " << __true_id << " node map " <<  node_map[__true_id] << " "
                                  << "new point id = " << elt->point( i ).id() << "\n";
#endif
                }
        }
    Debug( 4015 ) << "[mesh::renumber] done replace point ids in elements\n";
    for ( face_iterator elt = this->beginFace();
          elt != this->endFace(); ++elt )
        {
            face_type __face = *elt;
#if !defined( NDEBUG )
            Debug( 4015 ) << "face id: " << __face.id()
                          << " marker: " << __face.marker() << "\n";
#endif
            // renumber the nodes of the face
            for(int i = 0; i < __face.nPoints(); ++i )
                {
                    size_type __true_id =__face.point( i ).id();
                    this->faces().modify( elt,
                                          lambda::bind( &face_type::setPoint,
                                                        lambda::_1,
                                                        lambda::constant( i ),
                                                        boost::cref( this->point( node_map[__true_id] ) ) ) );
                    face_type __face2 = *elt;
#if !defined( NDEBUG )
                    Debug( 4015 ) << "id1= " << __face2.point(0).id() << " id2= " << __face2.point(1).id()<< "\n";
                    Debug( 4015 ) << "point lid = " << i << " id = " << __true_id
                                  << " nid = " << this->point( node_map[__true_id] ).id()
                                  << " new point id = " << elt->point( i ).id() << "\n";
#endif

                }
        }

    if( Shape == SHAPE_TETRA && nOrder==1 )
        {
            localrenumber();

#if !defined(NDEBUG)
            checkLocalPermutation(mpl::bool_< (Shape == SHAPE_TETRA) >());
#endif
        }

    // should we renumber also the faces and elements ?


}


template<typename Shape, typename T>
void
Mesh<Shape, T>::localrenumber()
{
    for ( element_const_iterator elt = this->beginElement();
          elt != this->endElement(); ++elt )
        {
            element_type __element = *elt;

            int16_type n_swap = 0;

            for(uint16_type i=Shape::numVertices-1; i > 1; --i)
                {
                    uint16_type min_index = 0;

                    for(uint16_type j=1; j < __element.nPoints()+i-(Shape::numVertices-1); ++j)
                        {
                            if( __element.point(j).id() < __element.point(min_index).id() )
                                min_index = j;
                        }

                    if( i != min_index )
                        {
                            __element.swapPoints(i, min_index);
                            n_swap += int16_type(1);
                        }
                }

            // Adds consistent permutation anti-clockwise for the two last points
            if( n_swap == 1 )
                __element.swapPoints(0,1);

            // Modify the points ids
            if( n_swap > 0 )
                {
                    for(uint16_type i=0; i < __element.nPoints(); ++i)
                        {
                            this->elements().modify( elt, typename super_elements::ElementUpdatePoint( i , this->point( __element.point(i).id() ) ) );
                        }
                }
        }
} /** void LocalRenumber **/

template<typename Shape, typename T>
void
Mesh<Shape, T>::updateEntitiesCoDimensionOne()
{
    boost::timer ti;
    face_type face;

    std::map<std::set<int>, size_type > _faces;
    typename std::map<std::set<int>, size_type >::iterator _faceit;
    int next_face = 0;
    element_type ele;

    // First We check if we have already Faces stored
    if ( ! this->faces().empty() )
        {
            // dump all faces in the container, to maintain the correct numbering
            // if everything is correct the numbering in the bareface structure
            // will reflect the actual face numbering However, if I want to create
            // the internal faces I need to make sure that I am processing only the
            // boundary ones. So I resize the container!
            //if ( cf ) this->faces().resize( _numBFaces );


            face_iterator __it = this->beginFace();
            face_iterator __en = this->endFace();
            for ( ;__it!=__en; )
                {
                    std::set<int> s;
                    for( int f = 0; f < face_type::numVertices; ++f )
                    {
                        s.insert( __it->point( f ).id() );
                    }
                    bool faceinserted = false;
                    boost::tie( _faceit, faceinserted ) = _faces.insert( std::make_pair( s, next_face ) );
                    if ( faceinserted )
                        ++next_face;

#if !defined( NDEBUG )
                    if ( faceinserted  )
                        Debug( 4015 ) << "added face with id " << __it->id () << "\n";
                    else
                        Debug( 4015 ) << "not added face with id " << __it->id ()
                                      << " was already face with id = " << _faceit->second << "\n";
                    FEEL_ASSERT( faceinserted )
                        (_faceit->second )
                        ( __it->id() ).warn( "duplicated face" );
#endif

                    if ( faceinserted == false )
                        {
                            // here we get the next face or \c end()
                            size_type theid = __it->id();
                            __it = this->eraseFace( __it );
                            face_iterator __other = this->faces().find( face_type( _faceit->second ) );
                            FEEL_ASSERT( __other->id() != theid )
                                ( __other->id() )
                                ( theid ).error( "faces should have different ids " );


                        }
                    else
                        {
                            // ensure that item handler ids are in sync with
                            // faces ids
                            face_type __f = *__it;
                            __f.setId( _faceit->second );
#if !defined( NDEBUG )
                            Debug( 4015 ) << "set face id " << __f.id()
                                          << " iterator id = " << __it->id()
                                          << " check id = " << _faceit->second << "\n";
#endif
                            this->faces().replace( __it, __f );
                            ++__it;
                        }
                }
        }
    Debug( 4015 ) << "[Mesh::updateFaces] adding faces : " << ti.elapsed() << "\n";
    ti.restart();

    Debug( 4015 ) << "[Mesh::updateFaces] numLocalFaces : " << this->numLocalFaces() << "\n";
    Debug( 4015 ) << "[Mesh::updateFaces] face_type::numVertices : " << face_type::numVertices << "\n";
    element_iterator iv,  en;
    boost::tie( iv, en ) = this->elementsRange();
    for ( ;iv != en; ++iv )
        {
            element_type const& __element = *iv;
            size_type __element_id = __element.id();

            //MakeBareEntity<element_type,nDim> baremaker( __element );
            for ( size_type j = 0; j < this->numLocalFaces(); j++ )
                {
                    std::set<int> s;
                    for( int f = 0; f < face_type::numVertices; ++f )
                    {
                        uint16_type pt_localid = (nDim==1)?j:iv->fToP( j, f );
                        s.insert( iv->point( pt_localid ).id() );
                        Debug( 4015 ) << "add point local id " << f << " to face " << j  << " " << iv->fToP( j, f )
                                      << " global id " << iv->point( pt_localid ).id() << "\n";
                    }

                    bool faceinserted = false;
                    boost::tie( _faceit, faceinserted ) = _faces.insert( std::make_pair( s, next_face ) );
                    if ( faceinserted )
                        ++next_face;
#if !defined( NDEBUG )
                    Debug( 4015 ) << "------------------------------------------------------------\n";
                    Debug( 4015 ) << "Element id: " << iv->id() << " local face id: " << j << "\n";
#endif

                    if ( faceinserted )
                        {
#if !defined( NDEBUG )
                            Debug( 4015 ) << "creating the face:" << _faceit->second << "\n";
#endif
                            // set face id
                            face.setId( _faceit->second );
                            face.disconnect();

                            // set the process id from element
                            face.setProcessId( __element.processId() );

                            // set the vertices of the face
                            for ( size_type k = 0;k < face_type::numPoints;++k )
                                face.setPoint( k, iv->point( ele.fToP( j, k ) ) );

                            // set the connection with the element
                            face.setConnection0( boost::make_tuple( boost::addressof( __element ), __element_id, j, __element.processId() ) );
                            face.setOnBoundary( true );

                            // adding the face
                            bool inserted = false;
                            face_iterator __fit;
                            boost::tie( __fit, inserted)  = this->addFace( face );
                            FEEL_ASSERT( inserted && __fit != this->endFace() )
                                ( _faceit->second )
                                ( iv->id() )
                                ( __fit->id() )
                                ( face.id() ).error( "invalid face iterator" );
                            this->elements().modify( iv, detail::UpdateFace<face_type>( boost::cref(*__fit) ) );

#if !defined( NDEBUG )
                            Debug( 4015 ) << "Adding [new] face info : \n";
                            Debug( 4015 ) << "element id: " << __element_id << "\n";
                            Debug( 4015 ) << "id: " << __fit->id() << "\n";
                            Debug( 4015 ) << "bdy: " << __fit->isOnBoundary() << "\n";
                            Debug( 4015 ) << "marker: " << __fit->marker() << "\n";
                            Debug( 4015 ) << "ad_first: " << __fit->ad_first() << "\n";
                            Debug( 4015 ) << "pos_first: " << __fit->pos_first() << "\n";
                            Debug( 4015 ) << "proc_first: " << __fit->proc_first() << "\n";
                            Debug( 4015 ) << "ad_second: " << __fit->ad_second() << "\n";
                            Debug( 4015 ) << "pos_second: " << __fit->pos_second() << "\n";
                            Debug( 4015 ) << "proc_second: " << __fit->proc_second() << "\n";
#endif
                        }
                    else
                        {
#if !defined( NDEBUG )
                            Debug( 4015 ) << "found the face:" << _faceit->second << " in element " << __element_id << " and local face: " << j << "\n";
#endif

                            // look in the face table for the face
                            face_iterator __fit = this->faces().find( face_type( _faceit->second ) );
                            FEEL_ASSERT( __fit != this->endFace() )( _faceit->second ).error( "face is not in face container" );


                            face_type face = *__fit;

                            // the face could have been entered apriori given by
                            // the mesh generator, so just set the connection0
                            // properly .
                            if ( !__fit->isConnectedTo0() )
                                {
#if !defined( NDEBUG )
                                    Debug( 4015 ) << "[updateFaces][boundary] element: " << __element_id
                                                  << " face: " << j << " id: " << _faceit->second << "\n";
#endif

                                    // set the connection with the element
                                    face.setConnection0( boost::make_tuple( boost::addressof( __element ), __element_id, j, __element.processId() ) );
                                    // set the process id from element
                                    face.setProcessId( __element.processId() );

                                    //this->faces().modify( __fit,
                                    //detail::UpdateFaceConnection0<typename face_type::element_connectivity_type>( boost::make_tuple( boost::addressof( __element ), __element_id, j, __element.processId() ) ) );

                                    this->faces().replace( __fit, face );

                                    this->elements().modify( iv, detail::UpdateFace<face_type>( boost::cref(*__fit) ) );
#if !defined( NDEBUG )
                                    Debug( 4015 ) << "adding [!isConnectedTo0] face info : \n";
                                    Debug( 4015 ) << "id: " << __fit->id() << "\n";
                                    Debug( 4015 ) << "bdy: " << __fit->isOnBoundary() << "\n";
                                    Debug( 4015 ) << "marker: " << __fit->marker() << "\n";
                                    Debug( 4015 ) << "ad_first: " << __fit->ad_first() << "\n";
                                    Debug( 4015 ) << "pos_first: " << __fit->pos_first() << "\n";
                                    Debug( 4015 ) << "proc_first: " << __fit->proc_first() << "\n";
                                    Debug( 4015 ) << "ad_second: " << __fit->ad_second() << "\n";
                                    Debug( 4015 ) << "pos_second: " << __fit->pos_second() << "\n";
                                    Debug( 4015 ) << "proc_second: " << __fit->proc_second() << "\n";
#endif
                                }
                            // we found an internal face
                            else
                                {


#if 0
                                    this->faces().modify( __fit,
                                                          detail::UpdateFaceConnection1<typename face_type::element_connectivity_type>( boost::make_tuple( boost::addressof( __element ), __element_id, j, __element.processId() ) ) );


                                    FEEL_ASSERT( __fit->isConnectedTo0() && __fit->isConnectedTo1() )
                                        ( __fit->isConnectedTo0() )( __fit->isConnectedTo1() ).error( "invalid face connection" );
#endif
                                    detail::UpdateFaceConnection1<typename face_type::element_connectivity_type> update1( boost::make_tuple( boost::addressof( __element ), __element_id, j, __element.processId() ) );
                                    update1( face );
                                    face.setOnBoundary( false );
                                    this->faces().replace( __fit, face );
#if 0
                                    // update neighbors for each element and replace in element container
                                    element_iterator elt1 = this->elementIterator( __fit->ad_first(), __fit->proc_first() );
                                    this->elements().modify( elt1, update_element_neighbor_type( __fit->pos_first(),
                                                                                                 __fit->ad_second() ) );
                                    this->elements().modify( iv, update_element_neighbor_type( __fit->pos_second(),
                                                                                               __fit->ad_first() ) );
#endif
                                    // be careful: this step is crucial to set proper neighbor
                                    // connectivity
                                    element_iterator elt1 = this->elementIterator( __fit->ad_first(), __fit->proc_first() );
                                    this->elements().modify( elt1, detail::UpdateFace<face_type>( boost::cref(*__fit) ) );
                                    this->elements().modify( iv, detail::UpdateFace<face_type>( boost::cref(*__fit) ) );

#if !defined( NDEBUG )
                                    Debug( 4015 ) << "adding face info : \n";
                                    Debug( 4015 ) << "id: " << __fit->id() << "\n";
                                    Debug( 4015 ) << "bdy: " << __fit->isOnBoundary() << "\n";
                                    Debug( 4015 ) << "marker: " << __fit->marker() << "\n";
                                    Debug( 4015 ) << "ad_first: " << __fit->ad_first() << "\n";
                                    Debug( 4015 ) << "pos_first: " << __fit->pos_first() << "\n";
                                    Debug( 4015 ) << "proc_first: " << __fit->proc_first() << "\n";
                                    Debug( 4015 ) << "ad_second: " << __fit->ad_second() << "\n";
                                    Debug( 4015 ) << "pos_second: " << __fit->pos_second() << "\n";
                                    Debug( 4015 ) << "proc_second: " << __fit->proc_second() << "\n";
#endif

                                }

                            FEEL_ASSERT( __fit->processId() == __fit->proc_first() )
                                ( __fit->processId() )( __fit->proc_first() ).error( "invalid process id" );
                        }

                    FEEL_ASSERT( iv->facePtr( j ) )( j )( iv->id() ).error( "invalid element face error" );
                } // face loop
        } // element loop
#if 0
    face_iterator f_it = this->beginFace();
    face_iterator f_en = this->endFace();
    for ( ;f_it!=f_en; ++f_it)
    {
        // cleanup the face data structure :

        if ( !f_it->isConnectedTo0() )
        {
            // remove all faces that are not connected to any elements
            this->faces().erase( f_it );
        }

    }
#endif
    boost::tie( iv, en ) = this->elementsRange();
    for ( ;iv != en; ++iv )
        {
            bool isOnBoundary = false;
            for ( size_type j = 0; j < this->numLocalFaces(); j++ )
                {
                    isOnBoundary |= iv->face( j ).isOnBoundary();
                }
            // an element on the boundary means that is shares a face
            // with the boundary
            this->elements().modify( iv, detail::OnBoundary( isOnBoundary ) );
        }
    Debug( 4015 ) << "[Mesh::updateFaces] element/face connectivity : " << ti.elapsed() << "\n";
    ti.restart();
}


template<typename Shape, typename T>
void
Mesh<Shape, T>::check() const
{
#if !defined( NDEBUG )
    Debug( 4015 ) << "[Mesh::check] numLocalFaces = " << this->numLocalFaces() << "\n";
    element_iterator iv = this->beginElementWithProcessId( M_comm.rank() );
    element_iterator en = this->endElementWithProcessId( M_comm.rank() );
    //boost::tie( iv, en ) = this->elementsRange();
    for ( ;iv != en; ++iv )
        {
            element_type const& __element = *iv;
            for ( size_type j = 0; j < this->numLocalFaces(); j++ )
                {
                    FEEL_ASSERT( iv->facePtr( j ) )( j )( iv->id() ).error( "invalid element face check" );
                    Debug( 4015 ) << "------------------------------------------------------------\n";
                    Debug( 4015 ) << "Element : " << iv->id() << " face lid: " << j << " face gid:  " << iv->face( j ).id() << "\n";

                }

            size_type counter = 0;
            for (uint16_type ms=0; ms < __element.nNeighbors(); ms++)
                {
                    if ( __element.neighbor(ms).first != invalid_size_type_value )
                        ++counter;

                }
            Debug( 4015 ) << "[Mesh::check] element " << __element.id() << " number of neighbors: " << counter << "\n";
            FEEL_ASSERT( counter >= 1 )( __element.id() )( __element.nNeighbors() )( counter ).warn( "invalid neighboring data" );
#if 0
            for ( size_type j = 0; j < (size_type)element_type::numEdges; ++j )
                {
                    FEEL_ASSERT( iv->edgePtr( j ) )( j )( iv->id() ).error( "invalid element edge check" );
                    Debug( 4015 ) << "------------------------------------------------------------\n";
                    Debug( 4015 ) << "Element : " << iv->id() << " edge lid: " << j << " edge gid:  " << iv->edge( j ).id() << "\n";

                }
#endif

        }

    // faces check
    typedef typename super::location_face_const_iterator location_face_const_iterator;
    location_face_const_iterator itf = this->beginFaceOnBoundary();
    location_face_const_iterator ite = this->endFaceOnBoundary();
    for( ; itf != ite; ++ itf )
        {
            face_type const& __face = *itf;
            FEEL_ASSERT( __face.isConnectedTo0() )
                (__face.id() )( __face.G() )( __face.ad_first() )( __face.pos_first() )( __face.proc_first() ).warn( "invalid face" );
            if ( __face.isConnectedTo0() )
                {
                    FEEL_ASSERT( __face.element(0).facePtr( __face.pos_first() ) )
                        ( __face.ad_first() )( __face.pos_first() )( __face.proc_first() )( __face.element(0).id() ).warn( "invalid face in element" );
                }
            FEEL_ASSERT( !__face.isConnectedTo1() )
                ( __face.ad_first() )( __face.pos_first() )( __face.proc_first() ).warn( "invalid boundary face" );

        }
#endif
}
template<typename Shape, typename T>
void
Mesh<Shape, T>::findNeighboringProcessors()
{
    // Don't need to do anything if there is
    // only one processor.
    if (M_comm.size() == 1)
        return;

#ifdef HAVE_MPI

    _M_neighboring_processors.clear();

    // Get the bounding sphere for the local processor
    Sphere bounding_sphere = processorBoundingSphere (*this, M_comm.rank() );

    // Just to be sure, increase its radius by 10%.  Sure would suck to
    // miss a neighboring processor!
    bounding_sphere.setRadius( bounding_sphere.radius()*1.1 );

    // Collect the bounding spheres from all processors, test for intersection
    {
        std::vector<float>
            send (4,                         0),
            recv (4*M_comm.size(), 0);

        send[0] = bounding_sphere.center()(0);
        send[1] = bounding_sphere.center()(1);
        send[2] = bounding_sphere.center()(2);
        send[3] = bounding_sphere.radius();

        MPI_Allgather (&send[0], send.size(), MPI_FLOAT,
                       &recv[0], send.size(), MPI_FLOAT,
                       M_comm );


        for (unsigned int proc=0; proc<M_comm.size(); proc++)
            {
                const Point center (recv[4*proc+0],
                                    recv[4*proc+1],
                                    recv[4*proc+2]);

                const Real radius = recv[4*proc+3];

                const Sphere proc_sphere (center, radius);

                if (bounding_sphere.intersects(proc_sphere))
                    _M_neighboring_processors.push_back(proc);
            }

        // Print out the _neighboring_processors list
        Debug( 4015 ) << "Processor " << M_comm.rank() << " intersects:\n";
        for (unsigned int p=0; p< _M_neighboring_processors.size(); p++)
            Debug( 4015 ) << " - proc " << _M_neighboring_processors[p] << "\n";
    }

#endif
}

template<typename Shape, typename T>
void
Mesh<Shape, T>::checkLocalPermutation( mpl::bool_<true> ) const
{
    bool mesh_well_oriented = true;
    std::vector<size_type> list_of_bad_elts;
    for ( element_const_iterator elt = this->beginElement();
          elt != this->endElement(); ++elt )
        {
            element_type const& __element = *elt;
            if( !__element.isAnticlockwiseOriented() )
                {
                    mesh_well_oriented = false;
                    list_of_bad_elts.push_back( elt->id() );
                }
        }

    if (mesh_well_oriented)
        Debug( 4015 ) << "Local numbering in the elements is OK . \n";
    else
        {
            Debug( 4015 ) << "Local numbering in the elements is not anticlockwise oriented. \n";
            std::for_each( list_of_bad_elts.begin(),
                           list_of_bad_elts.end(),
                           std::cout << lambda::constant( "bad element " ) << lambda::_1 << lambda::constant( "\n") );

        }
}

template<typename Shape, typename T>
void
Mesh<Shape, T>::checkAndFixPermutation(  )
{
    element_type __element = *this->beginElement();
    if ( ! (__element.isATriangleShape() ||
            __element.isATetrahedraShape() ) )
        return;

    static const uint16_type otn_triangle[ 10 ] =
        {
            1, 0, 2, 3, 5, 4
        };
    static const uint16_type otn_tetra[ 10 ] =
        {
            1, 0, 2, 3, 4, 6, 5, 8, 7, 9
        };

    for ( element_iterator elt = this->beginElement();
          elt != this->endElement(); ++elt )
        {
            element_type __element = *elt;
            bool is_anticlockwise = __element.isAnticlockwiseOriented();
            // --verbose
#if 0
            FEEL_ASSERT( is_anticlockwise == true )
                ( is_anticlockwise )
                ( __element.id() )
                ( __element.G() ).warn( "invalid element permutation, will fix it" );
#endif // 0
            // fix permutation
            if( !is_anticlockwise )
                {
                    if ( __element.isATriangleShape() )
                        __element.exchangePoints( otn_triangle );
                    else if ( __element.isATetrahedraShape() )
                        __element.exchangePoints( otn_tetra );
                    else
                        {
                            FEEL_ASSERT( 0 )
                                ( __element.id() )
                                ( __element.G() ).error( "invalid element type" );
                            throw std::logic_error( "invalid element type" );
                        }
                    is_anticlockwise = __element.isAnticlockwiseOriented();
                    FEEL_ASSERT( is_anticlockwise == true )
                        ( is_anticlockwise )
                        ( __element.id() )
                        ( __element.G() ).error( "invalid element permutation" );

                    this->elements().replace( elt, __element );
                }

        }
}

template<typename Shape, typename T>
void
Mesh<Shape, T>::Inverse::distribute( bool extrapolation )
{
    typename self_type::element_iterator el_it;
    typename self_type::element_iterator el_en;
    boost::tie( boost::tuples::ignore, el_it, el_en ) = Feel::elements( *M_mesh );

    typedef typename self_type::element_type element_type;
    typedef typename gm_type::template Context<vm::JACOBIAN|vm::KB|vm::POINT, element_type> gmc_type;
    typedef boost::shared_ptr<gmc_type> gmc_ptrtype;
    BoundingBox<> bb;

    typename gm_type::reference_convex_type refelem;
    typename gm_type::precompute_ptrtype __geopc( new typename gm_type::precompute_type( M_mesh->gm(),
                                                                                         refelem.points() ) );
    std::vector<bool> npt( this->nPoints() );

    M_dist.resize( this->nPoints() );
    M_ref_coords.resize( this->nPoints() );
    M_cvx_pts.resize( this->nPoints() );
    M_pts_cvx.clear();
    M_pts_cvx.resize( M_mesh->numElements() );

    KDTree::points_type boxpts;
    gmc_ptrtype __c( new gmc_type( M_mesh->gm(),
                                   *el_it,
                                   __geopc ) );
    Debug( 4015 ) << "[Mesh::Inverse] distribute mesh points ion kdtree\n";
    for( ; el_it != el_en; ++el_it )
        {
            // get geometric transformation
            __c->update( *el_it );
            gic_type gic( M_mesh->gm(), *el_it );

            // create bounding box
            //bb.make( el_it->points() );
            bb.make( el_it->G() );
            for (size_type k=0; k < bb.min.size(); ++k) { bb.min[k] -= 1e-10; bb.max[k] += 1e-10; }
#if !defined( NDEBUG )
            Debug( 4015 ) << "G = " << el_it->G() << " min = " << bb.min << ", max = " << bb.max << "\n";
#endif /* NDEBUG */


            // check if the points
            this->pointsInBox( boxpts, bb.min, bb.max );
#if !defined( NDEBUG )
            Debug( 4015 ) << "boxpts size = " << boxpts.size() << "\n";
#endif /*  */

            for( size_type i = 0; i < boxpts.size(); ++i )
                {
                    size_type index = boost::get<1>( boxpts[i] );
                    if ( (!npt[index]) || M_dist[index] < 0 )
                        {
                            // check if we are in
                            gic.setXReal( boost::get<0>( boxpts[i] ) );
                            bool isin;
                            value_type dmin;
                            boost::tie( isin, dmin ) = refelem.isIn( gic.xRef() );
                            bool tobeadded = extrapolation || isin;

#if !defined( NDEBUG )
                            Debug( 4015 ) << "i = " << i << " index = " << index << " isin = " << ( isin >= -1e-10 )  << " xref = " << gic.xRef() << " xreal = " << boost::get<0>( boxpts[i] ) << " tobeadded= " << tobeadded << " dist=" << dmin<< "\n";
#endif

                            if ( tobeadded && npt[index] )
                                {
                                    if ( dmin > M_dist[index] )
                                        M_pts_cvx[M_cvx_pts[index]].erase( index );
                                    else
                                        tobeadded = false;
                                }
                            if  ( tobeadded )
                                {
                                    M_ref_coords[index] = gic.xRef();
                                    M_dist[index] = dmin;
                                    M_cvx_pts[index]=el_it->id();
                                    M_pts_cvx[el_it->id()][index]=boost::get<2>( boxpts[i] );
                                    npt[index]=true;
                                }
                        }
                }

        }
    Debug( 4015 ) << "[Mesh::Inverse] distribute mesh points in kdtree done\n";
}

template<typename Shape, typename T>
void
Mesh<Shape, T>::Localization::init()
{

#if !defined( NDEBUG )
    FEEL_ASSERT( IsInit == false )
        ( IsInit ).warn( "You have already initialized the tool of localization" );
#endif
    //clear data
    M_geoGlob_Elts.clear();
    M_kd_tree->clear();

    typename self_type::element_iterator el_it;
    typename self_type::element_iterator el_en;
    boost::tie( boost::tuples::ignore, el_it, el_en ) = Feel::elements( *M_mesh );

    for( ; el_it != el_en; ++el_it )
        {
            for (int i=0;i<el_it->nPoints();++i)
                {
                    if (boost::get<1>( M_geoGlob_Elts[el_it->point(i).id()] ).size()==0)
                        {
                            boost::get<0>( M_geoGlob_Elts[el_it->point(i).id()] ) = el_it->point(i).node();
                            M_kd_tree->addPoint(el_it->point(i).node(),el_it->point(i).id() );
                        }
                    boost::get<1>( M_geoGlob_Elts[el_it->point(i).id()] ).push_back(el_it->id());
                }
        }

    IsInit=true;

}

template<typename Shape, typename T>
boost::tuple<bool, size_type, typename Mesh<Shape, T>::node_type>
Mesh<Shape, T>::Localization::searchElement(const node_type & p)
{

#if !defined( NDEBUG )
    FEEL_ASSERT( IsInit == true )
        ( IsInit ).warn( "You don't have initialized the tool of localization" );
#endif

    //search for nearest points
    M_kd_tree->search(p);

    //get the results of research
    typename KDTree::points_search_type ptsNN = M_kd_tree->pointsNearNeighbor();

    typename KDTree::points_search_const_iterator itNN = ptsNN.begin();
    typename KDTree::points_search_const_iterator itNN_end = ptsNN.end();

#if !defined( NDEBUG )
    FEEL_ASSERT( std::distance(itNN,itNN_end)>0 ).error( "none Near Neighbor Points are find" );
#endif

    //iterator on a l(ist index element
    typename std::list<size_type>::iterator itL;
    typename std::list<size_type>::iterator itL_end;

    //ListTri will contain the indices of elements (size_type)
    //and the number of occurence(uint)
    std::list< std::pair<size_type, uint> > ListTri;
    std::list< std::pair<size_type, uint> >::iterator itLT;
    std::list< std::pair<size_type, uint> >::iterator itLT_end;

    //create of ListTri : sort largest to smallest occurrences
    //In case of equality : if the point is closer than another then it will be before
    //                      if it is in the same point then  the lowest index will be before
    for( ; itNN != itNN_end; ++itNN ) {
        itL= boost::get<1>( M_geoGlob_Elts[boost::get<3>( *itNN )] ).begin();
        itL_end= boost::get<1>( M_geoGlob_Elts[boost::get<3>( *itNN )] ).end();
        for( ; itL != itL_end; ++itL ) {
            itLT=ListTri.begin();
            itLT_end=ListTri.end();bool find=false;
            while (itLT != itLT_end && !find ) {
                if (itLT->first == *itL) find=true;
                else ++itLT;
            }
            if (find) {
                uint nb=itLT->second+1;
                size_type numEl=itLT->first;
                ListTri.remove(*itLT);
                itLT=ListTri.begin();
                itLT_end=ListTri.end();bool find=false;
                while (itLT != itLT_end && !find ) {
                    if (itLT->second < nb) find=true;
                    else ++itLT;
                }
                ListTri.insert(itLT,std::make_pair(numEl,nb));
            }
            else ListTri.push_back(std::make_pair(*itL,1));
        }
    }

    typename self_type::element_type elt;
    typename self_type::gm_type::reference_convex_type refelem;

    bool isin=false;
    double dmin;
    node_type __x_ref;

    //research the element which contains the point p
    itLT=ListTri.begin();
    itLT_end=ListTri.end();

#if !defined( NDEBUG )
    //if(std::distance(itLT,itLT_end)==0) std::cout<<"\nListTri vide\n";
    FEEL_ASSERT( std::distance(itLT,itLT_end)>0 ).error( " problem in list localization : is empty" );
#endif

    while (itLT != itLT_end && !isin  )
        {
            //get element with the id
            elt= M_mesh->element(itLT->first );

            // get inverse geometric transformation
            typename self_type::Inverse::gic_type gic( M_mesh->gm(), elt );

            //apply the inverse geometric transformation for the point p
            gic.setXReal( p);
            __x_ref=gic.xRef();

            // the point is in the reference element ?
            boost::tie( isin, dmin ) = refelem.isIn( gic.xRef() );

            //if not inside, continue the research with an other element
            if (!isin) ++itLT;
        }

    bool __extrapolation=true;
    if (itLT != itLT_end) return boost::make_tuple( true, itLT->first, __x_ref);
    else if (itLT == itLT_end && !__extrapolation) return boost::make_tuple( false, 0, __x_ref );
    else
        {
            //std::cout << "\n WARNING EXTRAPOLATION \n";
            itLT=ListTri.begin();
            elt= M_mesh->element(itLT->first );
            typename self_type::Inverse::gic_type gic( M_mesh->gm(), elt );
            //apply the inverse geometric transformation for the point p
            //gic.setXReal(boost::get<0>(*ptsNN.begin()));
            gic.setXReal( p);
            __x_ref=gic.xRef();
            return boost::make_tuple( true, itLT->first, __x_ref);
        }

}

template<typename Shape, typename T>
void
Mesh<Shape, T>::Localization::run_analysis(const matrix_node_type & m)
{

#if !defined( NDEBUG )
    FEEL_ASSERT( IsInit == true )
        ( IsInit ).warn( "You don't have initialized the tool of localization" );
#endif

    bool find_x;
    size_type __cv_id;
    node_type __x_ref;

    M_resultAnalysis.clear();

    for (size_type i=0;i< m.size2();++i)
        {
            boost::tie( find_x, __cv_id, __x_ref ) = this->searchElement(ublas::column( m, i ));

            if (find_x)
                {
                    M_resultAnalysis[__cv_id].push_back( boost::make_tuple(i,__x_ref) );
                }
            //else std::cout<<"\nProbleme Localization\n";

#if !defined( NDEBUG )
            //FEEL_ASSERT( false )
            //( false ).warn( "problem localization" );
            //std::cout<<"\nProbleme de Localisation : "<<ublas::column( m, i )<<"\n";
            //this->M_kd_tree->showResultSearch();}
#endif

        }
} //run_analysis



template<typename Shape, typename T>
boost::tuple<bool, std::list<boost::tuple<size_type, typename Mesh<Shape, T>::node_type> > >
Mesh<Shape, T>::Localization::searchElementBis(const node_type & p)
{

#if !defined( NDEBUG )
    FEEL_ASSERT( IsInit == true )
        ( IsInit ).warn( "You don't have initialized the tool of localization" );
#endif

    //search for nearest points
    M_kd_tree->search(p);

    //get the results of research
    typename KDTree::points_search_type ptsNN = M_kd_tree->pointsNearNeighbor();

    typename KDTree::points_search_const_iterator itNN = ptsNN.begin();
    typename KDTree::points_search_const_iterator itNN_end = ptsNN.end();

#if !defined( NDEBUG )
    FEEL_ASSERT( std::distance(itNN,itNN_end)>0 ).error( "none Near Neighbor Points are find" );
#endif

    //iterator on a l(ist index element
    typename std::list<size_type>::iterator itL;
    typename std::list<size_type>::iterator itL_end;

    //ListTri will contain the indices of elements (size_type)
    //and the number of occurence(uint)
    std::list< std::pair<size_type, uint> > ListTri;
    std::list< std::pair<size_type, uint> >::iterator itLT;
    std::list< std::pair<size_type, uint> >::iterator itLT_end;

    //create of ListTri : sort largest to smallest occurrences
    //In case of equality : if the point is closer than another then it will be before
    //                      if it is in the same point then  the lowest index will be before
    for( ; itNN != itNN_end; ++itNN ) {
        itL= boost::get<1>( M_geoGlob_Elts[boost::get<3>( *itNN )] ).begin();
        itL_end= boost::get<1>( M_geoGlob_Elts[boost::get<3>( *itNN )] ).end();
        for( ; itL != itL_end; ++itL ) {
            itLT=ListTri.begin();
            itLT_end=ListTri.end();bool find=false;
            while (itLT != itLT_end && !find ) {
                if (itLT->first == *itL) find=true;
                else ++itLT;
            }
            if (find) {
                uint nb=itLT->second+1;
                size_type numEl=itLT->first;
                ListTri.remove(*itLT);
                itLT=ListTri.begin();
                itLT_end=ListTri.end();bool find=false;
                while (itLT != itLT_end && !find ) {
                    if (itLT->second < nb) find=true;
                    else ++itLT;
                }
                ListTri.insert(itLT,std::make_pair(numEl,nb));
            }
            else ListTri.push_back(std::make_pair(*itL,1));
        }
    }

    typename self_type::element_type elt;
    typename self_type::gm_type::reference_convex_type refelem;

    bool isin=false;
    double dmin;
    node_type __x_ref;

    //research the element which contains the point p
    itLT=ListTri.begin();
    itLT_end=ListTri.end();

#if !defined( NDEBUG )
    //if(std::distance(itLT,itLT_end)==0) std::cout<<"\nListTri vide\n";
    FEEL_ASSERT( std::distance(itLT,itLT_end)>0 ).error( " problem in list localization : is empty" );
#endif

    std::list<boost::tuple<size_type,node_type> > newlistelts;newlistelts.clear();
    bool find = false;
    while (itLT != itLT_end /*&& !isin*/  )
        {
            //get element with the id
            elt= M_mesh->element(itLT->first );

            // get inverse geometric transformation
            typename self_type::Inverse::gic_type gic( M_mesh->gm(), elt );

            //apply the inverse geometric transformation for the point p
            gic.setXReal( p);
            __x_ref=gic.xRef();

            // the point is in the reference element ?
            boost::tie( isin, dmin ) = refelem.isIn( gic.xRef() );

            if (isin) { newlistelts.push_back(boost::make_tuple(itLT->first,__x_ref) );find = true; }
            //if not inside, continue the research with an other element
            //if (!isin) ++itLT;
            ++itLT;
        }


    //boost::tuple<bool, std::list<boost::tuple<size_type,node_type> > > newsol;

    auto newsol = boost::make_tuple(find,newlistelts);

    bool __extrapolation=true;
    if (find/*itLT != itLT_end*/) {
        //std::cout << "\nOK listsize "<< newlistelts.size()<<"\n";
        return boost::make_tuple(true,newlistelts);
    }
    else if (!find && !__extrapolation) return boost::make_tuple(false,newlistelts);
    else
        {
            std::cout << "\n WARNING EXTRAPOLATION \n";
            itLT=ListTri.begin();
            elt= M_mesh->element(itLT->first );
            typename self_type::Inverse::gic_type gic( M_mesh->gm(), elt );
            //apply the inverse geometric transformation for the point p
            //gic.setXReal(boost::get<0>(*ptsNN.begin()));
            gic.setXReal( p);
            __x_ref=gic.xRef();
            //return boost::make_tuple( true, itLT->first, __x_ref);
            newlistelts.push_back(boost::make_tuple(itLT->first,__x_ref) );find = true;
            return boost::make_tuple(true,newlistelts);
        }
} // searchElementBis



} // namespace Feel





#endif // __MESHIMPL_HPP

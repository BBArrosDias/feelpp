/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=tcl:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-07-05

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2007,2008,2009,2010 Université Joseph Fourier (Grenoble I)

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
   \file mesh.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-07-05
 */
#ifndef __mesh_H
#define __mesh_H 1

#include <map>

#include <boost/foreach.hpp>
#include <boost/signal.hpp>

#include <feel/feelcore/context.hpp>

#include <feel/feelmesh/mesh1d.hpp>
#include <feel/feelmesh/mesh2d.hpp>
#include <feel/feelmesh/mesh3d.hpp>
#include <feel/feelmesh/meshutil.hpp>
#include <feel/feelmesh/filters.hpp>
#include <feel/feelmesh/enums.hpp>

#include <feel/feelpoly/geomap.hpp>
#include <feel/feelalg/boundingbox.hpp>
#include <feel/feelpoly/geomapinv.hpp>




#include <boost/preprocessor/comparison/less.hpp>
#include <boost/preprocessor/logical/and.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/list/at.hpp>
#include <boost/preprocessor/list/cat.hpp>
#include <boost/preprocessor/list/for_each_product.hpp>
#include <boost/preprocessor/logical/or.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include <boost/preprocessor/tuple/eat.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/facilities/identity.hpp>

#include <boost/enable_shared_from_this.hpp>


namespace Feel
{
const size_type EXTRACTION_KEEP_POINTS_IDS                = ( 1<<0 );
const size_type EXTRACTION_KEEP_EDGES_IDS                 = ( 1<<1 );
const size_type EXTRACTION_KEEP_FACES_IDS                 = ( 1<<2 );
const size_type EXTRACTION_KEEP_VOLUMES_IDS               = ( 1<<3 );
const size_type EXTRACTION_KEEP_ALL_IDS                   = ( EXTRACTION_KEEP_POINTS_IDS |
                                                              EXTRACTION_KEEP_EDGES_IDS |
                                                              EXTRACTION_KEEP_FACES_IDS |
                                                              EXTRACTION_KEEP_VOLUMES_IDS );

/**
 * partitioner base class
 */
template<typename Mesh> class Partitioner;

/*!
  \class mesh
  \brief unifying mesh class

  @author Christophe Prud'homme
  @see
*/
//    template <typename GeoShape, typename T > class Mesh;

template <typename GeoShape, typename T = double>
class Mesh
    :
        public mpl::if_<mpl::equal_to<mpl::int_<GeoShape::nDim>,mpl::int_<1> >,
                        mpl::identity<Mesh1D<GeoShape > >,
                        typename mpl::if_<mpl::equal_to<mpl::int_<GeoShape::nDim>,mpl::int_<2> >,
                                          mpl::identity<Mesh2D<GeoShape> >,
                                          mpl::identity<Mesh3D<GeoShape> > >::type>::type::type,
        public boost::enable_shared_from_this< Mesh<GeoShape,T> >
{
    typedef typename mpl::if_<mpl::equal_to<mpl::int_<GeoShape::nDim>,mpl::int_<1> >,
                              mpl::identity<Mesh1D<GeoShape> >,
                              typename mpl::if_<mpl::equal_to<mpl::int_<GeoShape::nDim>,mpl::int_<2> >,
                                                mpl::identity<Mesh2D<GeoShape> >,
                                                mpl::identity<Mesh3D<GeoShape> > >::type>::type::type super;
public:


    /** @name Constants
     */
    //@{

    static const uint16_type nDim = GeoShape::nDim;
    static const uint16_type nRealDim = GeoShape::nRealDim;
    static const uint16_type Shape = GeoShape::Shape;
    static const uint16_type nOrder = GeoShape::nOrder;

    //@}
    /** @name Typedefs
     */
    //@{
    typedef T value_type;
    typedef GeoShape shape_type;
    typedef typename super::return_type return_type;
    typedef typename node<double>::type node_type;

    typedef typename super::super_elements super_elements;
    typedef typename super::elements_type elements_type;
    typedef typename super::element_type element_type;
    typedef typename super::element_iterator element_iterator;
    typedef typename super::element_const_iterator element_const_iterator;

    typedef typename super::super_faces super_faces;
    typedef typename super::faces_type faces_type;
    typedef typename super::face_type face_type;
    typedef typename super::face_iterator face_iterator;
    typedef typename super::face_const_iterator face_const_iterator;

    typedef typename super::points_type points_type;
    typedef typename super::point_type point_type;
    typedef typename super::point_iterator point_iterator;
    typedef typename super::point_const_iterator point_const_iterator;

    typedef typename element_type::gm_type gm_type;
    typedef boost::shared_ptr<gm_type> gm_ptrtype;

    typedef typename element_type::gm1_type gm1_type;
    typedef boost::shared_ptr<gm1_type> gm1_ptrtype;

    template<size_type ContextID>
    struct gmc
    {
        typedef typename gm_type::template Context<ContextID, element_type> type;
        typedef boost::shared_ptr<type> ptrtype;
    };

    typedef Mesh<shape_type, T> self_type;
    typedef boost::shared_ptr<self_type> self_ptrtype;

    typedef typename element_type::template reference_convex<T>::type reference_convex_type;

    typedef Partitioner<self_type> partitioner_type;
    typedef boost::shared_ptr<partitioner_type> partitioner_ptrtype;

    typedef typename super::face_processor_type face_processor_type;
    typedef typename super::face_processor_type element_edge_type;

    typedef typename mpl::if_<mpl::bool_<GeoShape::is_simplex>,
                              mpl::identity< Mesh< Simplex< GeoShape::nDim,1,GeoShape::nRealDim>, value_type > >,
                              mpl::identity< Mesh< Hypercube<GeoShape::nDim,1,GeoShape::nRealDim>,value_type > > >::type::type P1_mesh_type;

    typedef boost::shared_ptr<P1_mesh_type> P1_mesh_ptrtype;

    //@}

    /**
     * Default mesh constructor
     */
    Mesh( std::string partitioner = "metis" );

    /** @name Accessors
     */
    //@{

    /**
     * \return the topological dimension
     */
    uint16_type dimension() const { return nDim; }

    /**
     * \return geometric mapping
     */
    gm_ptrtype const& gm() const { return _M_gm; }

/**
     * \return geometric mapping of order 1
     */
    gm1_ptrtype const& gm1() const { return _M_gm1; }

    /**
     * \return the geometric mapping
     */
    gm_ptrtype& gm() { return _M_gm; }

 /**
     * \return the geometric mapping of order 1
     */
    gm1_ptrtype& gm1() { return _M_gm1; }

    /**
     * \return the reference convex associated with the element of the
     * mesh
     */
    reference_convex_type referenceConvex() const { return reference_convex_type(); }

    /**
     * \return the face index of the face \p n in the element \p e
     */
    face_processor_type const& localFaceId( element_type const& e,
                                       size_type const n ) const
    {
        return _M_e2f[e.id()][n];
    }

    /**
     * \return the face index of the face \p n in the element \p e
     */
    face_processor_type const& localFaceId( size_type const e,
                                       size_type const n ) const
    {
        return _M_e2f[e][n];
    }

    /**
     * \return the communicator
     */
    mpi::communicator const& comm() const { return M_comm; }

    //@}

    /** @name  Mutators
     */
    //@{

    /**
     * set the partitioner to \p partitioner
     */
    void setPartitioner( std::string partitioner )
    {
        M_part = partitioner_ptrtype( partitioner_type::New( partitioner ) );
    }

    /**
     * \return the face index of the face \p n in the element \p e
     */
    face_processor_type& localFaceId( element_type const& e,
                                 size_type const n )
    {
        return _M_e2f[e.id()][n];
    }

    /**
     * \return the face index of the face \p n in the element \p e
     */
    face_processor_type& localFaceId( size_type const e,
                                 size_type const n )
    {
        return _M_e2f[e][n];
    }

    /**
     * \return the id associated to the \p marker
     */
    int markerName( std::string const& marker ) const
    {
        return M_markername.find( marker )->second.template get<0>();
    }
    /**
     * \return the topological dimension associated to the \p marker
     */
    int markerDim( std::string const& marker ) const
    {
        return M_markername.find( marker )->second.template get<1>();
    }

    /**
     * \return the marker names
     */
    std::map<std::string, boost::tuple<int, int> > markerNames() const { return M_markername; }

    /**
     * \return a localization tool
     */
    struct Localization;
    boost::shared_ptr<typename self_type::Localization> tool_localization() { return M_tool_localization;}

    /**
     * \return the measure of the mesh (sum of the measure of the elements)
     */
    value_type measure() const { return M_meas; }

    /**
     * \return the measure of the mesh (sum of the measure of the elements)
     */
    value_type measureBoundary() const { return M_measbdy; }

    //@}

    /** @name  Methods
     */
    //@{

    /**
     * add a new marker name
     */
    void addMarkerName( std::pair<std::string, boost::tuple<int,int> > const& marker )
    {
        M_markername.insert( marker );
    }

    /**
     * add a new marker name
     */
    void addMarkerName( std::string __name, int __id ,int __topoDim )
    {
        auto data = boost::make_tuple(__id,__topoDim );
        M_markername[__name]=data;
    }

    /**
     * creates a mesh by iterating over the elements between
     * \p begin_elt and and \p end_elt and adding them the the mesh
     * \p mesh
     *
     * \param mesh new mesh to construct
     * \param begin_elt begin iterator
     * \param begin_elt end iterator
     * \param extraction_policies not in use yet
     *
     * \todo make use of \c extraction_policies
     */
    template<typename Iterator>
    void createSubmesh( self_type& mesh,
                        Iterator const& begin_elt,
                        Iterator const& end_elt,
                        size_type extraction_policies = EXTRACTION_KEEP_ALL_IDS ) const;

    /**
     * A special case of \p createSubmesh() : it creates a mesh by
     * iterating over all elements having the process id \p pid
     *
     * \param mesh new mesh to construct
     * \param pid process id that will select the elements to add
     */
    void createSubmeshByProcessId( self_type& mesh, uint16_type pid ) const
    {
        this->createSubmesh( mesh,
                             this->beginElementWithProcessId( pid ),
                             this->endElementWithProcessId( pid ) );
    }

    /**
     * Create a P1 mesh from the HO mesh
     */
    P1_mesh_ptrtype createP1mesh() const;

    /**
     * Call the default partitioner (currently \p metis_partition()).
     */
    void partition ( const uint16_type n_parts = 1 );

    /**
     * After loading/defining a mesh, we want to have as much locality
     * as possible (elements/faces/nodes to be contiguous). In order
     * to do that the mesh elements/faces/nodes are renumbered. That
     * will be then most helpful when generating the \p Dof table.
     * This procedure should work also with
     * \p comm().size() == 1
     */
    void renumber() { renumber( mpl::bool_<(nDim > 1)>() ); }


    /**
     * This function only take sense in the 3D modal case with a simplex mesh.
     * In order to construct a global C^0 expansion, we need to assure that
     * two contiguous elements share the same top vertex !
     * This can be achieve by a local renumbering of the vertex.
     */

    void localrenumber();

    /**
     * check elements permutation and fix it if needed
     */
    void checkAndFixPermutation();

    FEEL_DEFINE_VISITABLE();
    //@}

    //private:

    /**
     * Finds all the processors that may contain
     * elements that neighbor my elements.  This list
     * is guaranteed to include all processors that border
     * any of my elements, but may include additional ones as
     * well.  This method computes bounding boxes for the
     * elements on each processor and checks for overlaps.
     */
    void findNeighboringProcessors();

    /**
     * This function checks if the local numbering of the mesh elements
     * is anticlockwise oriented. For the time being, this function
     * only applies to tetrahedra meshes
     */
    void checkLocalPermutation(mpl::bool_<false>) const {};
    void checkLocalPermutation(mpl::bool_<true>) const;


    /**
     * update the mesh data structure before using it
     *
     * by default if the number of processors if > 1, we partition
     * the mesh. A different behaviour is controlled by setting
     * properly \p setComponents(), \p components()
     */
    void updateForUse();

    struct Inverse
        :
        public mpl::if_<mpl::bool_<GeoShape::is_simplex>,
                        mpl::identity<GeoMapInverse<nDim,nOrder,nRealDim,T,Simplex> >,
                        mpl::identity<GeoMapInverse<nDim,nOrder,nRealDim,T,Hypercube> > >::type::type
    {
        typedef typename mpl::if_<mpl::bool_<GeoShape::is_simplex>,
                                  mpl::identity<GeoMapInverse<nDim,nOrder,nRealDim,T,Simplex> >,
                                  mpl::identity<GeoMapInverse<nDim,nOrder,nRealDim,T,Hypercube> > >::type::type super;
        typedef typename super::gic_type gic_type;

        Inverse( boost::shared_ptr<self_type> const& m )
            :
            super(),
            M_mesh ( m )
        {}

        ~Inverse()
        {}

        size_type nPointsInConvex(size_type i) const
        {
            return M_pts_cvx[i].size();
        }
        void pointsInConvex(size_type i, std::vector<boost::tuple<size_type, uint16_type > > &itab) const
        {
            itab.resize( M_pts_cvx[i].size() );
            size_type j = 0;
            for (map_iterator it = M_pts_cvx[i].begin(); it != M_pts_cvx[i].end(); ++it)
                itab[j++] = boost::make_tuple( it->first, it->second );
        }

        const std::vector<node_type> &referenceCoords(void)
        {
            return M_ref_coords;
        }

        /**
         * distribute the points of the mesh in a kdtree
         *
         * extrapolation = false : Only the points inside the mesh are distributed.
         * extrapolation = true  : Try to project the exterior points.
         * \todo : for extrapolation, verify that all the points have been taken
         *        into account, else test them on the frontiere convexes.
         */
        void distribute( bool extrapolation = false );

    private:
        boost::shared_ptr<self_type> M_mesh;
        std::vector<std::map<size_type,uint16_type > > M_pts_cvx;
        typedef typename std::map<size_type, uint16_type >::const_iterator map_iterator;
        //typedef typename node<value_type>::type node_type;
        std::vector<node_type> M_ref_coords;
        std::vector<double> M_dist;
        std::vector<size_type> M_cvx_pts;

    }; // Inverse

    struct Localization
    {

        typedef Localization localization_type;
        typedef boost::shared_ptr<localization_type> localization_ptrtype;

        typedef typename matrix_node<typename node_type::value_type>::type matrix_node_type;

        typedef KDTree kdtree_type;
        typedef typename boost::shared_ptr<KDTree> kdtree_ptrtype;

        // a node x => a list of id elt which contain the node x
        typedef boost::tuple<node_type, std::list<size_type> > node_elem_type;

        typedef typename std::list<boost::tuple<size_type,node_type> > container_output_type;
        typedef typename std::list<boost::tuple<size_type,node_type> >::iterator container_output_iterator_type;
        //map between element id and list of node described in the reference elt
        //typedef std::map<size_type, std::list<boost::tuple<size_type,node_type> > > container_search_type
        typedef std::map<size_type, container_output_type > container_search_type;
        typedef typename container_search_type::const_iterator container_search_const_iterator_type;
        typedef typename container_search_type::iterator container_search_iterator_type;

        /**--------------------------------------------------------------
         * Constructors
         */
        Localization() :
            M_mesh (),
            M_kd_tree(new kdtree_type()),
            IsInit(false)

        {
            M_kd_tree->nbNearNeighbor(15);
            M_resultAnalysis.clear();
        }

        Localization(boost::shared_ptr<self_type> m, bool init_b = true ) :
            M_mesh ( m ),
            IsInit(init_b)
        {
            if (IsInit)
                init();

            M_kd_tree->nbNearNeighbor(15);
            M_resultAnalysis.clear();
        }

        Localization(Localization const & L) :
            M_mesh(L.M_mesh),
            //M_kd_tree(L.M_kd_tree),
            M_kd_tree(new kdtree_type(*(L.M_kd_tree))),
            M_geoGlob_Elts(L.M_geoGlob_Elts),
            IsInit(L.IsInit),
            M_resultAnalysis(L.M_resultAnalysis)
        {}

        /*--------------------------------------------------------------
         * Define the mesh whith or not init
         */
        void
        setMesh(boost::shared_ptr<self_type> m,bool b=true)
        {
            M_mesh=m;
            if (b)
                init();
            else IsInit=b;
            M_resultAnalysis.clear();
        }

        /*--------------------------------------------------------------
         * Run the init function if necessary
         */
        void updateForUse()
        {
            if (IsInit==false)
                init();
        }

        /*--------------------------------------------------------------
         * Access
         */

        bool isInit() { return IsInit;}

        //KDTree kdtree() { return M_kd_tree; }
        kdtree_ptrtype kdtree() { return M_kd_tree; }

        container_search_type const & result_analysis() { return M_resultAnalysis;}

        //container_search_type & result_analysis() { return M_resultAnalysis;}

        container_search_iterator_type result_analysis_begin() { return M_resultAnalysis.begin();}
        container_search_iterator_type result_analysis_end() { return M_resultAnalysis.end();}


        /*---------------------------------------------------------------
         * Research the element wich contains the node p
         */
        boost::tuple<bool, size_type,node_type> searchElement(const node_type & p);
        boost::tuple<bool, std::list<boost::tuple<size_type,node_type> > > searchElementBis(const node_type & p);

        /*---------------------------------------------------------------
         * Research the element wich contains the node p, forall p in the
         * matrix_node_type m. The result is save by this object
         */
        void run_analysis(const matrix_node_type & m);

        /*
         * Reset all data
         */
        void reset()
        {
            IsInit=false;
            init();
        }

    private :

        /*---------------------------------------------------------------
         *initializes the kd tree and the map between node and list elements
         */
        void init();

    private:

        boost::shared_ptr<self_type> M_mesh;
        //KDTree M_kd_tree;
        kdtree_ptrtype M_kd_tree;
        //map between node and list elements
        std::map<size_type, node_elem_type > M_geoGlob_Elts;
        bool IsInit;
        container_search_type M_resultAnalysis;
    };


    /** @name  Signals
     */
    //@{


    /**
     * mesh changed its connectivity
     */
    boost::signal<void ( MESH_CHANGES )> meshChanged;

    template<typename Observer>
    void addObserver( Observer& obs )
    {
        meshChanged.connect( obs );
    }

    //@}

protected:
    /**
     * Update connectiviry of entities of codimension 1
     */
    void updateEntitiesCoDimensionOne();


    /**
     * check mesh connectivity
     */
    void check() const;

private:

    /**
     * \sa renumber()
     */
    void renumber( mpl::bool_<false> ) {}

    /**
     * \sa renumber()
     */
    void renumber( mpl::bool_<true> );

private:

    mpi::communicator M_comm;

    gm_ptrtype _M_gm;
    gm1_ptrtype _M_gm1;

    //! measure of the mesh
    value_type M_meas;

    //! measure of the boundary of the mesh
    value_type M_measbdy;

    /**
     * The processors who neighbor the current
     * processor
     */
    std::vector<uint16_type> _M_neighboring_processors;

    partitioner_ptrtype M_part;

    /**
     * Arrays containing the global ids of Faces of each element
     */
    boost::multi_array<face_processor_type,2> _M_e2f;

    /**
     * Arrays containing the global ids of edges of each element
     */
    boost::multi_array<element_edge_type,2> _M_e2e;

    /**
     * marker name disctionnary ( std::string -> <int,int> )
     * get<0>() provides the id
     * get<1>() provides the topological dimension
     */
    std::map<std::string, boost::tuple<int, int> > M_markername;

    /**
     * tool for localize point in the mesh
     */
    boost::shared_ptr<Localization> M_tool_localization;
};



template<typename Shape, typename T>
template<typename Iterator>
void
Mesh<Shape, T>::createSubmesh( self_type& new_mesh,
                               Iterator const& begin_elt,
                               Iterator const& end_elt,
                               size_type extraction_policies ) const
{
    Context policies( extraction_policies );

    Debug( 4015 ) << "[Mesh<Shape,T>::createSubmesh] start\n";
    // make sure it is all empty
    new_mesh.clear();
    // inherit all the markers
    new_mesh.M_markername = this->markerNames();
    BOOST_FOREACH( auto marker, new_mesh.M_markername )
    {
        std::cout << "marker name " << marker.first
                  << " id: " << marker.second.template get<0>()
                  << " geoe: " << marker.second.template get<1>() << "\n";

    }
    // How the nodes on this mesh will be renumbered to nodes
    // on the new_mesh.
    std::vector<size_type> new_node_numbers (this->numPoints());
    std::vector<size_type> new_vertex (this->numPoints());

    std::fill ( new_node_numbers.begin(),
                new_node_numbers.end(),
                invalid_size_type_value );

    std::fill ( new_vertex.begin(),
                new_vertex.end(),
                0 );



    // the number of nodes on the new mesh, will be incremented
    unsigned int n_new_nodes = 0;
    unsigned int n_new_elem  = 0;
    size_type n_new_faces = 0;

    for ( Iterator it = begin_elt; it != end_elt; ++it )
        {

            element_type const& old_elem = *it;

            // copy element so that we can modify it
            element_type new_elem = old_elem;

            // Loop over the nodes on this element.
            for (unsigned int n=0; n < old_elem.nPoints(); n++)
                {
                    FEEL_ASSERT (old_elem.point( n ).id() < new_node_numbers.size()).error( "invalid point id()" );

                    if ( new_node_numbers[old_elem.point(n).id()] == invalid_size_type_value )
                        {
                            new_node_numbers[old_elem.point(n).id()] = n_new_nodes;

                            Debug( 4015 ) << "[Mesh<Shape,T>::createSubmesh] insert point " << old_elem.point(n) << "\n";

                            point_type pt( old_elem.point(n) );
                            pt.setId( n_new_nodes );

                            // Add this node to the new mesh
                            new_mesh.addPoint (pt);

                            Debug( 4015 ) << "[Mesh<Shape,T>::createSubmesh] number of  points " << new_mesh.numPoints() << "\n";

                            // Increment the new node counter
                            n_new_nodes++;

                            if ( n < element_type::numVertices )
                                {
                                    FEEL_ASSERT( new_vertex[old_elem.point(n).id()] == 0 ).error( "already seen this point?" );
                                    new_vertex[old_elem.point(n).id()]=1;
                                }
                        }

                    // Define this element's connectivity on the new mesh
                    FEEL_ASSERT (new_node_numbers[old_elem.point(n).id()] < new_mesh.numPoints()).error("invalid connectivity");

                    Debug( 4015 ) << "[Mesh<Shape,T>::createSubmesh] adding point old(" << old_elem.point(n).id()
                                  << ") as point new(" << new_node_numbers[old_elem.point(n).id()]
                                  << ") in element " << new_elem.id() << "\n";

                    new_elem.setPoint(n, new_mesh.point( new_node_numbers[old_elem.point(n).id()] ) );

                }

            // set id of element
            new_elem.setId ( n_new_elem );

            // increment the new element counter
            n_new_elem++;


            // Add an equivalent element type to the new_mesh
            new_mesh.addElement( new_elem );


            // Maybe add faces for this element
            for (unsigned int s=0; s<old_elem.numTopologicalFaces; s++)
                {
                    if ( !old_elem.facePtr(s) ) continue;
#if 1
                    std::cout << "local face id: " << s
                              << " global face id: " << old_elem.face(s).id() << "\n";
#endif
                    // only add face on the boundary: they have some data
                    // (boundary ids) which cannot be retrieved otherwise
                    //if ( old_elem.neighbor(s) == invalid_size_type_value )
                    size_type global_face_id = old_elem.face(s).id();
                    if ( this->hasFace( global_face_id ) )
                    {

                        //std::cout << "found face " << global_face_id << "\n";
                        // get the corresponding face
                        face_type const& old_face = old_elem.face(s);
                        face_type new_face = old_face;

                        // disconnect from elements of old mesh,
                        // the connection will be redone in
                        // \c updateForUse()
                        new_face.disconnect();
                        //std::cout << "disconnect face\n";
                        // update points info
                        for ( uint16_type p = 0;p < new_face.nPoints(); ++p )
                            {
#if 1
                                std::cout << "add new point " << new_face.point(p).id() << " to face \n";
                                std::cout << "add old point " << old_face.point(p).id() << " to face \n";
                                std::cout << "new point id " << new_node_numbers[old_elem.point(old_elem.fToP(s,p)).id()] << "\n";
#endif
                                new_face.setPoint( p, new_mesh.point( new_node_numbers[old_elem.point(old_elem.fToP(s,p)).id()] ) );

                            }
                        new_face.setId( n_new_faces++ );
#if 1
                        std::cout << "face id" << new_face.id()
                                  << " marker1 : " << new_face.marker()
                                  << " old marker1 : " << old_face.marker()
                                  << "\n";
#endif
                        // add it to the list of faces
                        new_mesh.addFace( new_face );
                    }
                }
        }
    new_mesh.setNumVertices( std::accumulate( new_vertex.begin(), new_vertex.end(), 0 ) );

    Debug( 4015 ) << "[Mesh<Shape,T>::createSubmesh] update face/edge info if necessary\n";
    // Prepare the new_mesh for use
    new_mesh.components().set ( MESH_RENUMBER|MESH_UPDATE_EDGES|MESH_UPDATE_FACES|MESH_CHECK );
    new_mesh.updateForUse();

    Debug( 4015 ) << "[Mesh<Shape,T>::createSubmesh] stop\n";
}


template<typename Shape, typename T>
typename Mesh<Shape, T>::P1_mesh_ptrtype
Mesh<Shape, T>::createP1mesh() const
{

    P1_mesh_ptrtype new_mesh( new P1_mesh_type );


    // How the nodes on this mesh will be renumbered to nodes
    // on the new_mesh.
    std::vector<size_type> new_node_numbers (this->numPoints());
    std::vector<size_type> new_vertex (this->numPoints());

    std::fill ( new_node_numbers.begin(),
                new_node_numbers.end(),
                invalid_size_type_value );

    std::fill ( new_vertex.begin(),
                new_vertex.end(),
                0 );



    // the number of nodes on the new mesh, will be incremented
    unsigned int n_new_nodes = 0;
    unsigned int n_new_elem  = 0;
    size_type n_new_faces = 0;

    // inherit the table of markersName
    BOOST_FOREACH( auto itMark, this->markerNames() )
        {
            new_mesh->addMarkerName( itMark.first,itMark.second.template get<0>(),itMark.second.template get<1>() );
        }

    auto it = this->beginElementWithProcessId( this->comm().rank() );
    auto en = this->endElementWithProcessId( this->comm().rank() );
    for ( ; it != en; ++it )
        {
            element_type const& old_elem = *it;

            // create a new element
            typename P1_mesh_type::element_type new_elem;

            // get element markers
            new_elem.setMarker(old_elem.marker().value());
            new_elem.setMarker2(old_elem.marker2().value());
            new_elem.setMarker3(old_elem.marker3().value());

            // Loop over the P1 nodes on this element.
            for (unsigned int n=0; n < element_type::numVertices; n++)
                {
                    //!!!!!FEEL_ASSERT (old_elem.point( n ).id() < new_node_numbers.size()).error( "invalid point id()" );

                    if ( new_node_numbers[old_elem.point(n).id()] == invalid_size_type_value )
                        {
                            new_node_numbers[old_elem.point(n).id()] = n_new_nodes;

                            Debug( 4015 ) << "[Mesh<Shape,T>::createP1mesh] insert point " << old_elem.point(n) << "\n";

                            typename P1_mesh_type::point_type pt( old_elem.point(n) );
                            pt.setId( n_new_nodes);

                            // Add this node to the new mesh
                            new_mesh->addPoint(pt);

                            Debug( 4015 ) << "[Mesh<Shape,T>::createSubmesh] number of  points " << new_mesh->numPoints() << "\n";

                            // Increment the new node counter
                            n_new_nodes++;

                            if ( n < element_type::numVertices ) //???
                                {
                                    FEEL_ASSERT( new_vertex[old_elem.point(n).id()] == 0 ).error( "already seen this point?" );
                                    new_vertex[old_elem.point(n).id()]=1;
                                }
                        }

                    // Define this element's connectivity on the new mesh
                    FEEL_ASSERT (new_node_numbers[old_elem.point(n).id()] < new_mesh->numPoints()).error("invalid connectivity");

                    Debug( 4015 ) << "[Mesh<Shape,T>::createP1mesh] adding point old(" << old_elem.point(n).id()
                                  << ") as point new(" << new_node_numbers[old_elem.point(n).id()]
                                  << ") in element " << new_elem.id() << "\n";

                    new_elem.setPoint(n, new_mesh->point( new_node_numbers[old_elem.point(n).id()] ) );

                } // end for n

            // set id of element
            new_elem.setId ( n_new_elem );

            // increment the new element counter
            n_new_elem++;

            // Add an equivalent element type to the new_mesh
            new_mesh->addElement( new_elem );


            /////////////////////
            // Maybe add faces for this element
            for (unsigned int s=0; s<old_elem.numTopologicalFaces; s++)
                {
                    if ( !old_elem.facePtr(s) ) continue;

                    // only add face on the boundary: they have some data
                    // (boundary ids) which cannot be retrieved otherwise
                    //if ( old_elem.neighbor(s) == invalid_size_type_value )
                    size_type global_face_id = old_elem.face(s).id();
                    if ( this->hasFace( global_face_id ) )
                    {

                        // get the corresponding face
                        face_type const& old_face = old_elem.face(s);
                        typename P1_mesh_type::face_type new_face;

                        // disconnect from elements of old mesh,
                        // the connection will be redone in
                        // \c updateForUse()
                        new_face.disconnect();

                        if (old_face.isOnBoundary()) new_face.setOnBoundary( true );
                        else new_face.setOnBoundary( false );

                        new_face.setMarker(old_face.marker().value());
                        new_face.setMarker2(old_face.marker2().value());
                        new_face.setMarker2(old_face.marker3().value());

                        // update P1 points info
                        for ( uint16_type p = 0;p < face_type::numVertices; ++p )
                            {
                                new_face.setPoint( p, new_mesh->point( new_node_numbers[old_elem.point(old_elem.fToP(s,p)).id()] ) );
                            }
                        new_face.setId( n_new_faces++ );

                        // add it to the list of faces
                        new_mesh->addFace( new_face );
                    }
                }

        } // end for it

    new_mesh->setNumVertices( std::accumulate( new_vertex.begin(), new_vertex.end(), 0 ) );

    Debug( 4015 ) << "[Mesh<Shape,T>::createP1mesh] update face/edge info if necessary\n";
    // Prepare the new_mesh for use
    new_mesh->components().set ( MESH_RENUMBER|MESH_UPDATE_EDGES|MESH_UPDATE_FACES|MESH_CHECK );
    new_mesh->updateForUse();










    return new_mesh;
}



} // Feel


//#if !defined(FEEL_INSTANTIATION_MODE)
# include <feel/feeldiscr/meshimpl.hpp>
//#endif //

#endif /* __mesh_H */

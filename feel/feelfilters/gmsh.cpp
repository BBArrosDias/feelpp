/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=tcl:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-02-10

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2008 Université Joseph Fourier (Grenoble I)

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
   \file gmsh.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-02-10
 */
#include <cstdlib>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

#include <boost/regex.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/concept_check.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include <feel/feelcore/feel.hpp>
#include <feel/feelcore/application.hpp>
#include <feel/feelmesh/hypercube.hpp>
#include <feel/feelfilters/gmsh.hpp>
#include <feel/feelfilters/gmshsimplexdomain.hpp>
#include <feel/feelfilters/gmshhypercubedomain.hpp>
#include <feel/feelfilters/gmshellipsoiddomain.hpp>

namespace Feel
{
namespace fs = boost::filesystem;

const char* FEEL_GMSH_FORMAT_VERSION = "2.1";

Gmsh::Gmsh( int nDim, int nOrder )
    :
    M_dimension( nDim ),
    M_order( nOrder ),
    M_version( FEEL_GMSH_FORMAT_VERSION ),
    M_I( nDim ),
    M_h( 0.1 ),
    M_addmidpoint( true ),
    M_usePhysicalNames( false )
{
    this->setReferenceDomain();
}
Gmsh::Gmsh( Gmsh const & __g )
    :
    M_dimension( __g.M_dimension ),
    M_order( __g.M_order ),
    M_version( __g.M_version ),
    M_I( __g.M_I ),
    M_h( __g.M_h ),
    M_addmidpoint( __g.M_addmidpoint ),
    M_usePhysicalNames( __g.M_usePhysicalNames )
{}
Gmsh::~Gmsh()
{}

boost::shared_ptr<Gmsh>
Gmsh::New( po::variables_map const& vm )
{
    std::ostringstream ostr;
    ostr << vm["convex"].as<std::string>() << "(" << vm["dim"].as<int>() << "," << vm["order"].as<int>() << ")";
    boost::shared_ptr<Gmsh> gmsh_ptr( Gmsh::Factory::type::instance().createObject( ostr.str() ) );
    return gmsh_ptr;
}

boost::shared_ptr<Gmsh>
Gmsh::New( std::string const& shape, uint16_type d, uint16_type o, std::string const& ct )
{
    std::ostringstream ostr;
    if ( shape != "hypercube" && ct != "Hypercube" )
        ostr << shape << "(" << d << "," << o << ")";
    else
        ostr << shape << "(" << d << "," << o << "," << ct << ")";
    boost::shared_ptr<Gmsh> gmsh_ptr( Gmsh::Factory::type::instance().createObject( ostr.str() ) );
    return gmsh_ptr;
}

std::string
Gmsh::prefix( std::string const& __name, uint16_type __N ) const
{
    std::ostringstream __p;
    __p << __name << "-" << __N << "-" << this->order();

    return __p.str();
}
std::string
Gmsh::generateCircle( std::string const& __name, double __h )
{
    uint16_type N=( uint16_type )(1./__h);

    Debug(10000) << "prefix file name: " << __name << "\n";
    Debug(10000) << "h: " << __h << "\n";
    Debug(10000) << "N: " << N << "\n";

    // generate geo
    std::ostringstream __geoname;
    __geoname << this->prefix( __name, N ) << ".geo";
    fs::path __path( __geoname.str() );
    if ( !fs::exists( __path ) )
        {
            Debug(10000) << "generating: " << __geoname.str() << "\n";
            std::ofstream __geo( __geoname.str().c_str() );
            __geo << "Mesh.MshFileVersion = 2;\n"
                  << "h=" << __h << ";\n"
                  << "Point(1) = {0.0,0.0,0.0,h};\n"
                  << "Point(2) = {1,0.0,0.0,h};\n"
                  << "Point(3) = {0.2,0.0,0.0,h};\n"
                  << "Point(4) = {0,1,0,h};\n"
                  << "Point(5) = {0,0.2,0,h};\n"
                  << "Point(6) = {0,-0.2,0,h};\n"
                  << "Point(7) = {0,-1,0,h};\n"
                  << "Point(8) = {-0.2,0,0,h};\n"
                  << "Point(9) = {-1,0,0,h};\n"
                  << "Circle(1) = {2,1,4};\n"
                  << "Circle(2) = {4,1,9};\n"
                  << "Circle(3) = {9,1,7};\n"
                  << "Circle(4) = {7,1,2};\n"
                  << "Circle(5) = {3,1,5};\n"
                  << "Circle(6) = {5,1,8};\n"
                  << "Circle(7) = {8,1,6};\n"
                  << "Circle(8) = {6,1,3};\n"
                  << "Line Loop(9) = {1,2,3,4};\n"
                  << "Line Loop(10) = {6,7,8,5};\n"
                  << "Plane Surface(11) = {9,10};\n";

            __geo.close();
        }
    // generate mesh
    std::ostringstream __meshname;
    __meshname << this->prefix( __name, N ) << ".msh";
    Debug(10000) << "mesh file name: " << __meshname.str() << "\n";
    Debug(10000) << "does mesh file name exists ?: " << fs::exists(__meshname.str() ) << "\n";
    fs::path __meshpath( __meshname.str() );
    if ( !fs::exists( __meshpath ) )
        {
            Debug(10000) << "generating: " << __meshname.str() << "\n";
            generate( __geoname.str(), 2 );
        }
    return __meshname.str();
}
std::string
Gmsh::generateLine( std::string const& __name, double __h )
{
    uint16_type N=( uint16_type )(1./__h);

    Debug(10000) << "prefix file name: " << __name << "\n";
    Debug(10000) << "h: " << __h << "\n";
    Debug(10000) << "N: " << N << "\n";

    // generate geo
    std::ostringstream __geoname;
    __geoname << this->prefix( __name, N ) << ".geo";
    fs::path __path( __geoname.str() );
    if ( !fs::exists( __path ) )
        {
            Debug(10000) << "generating: " << __geoname.str() << "\n";
            std::ofstream __geo( __geoname.str().c_str() );
            __geo << "Mesh.MshFileVersion = 2;\n"
                  << "h=" << __h << ";\n"
                  << "Point(1) = {0.0,0.0,0.0,h};\n"
                  << "Point(2) = {1.0,0,0.0,h};\n"
                  << "Line(1) = {1,2};\n";
            __geo.close();
        }
    // generate mesh
    std::ostringstream __meshname;
    __meshname << this->prefix( __name, N ) << ".msh";
    Debug(10000) << "mesh file name: " << __meshname.str() << "\n";
    Debug(10000) << "does mesh file name exists ?: " << fs::exists(__meshname.str() ) << "\n";
    fs::path __meshpath( __meshname.str() );
    if ( !fs::exists( __meshpath ) )
        {
            Debug(10000) << "generating: " << __meshname.str() << "\n";
            generate( __geoname.str(), 1 );
        }
    return __meshname.str();
}
std::string
Gmsh::generateSquare( std::string const& __name, double __h )
{
    uint16_type N=( uint16_type )(1./__h);

    Debug(10000) << "prefix file name: " << __name << "\n";
    Debug(10000) << "h: " << __h << "\n";
    Debug(10000) << "N: " << N << "\n";

    // generate geo
    std::ostringstream __geoname;
    __geoname << this->prefix( __name, N ) << ".geo";
    fs::path __path( __geoname.str() );
    if ( !fs::exists( __path ) )
        {
            Debug(10000) << "generating: " << __geoname.str() << "\n";
            std::ofstream __geo( __geoname.str().c_str() );
            __geo << "Mesh.MshFileVersion = 2;\n"
                  << "h=" << __h << ";\n"
                  << "Point(1) = {0.0,0.0,0.0,h};\n"
                  << "Point(2) = {0.0,1,0.0,h};\n"
                  << "Point(3) = {1,1,0.0,h};\n"
                  << "Point(4) = {1,0,0.0,h};\n"
                  << "Line(1) = {1,4};\n"
                  << "Line(2) = {4,3};\n"
                  << "Line(3) = {3,2};\n"
                  << "Line(4) = {2,1};\n"
                  << "Line Loop(5) = {3,4,1,2};\n"
                  << "Plane Surface(6) = {5};\n"
                  << "Physical Line(10) = {1,2,4};\n"
                  << "Physical Line(20) = {3};\n"
                  << "Physical Surface(30) = {6};\n";
            __geo.close();
        }
    // generate mesh
    std::ostringstream __meshname;
    __meshname << this->prefix( __name, N ) << ".msh";
    Debug(10000) << "mesh file name: " << __meshname.str() << "\n";
    Debug(10000) << "does mesh file name exists ?: " << fs::exists(__meshname.str() ) << "\n";
    fs::path __meshpath( __meshname.str() );
    if ( !fs::exists( __meshpath ) )
        {
            Debug(10000) << "generating: " << __meshname.str() << "\n";
            generate( __geoname.str(), 2 );
        }
    return __meshname.str();
}

std::string
Gmsh::generateCube( std::string const& __name, double __h )
{
    uint16_type N=( uint16_type )(1./__h);

    Debug(10000) << "prefix file name: " << __name << "\n";
    Debug(10000) << "h: " << __h << "\n";
    Debug(10000) << "N: " << N << "\n";

    // generate geo
    std::ostringstream __geoname;
    __geoname << this->prefix( __name, N ) << ".geo";
    fs::path __path( __geoname.str() );
    if ( !fs::exists( __path ) )
        {
            Debug(10000) << "generating: " << __geoname.str() << "\n";
            std::ofstream __geo( __geoname.str().c_str() );
            __geo << "Mesh.MshFileVersion = 2;\n"
                  << "h=" << __h << ";\n"
                  << "Point(1) = {0.0,0.0,0.0,h};\n"
                  << "Point(2) = {0.0,1,0.0,h};\n"
                  << "Point(3) = {1,1,0.0,h};\n"
                  << "Point(4) = {1,0,0.0,h};\n"
                  << "Line(1) = {1,4};\n"
                  << "Line(2) = {4,3};\n"
                  << "Line(3) = {3,2};\n"
                  << "Line(4) = {2,1};\n"
                  << "Line Loop(5) = {3,4,1,2};\n"
                  << "Plane Surface(6) = {5};\n"
                  << "Extrude Surface {6, {0.0,0.0,1.0}};\n"
                  << "Physical Surface(10) = {19,27,15,23,6};\n"
                  << "Physical Surface(20) = {28};\n"
                  << "Surface Loop(31) = {28,15,-6,19,23,27};\n"
                  << "\n"
                  << "// volume \n"
                  << "Volume(1) = {31};\n"
                  << "Physical Volume(2) = {1};\n";
            __geo.close();
        }
    // generate mesh
    std::ostringstream __meshname;
    __meshname << this->prefix( __name, N ) << ".msh";
    Debug(10000) << "mesh file name: " << __meshname.str() << "\n";
    Debug(10000) << "does mesh file name exists ?: " << fs::exists(__meshname.str() ) << "\n";
    fs::path __meshpath( __meshname.str() );
    if ( !fs::exists( __meshpath ) )
        {
            Debug(10000) << "generating: " << __meshname.str() << "\n";
            generate( __geoname.str(), 3 );
        }
    return __meshname.str();
}

std::string
Gmsh::getDescriptionFromFile( std::string const& file ) const
{
    if ( !fs::exists( file ) )
    {
        std::ostringstream ostr;
        ostr << "File " << file << " does not exist";
        throw std::invalid_argument( ostr.str() );
    }

    std::ifstream __geoin( file.c_str() );

    std::ostringstream __geostream;
    std::istreambuf_iterator<char> src(__geoin.rdbuf());
    std::istreambuf_iterator<char> end;
    std::ostream_iterator<char> dest(__geostream);

    std::copy(src,end,dest);

    __geoin.close();
    return  __geostream.str();
}
bool
Gmsh::generateGeo( std::string const& __name, std::string const& __geo ) const
{
    boost::regex regex( "(?:(lc|h))[[:blank:]]*=[[:blank:]]*[+-]?(?:(?:(?:[[:digit:]]*\\.)?[[:digit:]]+(?:[eE][+-]?[[:digit:]]+)?));" );
    std::ostringstream hstr;
    hstr << "(?1$1) = " << M_h << ";";

    std::string _geo = boost::regex_replace( __geo, regex, hstr.str(), boost::match_default | boost::format_all);
    // generate geo
    std::ostringstream __geoname;
    __geoname << __name << ".geo";
    fs::path __path( __geoname.str() );
    bool geochanged = false;
    if ( !fs::exists( __path ) )
        {
            Debug(10000) << "generating: " << __geoname.str() << "\n";
            std::ofstream __geofile( __geoname.str().c_str() );
            __geofile << _geo;
            __geofile.close();
            geochanged = true;
        }
    else
        {
            std::string s = this->getDescriptionFromFile( __geoname.str() );
            if ( s != _geo )
                {
                    std::ofstream __geofile( __geoname.str().c_str() );
                    __geofile << _geo;
                    __geofile.close();
                    geochanged = true;
                }
        }

    return geochanged;
}
std::string
Gmsh::generate( std::string const& name ) const
{
    std::string descr = this->getDescription();
    return this->generate( name, descr );
}

std::string
Gmsh::generate( std::string const& __name, std::string const& __geo, bool const __forceRebuild, bool const parametric ) const
{
    std::string fname;
    if ( !mpi::environment::initialized() || (mpi::environment::initialized()  && M_comm.rank() == 0 ) )
        {
            bool geochanged (generateGeo(__name,__geo));
            std::ostringstream __geoname;
            __geoname << __name << ".geo";

            // generate mesh
            std::ostringstream __meshname;
            __meshname << __name << ".msh";
            Debug(10000) << "mesh file name: " << __meshname.str() << "\n";
            Debug(10000) << "does mesh file name exists ?: " << fs::exists(__meshname.str() ) << "\n";
            fs::path __meshpath( __meshname.str() );
            if ( geochanged || __forceRebuild || !fs::exists( __meshpath ) )
                {
                    Debug(10000) << "generating: " << __meshname.str() << "\n";
                    if ( __geo.find( "Volume" ) != std::string::npos )
                        generate( __geoname.str(), 3, parametric );
                    else if ( __geo.find( "Surface" ) != std::string::npos )
                        generate( __geoname.str(), 2, parametric );
                    else if ( __geo.find( "Line" )  != std::string::npos )
                        generate( __geoname.str(), 1, parametric );
                    else
                        generate( __geoname.str(), 3, parametric );
                }
            Debug( 10000 ) << "[Gmsh::generate] meshname = " << __meshname.str() << "\n";
            fname=__meshname.str();
        }
    if ( mpi::environment::initialized() )
        mpi::broadcast( M_comm, fname, 0 );
    return fname;
}
void
Gmsh::refine( std::string const& name, int level, bool parametric  ) const
{
#if HAVE_GMSH
    // generate mesh
    std::ostringstream __str;
    if ( parametric )
        __str << "gmsh -parametric -refine " << name;
    else
        __str << "gmsh -refine " << name;
    ::system( __str.str().c_str() );
#else
    throw std::invalid_argument("Gmsh is not available on this system");
#endif
}
void
Gmsh::generate( std::string const& __geoname, uint16_type dim, bool parametric  ) const
{
#if HAVE_GMSH
    // generate mesh
    std::ostringstream __str;
    //__str << "gmsh -algo tri -" << dim << " " << "-order " << this->order() << " " << __geoname;
    if ( parametric )
        __str << "gmsh -parametric -" << dim << " " << __geoname;
    else
        __str << "gmsh -" << dim << " " << __geoname;
    ::system( __str.str().c_str() );
#else
    throw std::invalid_argument("Gmsh is not available on this system");
#endif
}
std::string
Gmsh::preamble() const
{
    std::ostringstream ostr;

    ostr << "Mesh.MshFileVersion = " << this->version() << ";\n"
         << "Mesh.CharacteristicLengthExtendFromBoundary=1;\n"
         << "Mesh.CharacteristicLengthFromPoints=1;\n"
         << "Mesh.ElementOrder=" << M_order << ";\n"
         << "Mesh.SecondOrderIncomplete = 0;\n"
         << "Mesh.Algorithm = 6;\n"
         << "h=" << M_h << ";\n";
    return ostr.str();
}

/// \cond detail
namespace detail {

struct HypercubeDomain
{
    HypercubeDomain( int _Dim, int _Order )
        :
        Dim( _Dim ), Order( _Order ), RDim( _Dim ), Hyp( false )
        {}
    HypercubeDomain( int _Dim, int _Order, int _RDim, std::string const& hyp )
        :
        Dim( _Dim ), Order( _Order ), RDim( _RDim ), Hyp( hyp == "Hypercube" )
        {}
    Gmsh* operator()() { return new GmshHypercubeDomain(Dim, Order, RDim, Hyp); }
    int Dim, Order,RDim;
    bool Hyp;
};

struct SimplexDomain
{
    SimplexDomain( int _Dim, int _Order )
        :
        Dim( _Dim ), Order( _Order )
        {}
    Gmsh* operator()() { return new GmshSimplexDomain(Dim, Order); }
    int Dim, Order;
};

struct EllipsoidDomain
{
    EllipsoidDomain( int _Dim, int _Order )
        :
        Dim( _Dim ), Order( _Order )
        {}
    Gmsh* operator()() { return new GmshEllipsoidDomain(Dim, Order); }
    int Dim, Order;
};

} // detail



# define DIMS BOOST_PP_TUPLE_TO_LIST(3,(1,2,3))
# define ORDERS BOOST_PP_TUPLE_TO_LIST(5,(1,2,3,4,5))
# define SHAPES1 BOOST_PP_TUPLE_TO_LIST(3, ((2,(simplex, Simplex)) ,    \
                                            (2,(ellipsoid, Ellipsoid)) , \
                                            (2,(hypercube, Hypercube)) ))
# define SHAPES2 BOOST_PP_TUPLE_TO_LIST(2, ((3,(hypercube, Hypercube, Simplex)), \
                                            (3,(hypercube, Hypercube, Hypercube)) ) )


#define FACTORY1NAME( LDIM, LORDER, LSHAPE )                             \
    BOOST_PP_STRINGIZE(BOOST_PP_ARRAY_ELEM(0,LSHAPE) BOOST_PP_LPAREN() LDIM BOOST_PP_COMMA() LORDER BOOST_PP_RPAREN())

# define FACTORY1(LDIM,LORDER,LSHAPE )                                   \
const bool BOOST_PP_CAT( BOOST_PP_CAT( BOOST_PP_CAT( mesh, LDIM ), LORDER), BOOST_PP_ARRAY_ELEM(1,LSHAPE))  = \
                           Gmsh::Factory::type::instance().registerProduct( boost::algorithm::erase_all_copy( std::string( FACTORY1NAME(LDIM, LORDER, LSHAPE ) ), " " ), \
                                                                            *new detail::BOOST_PP_CAT(BOOST_PP_ARRAY_ELEM(1,LSHAPE),Domain)(LDIM,LORDER) );

# define FACTORY1_OP(_, GDO) FACTORY1 GDO

#define FACTORY2NAME( LDIM, LORDER, LSHAPE )                             \
    BOOST_PP_STRINGIZE(BOOST_PP_ARRAY_ELEM(0,LSHAPE) BOOST_PP_LPAREN() LDIM BOOST_PP_COMMA() LORDER BOOST_PP_COMMA() BOOST_PP_ARRAY_ELEM(2,LSHAPE) BOOST_PP_RPAREN())

# define FACTORY2(LDIM,LORDER,LSHAPE )                                   \
const bool BOOST_PP_CAT( BOOST_PP_CAT( BOOST_PP_CAT( BOOST_PP_CAT( mesh, LDIM ), LORDER), BOOST_PP_ARRAY_ELEM(1,LSHAPE)), BOOST_PP_ARRAY_ELEM(2,LSHAPE))   = \
                           Gmsh::Factory::type::instance().registerProduct( boost::algorithm::erase_all_copy( std::string( FACTORY2NAME(LDIM, LORDER, LSHAPE ) ), " " ), \
                                                                            *new detail::BOOST_PP_CAT(BOOST_PP_ARRAY_ELEM(1,LSHAPE),Domain)(LDIM,LORDER,LDIM,BOOST_PP_STRINGIZE(BOOST_PP_ARRAY_ELEM(2,LSHAPE))) );

# define FACTORY2_OP(_, GDO) FACTORY2 GDO

// only up to 4 for mesh data structure nit supported for higher order in Gmsh
BOOST_PP_LIST_FOR_EACH_PRODUCT(FACTORY1_OP, 3, (DIMS, ORDERS, SHAPES1))
BOOST_PP_LIST_FOR_EACH_PRODUCT(FACTORY2_OP, 3, (DIMS, ORDERS, SHAPES2))

const bool meshs213s = Gmsh::Factory::type::instance().registerProduct( "hypercube(2,1,3,Simplex)", *new detail::HypercubeDomain( 2, 1, 3, "Simplex" ) );
const bool meshs213ts = Gmsh::Factory::type::instance().registerProduct( "hypercube(2,1,3,Hypercube)", *new detail::HypercubeDomain( 2, 1, 3, "Hypercube" ) );
const bool meshs112s = Gmsh::Factory::type::instance().registerProduct( "hypercube(1,1,2,Simplex)", *new detail::HypercubeDomain( 1, 1, 2, "Simplex" ) );
const bool meshs112ts = Gmsh::Factory::type::instance().registerProduct( "hypercube(1,1,2,Hypercube)", *new detail::HypercubeDomain( 1, 1, 2, "Hypercube" ) );

/// \endcond detail
} // Feel

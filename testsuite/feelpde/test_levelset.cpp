/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t
   -*- vim: set ft=cpp fenc=utf-8 sw=4 ts=4 sts=4 tw=80 et cin cino=N-s,c0,(0,W4,g0:

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
       Date: 2008-02-07

  Copyright (C) 2008-2012 Universite Joseph Fourier (Grenoble I)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
   \file dist2wallsoptimized.cpp
   \author Guillaume Dolle <gdolle at unistra.fr>
   \date 2014-01-21
 */

//#define USE_BOOST_TEST 1
#if defined(USE_BOOST_TEST)
#define BOOST_TEST_MODULE levelset
#include <testsuite/testsuite.hpp>
#endif

#include <feel/feelfilters/exporter.hpp>
#include <feel/feeldiscr/pch.hpp>
#include <feel/feeldiscr/pdh.hpp>
#include <feel/feelfilters/loadmesh.hpp>
#include <feel/feelpde/reinit_fms.hpp>
#include <feel/feelvf/vf.hpp>
#include <feel/feelfilters/domain.hpp>

using namespace Feel;

inline
AboutData
makeAbout()
{
    AboutData about( "test_levelset" ,
                     "test_levelset" ,
                     "0.1",
                     "test levelset",
                     Feel::AboutData::License_GPL,
                     "Copyright (c) 2015 Feel++ Consortium" );

    about.addAuthor( "Guillaume Dolle", "developer", "gdolle@unistra.fr", "" );
    about.addAuthor( "Vincent Doyeux", "developer", "vincent.doyeux@ujf-grenoble.fr", "" );

    return about;
}

/// Test the reinitialisation method (Fast Marching, ...)
///     \tparam DIM         Topological dimension.
///     \tparam H_ORDER     Space polynomial order..
///     \tparam G_ORDER     Geometrical polynomial order.
template<int DIM, int H_ORDER, int G_ORDER>
class TestLevelSet
{
public:
    using mesh_type = Mesh< Simplex<DIM,G_ORDER> >;
    using mesh_ptrtype = boost::shared_ptr< mesh_type >;

    /// Init the geometry with a circle/sphere from radius and characteristic length
    ///     \param radius   Circle or sphere radius.
    ///     \param h        Mesh size.
    TestLevelSet( double radius=1.0, double h=0.1 ) :
        M_mesh( createGMSHMesh( _mesh=new mesh_type,
                                _desc=domain( _name="ellipsoid_nd",
                                              _shape="ellipsoid",
                                              _dim=DIM,
                                              _xmin=-radius,
                                              _ymin=-radius,
                                              _zmin=-radius,
                                              _xmax=radius,
                                              _ymax=radius,
                                              _zmax=radius,
                                              _h= h ),
                                _update=MESH_CHECK|MESH_UPDATE_FACES|MESH_UPDATE_EDGES
                              ) ),
        M_radius( radius )
    {}

    /// First method: let the FMM search for the elements crossed by the interface
    /// and the maximum distance is checked for a circle/sphere.
    void wallDist_1()
    {
        auto Xh = Pch<H_ORDER>(M_mesh);
        auto thefms = fms( Xh );
        auto phio = Xh->element();
        phio = vf::project(Xh, elements(M_mesh), h() );
        phio.on( _range=boundaryfaces(M_mesh), _expr= -h()/100. );
        auto phi = thefms->march(phio);

#if defined(USE_BOOST_TEST)
        // Max error around mesh size h.
        BOOST_CHECK_CLOSE( phi.max(), M_radius, h() );
#else

        VLOG(2) << "phi_max" << phi.max();
        auto exp = exporter(_mesh=M_mesh, _name="testsuite_levelset_distw1");
        exp->step(0)->add("phio", phio);
        exp->step(0)->add("phi", phi);
        exp->save();
#endif
    }

    /// Second method: give to the FMM the elements having the good value to start with,
    /// and the maximum distance is checked for a circle/sphere.
    void wallDist_2()
    {
        auto Xh = Pch<H_ORDER>(M_mesh);
        auto Xh0 = Pdh<0>(M_mesh);
        auto thefms = fms( Xh );
        auto phio = Xh->element();
        auto mark = Xh0->element();

        phio.on( _range=boundaryelements(M_mesh), _expr=h() );
        phio.on( _range=boundaryfaces(M_mesh), _expr=cst(0) );

        mark.on( _range=boundaryelements(M_mesh), _expr=cst(1) );
        M_mesh->updateMarker2( mark );

        auto phi = thefms->march(phio, true);

#if defined(USE_BOOST_TEST)
        // Max error around mesh size h.
        BOOST_CHECK_CLOSE( phi.max(), M_radius, h() );
#else
        VLOG(2) << "phi_max" << phi.max();
        auto exp = exporter(_mesh=M_mesh, _name="testsuite_levelset_distw2");
        exp->step(0)->add("mark", mark);
        exp->step(0)->add("phio", phio);
        exp->step(0)->add("phi", phi);
        exp->save();
#endif
    }

private:
    mesh_ptrtype M_mesh;
    double M_radius;

};

#if defined(USE_BOOST_TEST)
FEELPP_ENVIRONMENT_WITH_OPTIONS( makeAbout(), feel_options() );
BOOST_AUTO_TEST_SUITE( levelset )

// Test wall distance on :
// Unit 2D circle P1G1.
BOOST_AUTO_TEST_CASE( test_2d_p1g1 )
{
    TestLevelSet<2,1,1> tls;
    tls.wallDist_1();
}

// Unit 3D sphere P1G1.
BOOST_AUTO_TEST_CASE( test_3d_p1g1 )
{
    TestLevelSet<3,1,1> tls;
    tls.wallDist_1();
}

// Unit 2D circle P1G2.
BOOST_AUTO_TEST_CASE( test_2d_p1g2 )
{
    TestLevelSet<2,1,2> tls;
    tls.wallDist_1();
}

// Unite 3D sphere P1G2.
BOOST_AUTO_TEST_CASE( test_3D_p1g2 )
{
    TestLevelSet<3,1,2> tls;
    tls.wallDist_1();
}

BOOST_AUTO_TEST_SUITE_END()

#else
int main(int argc, char** argv )
{
    Feel::Environment env( _argc=argc, _argv=argv,
                           _about=makeAbout(),
                           _desc=feel_options() );
#if 0
    TestLevelSet<2,1,1> tls;
    TestLevelSet<3,1,1> tls;
    TestLevelSet<2,1,1> tls( 10, 0.1 );
    TestLevelSet<3,1,1> tls( 10, 1 );
#endif
    //TestLevelSet<3,1,1> tls( 1, 0.1 );
    TestLevelSet<2,1,2> tls( 1, 0.1 );
    //TestLevelSet<2,1,1> tls( 1, 0.1 );
    tls.wallDist_1();
    tls.wallDist_2();

    return 0;
}
#endif

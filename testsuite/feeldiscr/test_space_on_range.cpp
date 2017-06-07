/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*-

 This file is part of the Feel++ library

 Author(s): Vincent Chabannes <vincent.chabannes@feelpp.org>
 Date: 7 june 2017

 Copyright (C) 2017 Feel++ Consortium

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

// give a name to the testsuite
#define BOOST_TEST_MODULE function space on range testsuite

#include <testsuite/testsuite.hpp>

#include <feel/feeldiscr/functionspace.hpp>
#include <feel/feelfilters/loadmesh.hpp>
#include <feel/feelfilters/exporter.hpp>
#include <feel/feelmesh/concatenate.hpp>


FEELPP_ENVIRONMENT_NO_OPTIONS

BOOST_AUTO_TEST_SUITE( space_on_range )

BOOST_AUTO_TEST_CASE( test_2d )
{
    using namespace Feel;
    typedef Mesh<Simplex<2,1>> mesh_type;
    auto mesh = loadMesh(_mesh=new mesh_type );

    typedef FunctionSpace<mesh_type,bases<Lagrange<2,Scalar> > > space_type;

    auto r1 = elements(mesh, pow(Px()-0.4,2)+pow(Py()-0.5,2) < pow(cst(0.23),2) );
    auto r2 = elements(mesh, pow(Px()-0.7,2)+pow(Py()-0.5,2) < pow(cst(0.15),2) );
    auto therange = concatenate(r1,r2);

    auto VhPS = space_type::New(_mesh=mesh,_range=therange);
    BOOST_TEST_MESSAGE( "VhPS->nDof() : " << VhPS->nDof() );
    auto VhFS = space_type::New(_mesh=mesh);
    BOOST_TEST_MESSAGE( "VhFS->nDof() : " << VhFS->nDof());

    // test element of space
    auto chiShapeFS = VhFS->element();
    chiShapeFS.setConstant(5.);
    chiShapeFS.on(_range=therange,_expr=cst(3.));
    auto chiShapePS = VhPS->element();
    chiShapePS.setConstant(3.);
    double err = normL2(_range=therange,_expr=idv(chiShapeFS)-idv(chiShapePS));
    BOOST_CHECK_SMALL( err,1e-12 );
    chiShapeFS.on(_range=therange,_expr=Px()*Py());
    chiShapePS.on(_range=therange,_expr=Px()*Py());
    err = normL2(_range=therange,_expr=idv(chiShapeFS)-idv(chiShapePS));
    BOOST_CHECK_SMALL( err,1e-12 );
    double chiFSnorm2 = chiShapeFS.l2Norm();
    double chiPSnorm2 = chiShapePS.l2Norm();
    BOOST_CHECK( chiFSnorm2 > chiPSnorm2 );

    // test a laplacian solve
    auto u = VhPS->element("u");
    auto v = VhPS->element("v");
    BOOST_CHECK( VhPS->dof()->hasMeshSupport() );
    VhPS->dof()->meshSupport()->updateBoundaryFaces();
    auto myboundaryfaces = VhPS->dof()->meshSupport()->rangeBoundaryFaces();
    auto l = form1( _test=VhPS );
    l = integrate(_range=therange,
                  _expr=id(v));
    auto a = form2( _trial=VhPS, _test=VhPS);
    a = integrate(_range=therange,
                  _expr=gradt(u)*trans(grad(v)) );
    a+=on(_range=myboundaryfaces, _rhs=l, _element=u, _expr=cst(0.) );
    a.solve(_rhs=l,_solution=u);

    // export results
    auto e = exporter( _mesh=mesh,_name="test2d" );
    e->addRegions();
    e->add( "u", u );
    e->add( "chi-partial-support", chiShapeFS );
    e->add( "chi-full-support", chiShapePS );
    e->save();
}

BOOST_AUTO_TEST_CASE( test_composite_2d )
{
    using namespace Feel;
    typedef Mesh<Simplex<2,1>> mesh_type;
    auto mesh = loadMesh(_mesh=new mesh_type );

    typedef FunctionSpace<mesh_type,bases<Lagrange<2,Vectorial>,Lagrange<2,Scalar> > > space_type;

    auto r1 = elements(mesh, pow(Px()-0.4,2)+pow(Py()-0.5,2) < pow(cst(0.23),2) );
    auto r2 = elements(mesh, pow(Px()-0.7,2)+pow(Py()-0.5,2) < pow(cst(0.15),2) );
    auto therange = concatenate(r1,r2);
    auto VhPS1 = space_type::New(_mesh=mesh,_range=therange);
    BOOST_TEST_MESSAGE( "VhPS1->nDof() : " << VhPS1->nDof() );

    auto supp1 = std::make_shared<MeshSupport<mesh_type>>(mesh,r1);
    auto supp2 = std::make_shared<MeshSupport<mesh_type>>(mesh,r2);
    auto VhPS2 = space_type::New(_mesh=mesh,_range=fusion::make_vector(supp1,supp2));
    BOOST_TEST_MESSAGE( "VhPS2->nDof() : " << VhPS2->nDof() );

    auto VhFS = space_type::New(_mesh=mesh);
    BOOST_TEST_MESSAGE( "VhFS->nDof() : " << VhFS->nDof() );

    auto fieldFS_U = VhFS->element();
    auto fieldFS_u = fieldFS_U.element<0>();
    auto fieldFS_p = fieldFS_U.element<1>();
    fieldFS_u.on(_range=therange,_expr=3*one());
    fieldFS_p.on(_range=therange,_expr=cst(4.));
    auto fieldPS1_U = VhPS1->element();
    auto fieldPS1_u = fieldPS1_U.element<0>();
    auto fieldPS1_p = fieldPS1_U.element<1>();
    fieldPS1_u.setConstant(3.);
    fieldPS1_p.setConstant(4.);
    double err1_u = normL2(_range=therange,_expr=idv(fieldFS_u)-idv(fieldPS1_u));
    BOOST_CHECK_SMALL( err1_u,1e-12 );
    double err1_p = normL2(_range=therange,_expr=idv(fieldFS_p)-idv(fieldPS1_p));
    BOOST_CHECK_SMALL( err1_p,1e-12 );

    fieldFS_U.zero();
    fieldFS_u.on(_range=r1,_expr=3*one());
    fieldFS_p.on(_range=r2,_expr=cst(4.));
    auto fieldPS2_U = VhPS2->element();
    auto fieldPS2_u = fieldPS2_U.element<0>();
    auto fieldPS2_p = fieldPS2_U.element<1>();
    fieldPS2_u.setConstant(3.);
    fieldPS2_p.setConstant(4.);
    double err2_u = normL2(_range=r1,_expr=idv(fieldFS_u)-idv(fieldPS2_u));
    BOOST_CHECK_SMALL( err2_u,1e-12 );
    double err2_p = normL2(_range=r2,_expr=idv(fieldFS_p)-idv(fieldPS2_p));
    BOOST_CHECK_SMALL( err2_p,1e-12 );
}
BOOST_AUTO_TEST_CASE( test_composite_meshes_list_2d )
{
    using namespace Feel;
    static const uint16_type nDim = 2;
    typedef Mesh<Simplex<nDim,1>> mesh_type;
    auto mesh = loadMesh(_mesh=new mesh_type );

    typedef Mesh<Simplex<nDim-1,1,nDim>> submesh_type;
    auto submesh = createSubmesh(mesh,boundaryfaces(mesh) );

    typedef FunctionSpace< meshes<mesh_type,submesh_type>,bases<Lagrange<2,Scalar>,Lagrange<2,Scalar> > > space_type;

    auto r1 = elements(mesh, pow(Px()-0.4,2)+pow(Py()-0.5,2) < pow(cst(0.23),2) );
    auto r2 = elements(submesh, Px()*Py()<cst(0.25) );

    auto supp1 = std::make_shared<MeshSupport<mesh_type>>(mesh,r1);
    auto supp2 = std::make_shared<MeshSupport<submesh_type>>(submesh,r2);
    auto VhPS = space_type::New(_mesh=fusion::make_vector(mesh,submesh),_range=fusion::make_vector(supp1,supp2));
    BOOST_TEST_MESSAGE( "VhPS->nDof() : " << VhPS->nDof() );

    auto VhFS = space_type::New(_mesh=fusion::make_vector(mesh,submesh));
    BOOST_TEST_MESSAGE( "VhFS->nDof() : " << VhFS->nDof() );

    auto fieldFS_U = VhFS->element();
    auto fieldFS_u1 = fieldFS_U.element<0>();
    auto fieldFS_u2 = fieldFS_U.element<1>();
    fieldFS_u1.on(_range=r1,_expr=cst(3.));
    fieldFS_u2.on(_range=r2,_expr=cst(4.));

    auto fieldPS_U = VhPS->element();
    auto fieldPS_u1 = fieldPS_U.element<0>();
    auto fieldPS_u2 = fieldPS_U.element<1>();
    fieldPS_u1.setConstant(3.);
    fieldPS_u2.setConstant(4.);
    double err_u1 = normL2(_range=r1,_expr=idv(fieldFS_u1)-idv(fieldPS_u1));
    BOOST_CHECK_SMALL( err_u1,1e-12 );
    double err_u2 = normL2(_range=r2,_expr=idv(fieldFS_u2)-idv(fieldPS_u2));
    BOOST_CHECK_SMALL( err_u2,1e-12 );
}
BOOST_AUTO_TEST_SUITE_END()


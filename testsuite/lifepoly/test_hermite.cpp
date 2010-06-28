/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2010-03-04

  Copyright (C) 2010 Universit� Joseph Fourier (Grenoble I)

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
/**
   \file testhermite.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2010-03-04
 */
#define BOOST_TEST_MODULE hermite polynomials test
// Boost.Test
#define USE_TEST 1
#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;
#include <life/lifepoly/polynomialset.hpp>
#include <life/lifepoly/orthonormalpolynomialset.hpp>
#include <test_hermite.hpp>

#if defined(USE_TEST )
BOOST_AUTO_TEST_SUITE( hermite_testsuite )

BOOST_AUTO_TEST_SUITE( hermite_simplex_testsuite )

BOOST_AUTO_TEST_CASE( hermite1 )
{
    BOOST_TEST_MESSAGE( "Hermite(1)" );
    TestHermite<fem::Hermite<1, 3, Scalar, real64_type, Simplex> > t; t();
}
BOOST_AUTO_TEST_CASE( hermite2 )
{
    BOOST_TEST_MESSAGE( "Hermite(2)" );
    TestHermite<fem::Hermite<2, 3, Scalar, real64_type, Simplex> > t2;t2();
}
BOOST_AUTO_TEST_SUITE_END() // hermite_1d_simplex_testsuite

BOOST_AUTO_TEST_SUITE_END()

#else

int main()
{
    TestHermite<fem::Hermite<1, 3, Scalar, real64_type, Simplex> > t;
    t();
}

#endif // USE_TEST


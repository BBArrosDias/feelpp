/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2009-04-23

  Copyright (C) 2009 Universit� Joseph Fourier (Grenoble I)

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
   \file lagrange.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2009-04-23
 */
#ifndef __plagrange_H
#define __plagrange_H 1

#include "basisgen.hpp"

namespace Life
{
namespace detail
{
template<typename Convex, uint16_type Order>
PolyvisBase*
createLagrange()
{
    std::cout << "Creating Lagrange(" << Convex::nDim << "," << Order << ")\n";
    return new Polyvis<basis_type<Lagrange<Order> >,convex_type<Convex> >;
}
# /* Generates code for all dim and order. */
# define LAGRANGE_FACTORY_OP(_, GDO)           \
    LAGRANGE_FACTORY GDO                        \
    /**/
#
# define GEN_LAGRANGE(LDIM,LORDER,CONVEX)              \
    "lagrange(" #LDIM "," #LORDER "," #CONVEX ")"       \
    /**/
#
#
#define LAGRANGE_FACTORY(LDIM,LORDER,CONVEX)                     \
    const bool BOOST_PP_CAT( BOOST_PP_CAT( BOOST_PP_CAT(lag, CONVEX), LDIM), LORDER ) = \
        PolyvisBase::factory_type::instance()                           \
        .registerProduct( GEN_LAGRANGE(LDIM,LORDER,CONVEX), &createLagrange<CONVEX< LDIM >,LORDER> );

} // detail
} // Life
#endif /* __lagrange_H */

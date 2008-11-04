/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2008-09-30

  Copyright (C) 2008 Universit� Joseph Fourier (Grenoble I)

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
   \file periodic.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2008-09-30
 */
#ifndef __Periodic_H
#define __Periodic_H 1

#include <life/lifealg/glas.hpp>


namespace Life
{

/**
 * \class Periodic
 * \brief Periodic class holder
 *
 the periodic condition is set both as template parameter for the
 functionspace and constructor parameter. In the former the tags that
 are periodic/linked together are set as well as the fact that we have
 a periodic condition. In the former we set the translation Trans that
 allow to link the dofs of tag2 and tag1.

 This data structure is then passed to the Dof class as well that will
 actually do the real work to treat the periodic condition. Its job is
 to set the same dof identifier for the dof on Tag2 as the Dof on Tag1
 with respect to the translation Trans. The 'periodic condition' check
 must be done for all entities of an element (vertex,edge,face,volume)
 even for the volume (think P0 discontinuous). During the check the
 dof points coordinates of Tag1 are put into a data structure DS as
 well as the corresponding dof identifier for rapid localisation then
 the dof points are translation by Trans and look for in DS. One and
 only one point must be found, then the dof identifier of the original
 point in Tag2 is set to the one in Tag1.

 Issues:

 - the dof are constructed by entity (vertex,edge,face,volume) so
 for each entity one must check that the dof points belongs to
 either Tag1 or Tag2. This is not yet clear how to do this.

 - the data structure DS must be efficient to find the proper point in
 Tag1 list, see sedgewick for reference.

 - the implementation of the periodic logic could be done either in
 dof or in periodic, with the former we have access to all information
 a priori but Dof get some added weight, with the latter we need to
 pass extra information and we make sure that the Dof stays more or
 less as it is. The other issue is that we must implement the same
 interface for all related periodic conditions.


 * @author Christophe Prud'homme
 * @see
 */
template<uint16_type Tag1, uint16_type Tag2, typename T = double >
class Periodic
{
public:

    /** @name Constants
     */
    //@{

    typedef typename node<T>::type node_type;

    //@}

    /** @name Constants
     */
    //@{

    static const bool is_periodic = true;
    static const uint16_type tag1 = Tag1;
    static const uint16_type tag2 = Tag2;

    //@}

    /** @name Constructors, destructor
     */
    //@{

    Periodic( node_type const& trans ) : M_trans( trans) {}
    Periodic( Periodic const & p ) : M_trans( p.M_trans ) {}
    ~Periodic() {}

    //@}

    /** @name Operator overloads
     */
    //@{


    //@}

    /** @name Accessors
     */
    //@{

    //! return whether the condition is periodic or not
    static bool isPeriodic()  { return is_periodic; }

    //! return the translation condition that should be applied on Tag2
    node_type const& translation()  { return M_trans; }

    //@}

    /** @name  Mutators
     */
    //@{


    //@}

    /** @name  Methods
     */
    //@{


    //@}



protected:

private:

    node_type M_trans;
};
/**
 * \class NoPeriodicity
 * \brief NoPeriodicity class holder
 *
 * This class the default parameter for function spaces without
 * periodic boundary conditions
 *
 *
 * @author Christophe Prud'homme
 * @see
 */
class NoPeriodicity
{
public:
    /** @name Constants
     */
    //@{

    static const bool is_periodic = false;
    static const uint16_type tag1 = invalid_uint16_type_value;
    static const uint16_type tag2 = invalid_uint16_type_value;
    //@}

    /** @name Accessors
     */
    //@{

    //! return whether the condition is periodic or not
    static bool isPeriodic()  { return is_periodic; }

    //@}
};

}
#endif /* __Periodic_H */

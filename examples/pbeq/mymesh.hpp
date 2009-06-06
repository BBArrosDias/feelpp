/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Simone Deparis <simone.deparis@epfl.ch>
       Date: 2007-07-17

  Copyright (C) 2007 Unil

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
   \file Mesh.hpp
   \author Simone Deparis <simone.deparis@epfl.ch>
   \date 2007-07-17
 */
#ifndef _MYMESH_HPP
#define _MYMESH_HPP

#include <life/lifemesh/geoentity.hpp>
#include <life/lifefilters/gmshtensorizeddomain.hpp>

namespace Life
{

/**
 * \class Mesh
 * \brief Mesh Mesh Generator
 *
 * The \c Mesh class helps with the generation of meshes using the
 * program \c createmesh. Typically one would generate a \c .geo std::string
 * for example and pass it to \c generate() along with the prefix \c
 * name of the \c .geo and \c .msh files to create/generate.
 *
 */

class myMesh  : public GmshTensorizedDomain<3,1,3,Simplex>
{
    typedef GmshTensorizedDomain< 3, 1, 3, Simplex > super;
public:
    /** @name Constants and Typedefs
     */
    //@{

    //@}

    /** @name Constructors, destructor
     */
    //@{

    myMesh()
        :
        super(),
        _M_factor (1),
        _M_farfactor(1),
        _M_farBnd(2)
    {}

    myMesh( myMesh const & td )
        :
        super( td ),
        _M_factor (td._M_factor),
        _M_farfactor(td._M_farfactor),
        _M_farBnd(td._M_farBnd)
    {}

    ~myMesh()
    {}

    //@}

    /** @name Operator overloads
     */
    //@{


    //@}

    /** @name Accessors
     */
    //@{


    //@}

    /** @name  Mutators
     */
    //@{

    /** sets the far boundary characteristic length
     */
    void setFarCharacteristic( double const&  farfactor,
                               double const&  farBnd)
    {
        _M_farfactor = farfactor;
        _M_farBnd = farBnd;
    }

    /** adds a point (ptChar[0],ptChar[1],ptChar[2])
        with characteristic length set by setPointCharacteristic
     */
    void setPointCharacteristic( double const&  factor)
    {
        _M_factor = factor;
    }


    void setPointCharacteristic( std::vector<double> const&  ptChar )
    {
        LIFE_ASSERT( this->nDim != ptChar.size() )( this->nDim ).error( "invalid dimension" );
        _M_ptChar.push_back(ptChar);
    }

    //@}

    /** @name  Methods
     */
    //@{

    bool generateGeo( std::string const& name ) const
    {
        std::string descr = getDescription();
        return Gmsh::generateGeo( name, descr );
    }

    std::string generate( std::string const& name ) const
    {
        std::string descr = getDescription();
        return Gmsh::generate( name, descr );
    }

    //@}


protected:

private:

    std::string getDescription() const;

    std::string getPointCharacteristicDescription() const;

private:

    double _M_factor;
    double _M_farfactor;
    double _M_farBnd;
    std::list<std::vector<double> >  _M_ptChar;

}; // end class myMesh

} // end namespace Life

#endif

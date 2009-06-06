/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-01-26

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2006,2007 Universit� Joseph Fourier (Grenoble I)

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
   \file block.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-01-26
 */
#ifndef __Block_H
#define __Block_H 1

#include <sstream>
#include <list>

#include <life/lifecore/life.hpp>

namespace Life
{
namespace vf
{
/// \cond detail
/*!
  \class Block
  \brief class that describes a matrix or vector block

  @author Christophe Prud'homme
  @see
*/
class Block
{
public:


    /** @name Typedefs
     */
    //@{


    //@}

    /** @name Constructors, destructor
     */
    //@{

    Block( uint16_type __ic = 0, uint16_type __jc = 0, size_type __gic = 0, size_type __gjc = 0 )
        :
        _M_lr( __ic ),
        _M_lc( __jc ),
        _M_gr( __gic ),
        _M_gc( __gjc )
        {}
    Block( Block const & __b )
        :
        _M_lr( __b._M_lr ),
        _M_lc( __b._M_lc ),
        _M_gr( __b._M_gr ),
        _M_gc( __b._M_gc )
        {}
    ~Block()
        {}

    //@}

    /** @name Operator overloads
     */
    //@{

    Block& operator=( Block const& __b )
        {
            if ( LIFE_ISLIKELY( this != &__b ) )
            {
                _M_lr = __b._M_lr;
                _M_lc = __b._M_lc;
                _M_gr = __b._M_gr;
                _M_gc = __b._M_gc;
            }
            return *this;
        }


    //@}

    /** @name Accessors
     */
    //@{

    uint16_type localRow() const { return _M_lr; }
    uint16_type localColumn() const { return _M_lc; }

    size_type globalRowStart() const { return _M_gr; }
    size_type globalColumnStart() const { return _M_gc; }

    //@}

    /** @name  Mutators
     */
    //@{

    void setLocalRow( uint16_type __lr ) { _M_lr = __lr; }
    void setLocalColumn( uint16_type __lc ) { _M_lc = __lc; }

    void setGlobalRowStart( size_type __r ) { _M_gr = __r; }
    void setGlobalColumnStart( size_type __c ) { _M_gc = __c; }

    //@}

    /** @name  Methods
     */
    //@{


    //@}

protected:

private:

    uint16_type _M_lr;
    uint16_type _M_lc;
    size_type _M_gr;
    size_type _M_gc;
};
typedef std::list<Block> list_block_type;

inline
DebugStream&
operator<<( DebugStream& __os, Block const& __b )
{
    std::ostringstream __str;
    __str << "Block [ "
          << __b.localRow() << ","
          << __b.localColumn() << ","
          << __b.globalRowStart() << ","
          << __b.globalColumnStart() << "]";

    __os << __str.str() << "\n";
    return __os;
}

inline
NdebugStream&
operator<<( NdebugStream& __os, Block const& /*__b*/ )
{
    return __os;
}

inline
std::ostream&
operator<<( std::ostream& __os, Block const& __b )
{
    __os << "Block [ "
         << __b.localRow() << ","
         << __b.localColumn() << ","
         << __b.globalRowStart() << ","
         << __b.globalColumnStart() << "]";

    return __os;
}
/// \endcond
} // vf
} // life
#endif /* __Block_H */

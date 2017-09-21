//! -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
//!
//! This file is part of the Feel++ library
//!
//! This library is free software; you can redistribute it and/or
//! modify it under the terms of the GNU Lesser General Public
//! License as published by the Free Software Foundation; either
//! version 2.1 of the License, or (at your option) any later version.
//!
//! This library is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//! Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public
//! License along with this library; if not, write to the Free Software
//! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//!
//! @file
//! @author Christophe Prud'homme <christophe.prudhomme@feelpp.org>
//! @date 18 Sep 2017
//! @copyright 2017 Feel++ Consortium
//!
#include <feel/feelpython/pyexpr.hpp>
#include <feel/feelcore/feel.hpp>
#include <feel/feelcore/feelio.hpp>


namespace Feel {
std::map<std::string,std::string> 
pyexpr( std::string const& pycode, std::vector<std::string> const& locals )
{
    using Feel::cout;
    py::scoped_interpreter guard{};

    py::dict _locals;
    py::exec(pycode.c_str(), py::globals(),_locals);

    std::map<std::string,std::string> r;
    for( auto l : locals )
    {
        std::string cmd = l + "= toginac(sympify(" + l + "), [x] if len(" + l + ".free_symbols)==0 else " + l + ".free_symbols );";
        cout << "cmd : " << cmd << std::endl;
        py::exec(cmd, py::globals(), _locals );
        r[l] = _locals[l.c_str()].cast<std::string>();
    }
    return r;
}




}

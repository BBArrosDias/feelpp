/* -*- mode: c++ -*-

This file is part of the Life library

Author(s): Simone Deparis <simone.deparis@epfl.ch>
Date: 2007-07-10

Copyright (C) 2007 EPFL

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
   \file atom.cpp
   \author Simone Deparis <simone.deparis@epfl.ch>
   \date 2007-07-10
*/

#include "atom.hpp"
//#include <iostream>
#include <fstream>
int
main( int argc, char** argv )
{
    Life::Atom a;
    std::ifstream pqrfile;

    if (argc == 1)
        {
            std::cout << "argc (here = " << argc << ", " << argv[0]
                      << ") must be > 1"<<  std::endl;
            return 1;
        }

    pqrfile.open(argv[1], std::ifstream::in);
    if (pqrfile.fail())
        {
            Life::Log() << "failed to open " << argv[1] << "\n";
            return 1;
        }

    if (a.readPQRline(pqrfile) == 0)
        {
            a.showMe();
        }
    else
        Life::Log() << "Problems reading atom" << "\n";

    Life::Log() << "Finished" << "\n";

    return 0;


}


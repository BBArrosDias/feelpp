/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-10-18

  Copyright (C) 2005,2006 EPFL

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
   \file test_matrix.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-10-18
 */
#include <life/lifecore/life.hpp>
#include <life/lifecore/traits.hpp>
#include <life/lifecore/application.hpp>

#include <life/lifealg/matrixpetsc.hpp>
#include <life/lifealg/vectorpetsc.hpp>

using namespace Life;


Life::AboutData
makeAbout()
{
    Life::AboutData about( "test_matrix" ,
                            "test_matrix" ,
                            "0.1",
                            "matrix test",
                            Life::AboutData::License_LGPL,
                            "Copyright (c) 2005,2006 EPFL");

    about.addAuthor("Christophe Prud'homme", "developer", "christophe.prudhomme@ujf-grenoble.fr", "");
    return about;

}
#if defined(HAVE_PETSC_H)
class test_matrixpetsc
    :
    public Application
{
public:

    test_matrixpetsc( int argc, char** argv )
        :
        Application( argc, argv, makeAbout() )
    {
        std::cout << "id: " << Application::processId() << "\n"
                  << "nprocs: " << Application::nProcess() << "\n";
    }

    void operator()() const
    {
        int m = 8;//10*Application::nProcess();
        int n = 8;//10*Application::nProcess();

        VectorPetsc<double> vec;
        MatrixPetsc<double> mat;
        std::cout << "is initialized ? " << mat.isInitialized() << "\n";
        mat.init( m*n, m*n, n/Application::nProcess(), n );
        std::cout << "is initialized ? " << mat.isInitialized() << "\n";

        /*
           Currently, all PETSc parallel matrix formats are partitioned by
           contiguous chunks of rows across the processors.  Determine which
           rows of the matrix are locally owned.
        */
        int Istart;
        int Iend;
        MatGetOwnershipRange(mat.mat(),&Istart,&Iend);
        Debug() << "Istart = "<< Istart << "\n";
        Debug() << "Iend = "<< Iend << "\n";

        /*
           Set matrix elements for the 2-D, five-point stencil in parallel.
           - Each processor needs to insert only elements that it owns
           locally (but any non-local elements will be sent to the
           appropriate processor during matrix assembly).
           - Always specify global rows and columns of matrix entries.

           Note: this uses the less common natural ordering that orders first
           all the unknowns for x = h then for x = 2h etc; Hence you see J = I +- n
           instead of J = I +- m as you might expect. The more standard ordering
           would first do all variables for y = h, then y = 2h etc.

        */

        for (int I=Istart; I<Iend; I++)
        {
            int J;
            double v = -1.0;
            int i = I/n;
            int j = I - i*n;
            Debug() << "I= " << I << "\n";

            if (i>0)
                {
                    J = I - n+1;
                    Debug() << "1 J= " << J << "\n";
                    mat.set(I,J,v);
                }
            if (i<m-1)
                {
                    J = I + n-1;
                    Debug() << "2 J= " << J << "\n";
                    mat.set(I,J,v);
                }
            if (j>0)
                {
                    J = I - 1;
                    Debug() << "3 J= " << J << "\n";
                    mat.set(I,J,v);
                }
            if (j<n-1)
                {
                    J = I + 1;
                    Debug() << "4 J= " << J << "\n";
                    mat.set(I,J,v);
                }
            v = 4.0; mat.set(I,I,v);
        }
        Debug() << "closing petsc matrix\n";
        mat.close();
        Debug() << "closing petsc matrix done\n";

        Debug() << "saving petsc matrix in matlab\n";
        //mat.printMatlab("m");
        mat.printMatlab(std::string("/tmp/mat.m") );
        Debug() << "saving petsc matrix in matlab done\n";

    }
};
#endif

#if defined(HAVE_PETSC_H)
int main( int argc, char** argv )
{
    test_matrixpetsc t( argc, argv );
    t();
    return EXIT_SUCCESS;
}
#else
int main( int /*argc*/, char** /*argv*/ )
{
  return EXIT_SUCCESS;
}
#endif

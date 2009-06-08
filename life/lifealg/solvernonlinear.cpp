/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2008-04-17

  Copyright (C) 2008 Universit� Joseph Fourier (Grenoble I)

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
   \file solvernonlinear.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2008-04-17
 */
#include <life/lifecore/life.hpp>
#include <life/lifealg/solvernonlinear.hpp>
#include <life/lifealg/solvernonlinearpetsc.hpp>
#include <life/lifealg/solvernonlineartrilinos.hpp>

namespace Life
{
template <typename T>
inline
SolverNonLinear<T>::SolverNonLinear ()
  :
    residual        (0),
    jacobian        (0),
    matvec          (0),
    M_is_initialized (false),
    M_prec_matrix_structure( SAME_NONZERO_PATTERN )
{
}

template <typename T>
inline
SolverNonLinear<T>::SolverNonLinear ( SolverNonLinear const& snl )
  :
    residual        ( snl.residual ),
    jacobian        ( snl.jacobian ),
    matvec          ( snl.matvec ),
    M_is_initialized (snl.M_is_initialized ),
    M_prec_matrix_structure( snl.M_prec_matrix_structure )
{
}



template <typename T>
inline
SolverNonLinear<T>::~SolverNonLinear ()
{
  this->clear ();
}
template <typename T>
boost::shared_ptr<SolverNonLinear<T> >
SolverNonLinear<T>::build( po::variables_map const& vm, std::string const& prefix )
{
    SolverPackage solver_package;
    if ( vm["nlsolver"].template as<std::string>() == "petsc" )
        {
#if defined( HAVE_PETSC )
            solver_package = SOLVERS_PETSC;
#endif
        }
    else if ( vm["nlsolver"].template as<std::string>() == "trilinos" )
        {
#if defined( HAVE_TRILINOS )
            solver_package = SOLVERS_TRILINOS;
#endif
        }
    else
        {
            Log() << "[SolverNonLinear] solver " << vm["nlsolver"].template as<std::string>() << " not available\n";
            Log() << "[Backend] use fallback  gmm\n";
#if defined( HAVE_PETSC )
            solver_package = SOLVERS_PETSC;
#endif
        }
    // Build the appropriate solver
    switch (solver_package)
        {
#if defined( HAVE_PETSC )
        case SOLVERS_PETSC:
            {


                solvernonlinear_ptrtype ap(new SolverNonLinearPetsc<T>);
                return ap;
            }
            break;
#endif

#if defined( HAVE_TRILINOS )
        case SOLVERS_TRILINOS:
            {
                solvernonlinear_ptrtype ap(new SolverNonLinearTrilinos<T>);
                return ap;
            }
            break;
#endif

        default:
            std::cerr << "ERROR:  Unrecognized NonLinear solver package: "
                      << solver_package
                      << std::endl;
            throw std::invalid_argument( "invalid solver package" );
        }

    return solvernonlinear_ptrtype();
}

template <typename T>
boost::shared_ptr<SolverNonLinear<T> >
SolverNonLinear<T>::build( SolverPackage solver_package )
{
#if defined( HAVE_PETSC )
    if ( solver_package != SOLVERS_PETSC )
        {
            Log() << "[SolverNonLinear] solver " << solver_package << " not available\n";
            Log() << "[Backend] use fallback  petsc: " << SOLVERS_PETSC << "\n";
            solver_package = SOLVERS_PETSC;
        }
    // Build the appropriate solver
    switch (solver_package)
        {

        case SOLVERS_PETSC:
            {

#if defined( HAVE_PETSC )
                solvernonlinear_ptrtype ap(new SolverNonLinearPetsc<T>);
                return ap;
#else
                std::cerr << "PETSc is not available/installed" << std::endl;
                throw std::invalid_argument( "invalid solver PETSc package" );
#endif
            }
            break;

        case SOLVERS_TRILINOS:
            {
#if defined( HAVE_TRILINOS )
                solvernonlinear_ptrtype ap(new SolverNonLinearTrilinos<T>);
                return ap;
#else
                std::cerr << "Trilinos NOX is not available/installed" << std::endl;
                throw std::invalid_argument( "invalid solver NOX package" );
#endif
            }
            break;

        default:
            std::cerr << "ERROR:  Unrecognized NonLinear solver package: "
                      << solver_package
                      << std::endl;
            throw std::invalid_argument( "invalid solver package" );
        }
#endif
    return solvernonlinear_ptrtype();
}


/*
 * Explicit instantiations
 */
template class SolverNonLinear<double>;

/**
 * \return the command lines options of the petsc backend
 */
po::options_description nlsolver_options()
{
    po::options_description _options( "Non Linear Solver options");
    _options.add_options()
        // solver options
        ("nlsolver", Life::po::value<std::string>()->default_value( "petsc" ), "nonlinear solver type: petsc")
        ("nlsolver", Life::po::value<std::string>()->default_value( "trilinos" ), "nonlinear solver type: trilinos NOX")
        ;
    return _options;
}

}

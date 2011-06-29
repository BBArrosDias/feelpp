/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=tcl:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2007-12-23

  Copyright (C) 2007-2010 Université Joseph Fourier (Grenoble I)

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
   \file backend.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2007-12-23
 */
#include <feel/feelalg/backend.hpp>
#include <feel/feelalg/backendgmm.hpp>
#include <feel/feelalg/backendpetsc.hpp>
#include <feel/feelalg/backendtrilinos.hpp>

namespace Feel
{
std::string
_o(std::string const& prefix, std::string const& opt )
{
    std::string o = prefix;
    if ( !o.empty() )
        o += "-";
    return o+opt;
}
template <typename T>
Backend<T>::Backend()
    :
#if defined( HAVE_PETSC_H )
    M_backend    (BACKEND_PETSC),
#else
    M_backend    (BACKEND_GMM),
#endif
    M_prefix(""),
    M_nlsolver(),
    M_rtolerance( 1e-13 ),
    M_dtolerance( 1e5 ),
    M_atolerance( 1e-50 ),
    M_transpose( false ),
    M_maxit( 1000 ),
    M_ksp( "gmres" ),
    M_pc( "lu" ),
    M_fieldSplit("additive")
{
}

template <typename T>
Backend<T>::Backend( Backend const& backend )
    :
    M_backend( backend.M_backend ),
    M_prefix( backend.M_prefix ),
    M_nlsolver( backend.M_nlsolver ),
    M_prec_matrix_structure( SAME_NONZERO_PATTERN ),
    M_rtolerance( backend.M_rtolerance ),
    M_dtolerance( backend.M_dtolerance ),
    M_atolerance( backend.M_atolerance ),
    M_transpose( backend.M_transpose ),
    M_maxit( backend.M_maxit ),
    M_ksp( backend.M_ksp ),
    M_pc( backend.M_pc ),
    M_fieldSplit( backend.M_fieldSplit)
{
}
template <typename T>
Backend<T>::Backend( po::variables_map const& vm, std::string const& prefix )
    :
    M_prefix( prefix ),
    M_nlsolver( solvernonlinear_type::build( vm ) ),
    M_prec_matrix_structure( SAME_NONZERO_PATTERN ),
    M_rtolerance( vm[_o(prefix,"ksp-rtol")].template as<double>() ),
    M_dtolerance( vm[_o(prefix,"ksp-dtol")].template as<double>() ),
    M_atolerance( vm[_o(prefix,"ksp-atol")].template as<double>() ),
    M_transpose( false ),
    M_maxit( vm[_o(prefix,"ksp-maxit")].template as<size_type>() ),
    M_ksp( vm[_o(prefix,"ksp-type")].template as<std::string>() ),
    M_pc( vm[_o(prefix,"pc-type")].template as<std::string>() ),
    M_fieldSplit( vm[_o(prefix,"fieldsplit-type")].template as<std::string>() )
{
}
template <typename T>
Backend<T>::~Backend()
{
    //this->clear ();
}
template <typename T>
typename Backend<T>::backend_ptrtype
Backend<T>::build( BackendType bt )
{
    // Build the appropriate solver
    switch ( bt )
        {

        case BACKEND_GMM:
            {
                return backend_ptrtype( new BackendGmm<value_type> );
            }
            break;
#if defined ( HAVE_PETSC_H )
        case BACKEND_PETSC:
            {
                return backend_ptrtype( new BackendPetsc<value_type> );
            }
            break;
#endif

#if defined ( HAVE_TRILINOS_EPETRA )
        case BACKEND_TRILINOS:
            {
                return backend_ptrtype( new BackendTrilinos );
            }
            break;
#endif
        default:
            std::cerr << "ERROR:  Unrecognized backend type package: "
                      << bt
                      << std::endl;
            throw std::invalid_argument( "invalid backend type" );
        }

    return backend_ptrtype();
}
template <typename T>
typename Backend<T>::backend_ptrtype
Backend<T>::build( po::variables_map const& vm, std::string const& prefix )
{
    Log() << "[Backend] backend " << vm["backend"].template as<std::string>() << "\n";
    BackendType bt;
    if ( vm["backend"].template as<std::string>() == "gmm" )
        bt = BACKEND_GMM;
    else if ( vm["backend"].template as<std::string>() == "petsc" )
        bt = BACKEND_PETSC;
    else if ( vm["backend"].template as<std::string>() == "trilinos" )
        bt = BACKEND_TRILINOS;
    else
        {

#if defined( HAVE_PETSC_H )

            Log() << "[Backend] use fallback backend petsc\n";
            bt = BACKEND_PETSC;
#else
            Log() << "[Backend] backend " << vm["backend"].template as<std::string>() << " not available\n";
            Log() << "[Backend] use fallback backend gmm\n";
            bt = BACKEND_GMM;
#endif
        }

    // Build the appropriate solver
    switch ( bt )
        {
#if defined ( HAVE_PETSC_H )
        case BACKEND_PETSC:
            {
                Log() << "[Backend] Instantiate a Petsc backend\n";
                return backend_ptrtype( new BackendPetsc<value_type>( vm, prefix ) );
            }
            break;
#endif
#if defined ( HAVE_TRILINOS_EPETRA )
        case BACKEND_TRILINOS:
            {
#if defined ( HAVE_TRILINOS_EPETRA )
                return backend_ptrtype( new BackendTrilinos( vm, prefix ) );
#else
                return backend_ptrtype();
#endif
            }
            break;
#endif
        case BACKEND_GMM:
        default:
            {
                return backend_ptrtype( new BackendGmm<value_type>( vm, prefix ) );
            }
            break;
        }

    // should never happen
    return backend_ptrtype();
}
template <typename T>
typename Backend<T>::solve_return_type
Backend<T>::solve( sparse_matrix_ptrtype const& A,
                   sparse_matrix_ptrtype const& P,
                   vector_ptrtype& x,
                   vector_ptrtype const& b,
                   bool reusePC )
{
    M_reusePC = reusePC;
    if ( !M_reusePC ) {
        reset();
    }
    start();

    this->setPrecMatrixStructure( SAME_PRECONDITIONER );
    //std::cout << "backend: " << this->precMatrixStructure() << "\n";
    boost::tie( M_converged, M_iteration, M_residual ) = this->solve( A, P, x, b );
    stop();
    M_reuseFailed = M_reusedPC && (!M_converged);
    /*
    if (M_reuseFailed)
        std::cout << "\n OK :nb iteration : "<<M_iteration<<"  le residu : "<< M_residual;
    else
    std::cout << "\n BAD :nb iteration : "<<M_iteration<<"  le residu : "<< M_residual;
    */

    if ( M_reuseFailed )
        {
            reset();
            start();
            this->setPrecMatrixStructure( SAME_NONZERO_PATTERN );
            std::cout << "reuse fail backend: " << this->precMatrixStructure() << "\n";
            boost::tie( M_converged, M_iteration, M_residual ) = this->solve( A, P, x, b );
            stop();
        }

    return boost::make_tuple( M_converged, M_iteration, M_residual );
}
template <typename T>
typename Backend<T>::nl_solve_return_type
Backend<T>::nlSolve( sparse_matrix_ptrtype& A,
                     vector_ptrtype& x,
                     vector_ptrtype& b,
                     const double tol, const int its,
                     bool reusePC )
{
    M_nlsolver->setPrecMatrixStructure( this->precMatrixStructure() );
    M_nlsolver->solve( A, x, b, tol, its );
    return boost::make_tuple( true, its, tol );

}
template <typename T>
typename Backend<T>::nl_solve_return_type
Backend<T>::nlSolve( sparse_matrix_ptrtype& A,
                     vector_ptrtype& x,
                     vector_ptrtype& b,
                     const double tol, const int its )
{

    M_nlsolver->setPreconditionerType( this->pcEnumType() );
    M_nlsolver->setKspSolverType( this->kspEnumType() );

    M_nlsolver->setPrecMatrixStructure( this->precMatrixStructure() );
    M_nlsolver->solve( A, x, b, tol, its );
    return boost::make_tuple( true, its, tol );
}
template <typename T>
typename Backend<T>::real_type
Backend<T>::dot( vector_type const& x, vector_type const& y ) const
{
    real_type localres = 0;
    for( size_type i = 0; i < x.localSize(); ++i )
        {
            localres += x(i)*y(i);
        }
    real_type globalres=localres;
    mpi::all_reduce( M_comm, localres, globalres, std::plus<real_type>() );
    return globalres;
}

template <typename T>
void
Backend<T>::start()
{
    M_timer.restart();
}

template <typename T>
void
Backend<T>::stop()
{
    double solveTime = M_timer.elapsed();
    double solveIter = M_iteration + 0.01;
    M_reusedPC = M_reusePC;
    ++M_nUsePC;
    if ( M_nUsePC == 1 )
        {
            M_reusePC = true;
            M_firstSolveTime = solveTime;
            if ( !M_reuseFailed )
                M_maxit = std::min( M_maxit, (size_type)(1.5*solveIter + 10.5));
        }
    else
        {
            double nextSolveIter;
            if ( M_nUsePC == 2 )
                {
                    M_totalSolveIter = solveIter*(1.0+M_firstSolveTime/solveTime);
                    nextSolveIter = solveIter;
                }
            else
                {
                    M_totalSolveIter += solveIter;
                    //                 if ( solveIter > M_lastSolveIter )
                    //                     nextSolveIter = 2*solveIter - M_lastSolveIter;
                    //                 else
                    //                     nextSolveIter = solveIter * solveIter / M_lastSolveIter;
                    nextSolveIter = solveIter;
                }
            M_reusePC = ( M_totalSolveIter > M_nUsePC * nextSolveIter );
            M_lastSolveIter = solveIter;
            if ( M_reusePC )
                {
                    M_maxit = std::min( M_maxit, (size_type)( M_totalSolveIter/M_nUsePC + 0.5 ) );
                }
        }
}

template <typename T>
void
Backend<T>::reset()
{
    M_reusePC = false;
    M_totalSolveIter = 0.0;
    M_nUsePC = 0;
    //M_backend->set_maxiter( M_maxit );

}

template<typename T>
SolverType
Backend<T>::kspEnumType() const
{
    if (this->kspType()=="cg")              return CG;
    else if (this->kspType()=="cr")         return CR;
    else if (this->kspType()=="cgs")        return CGS;
    else if (this->kspType()=="bicg")       return BICG;
    else if (this->kspType()=="tcqmr")      return TCQMR;
    else if (this->kspType()=="tfqmr")      return TFQMR;
    else if (this->kspType()=="lsqr")       return LSQR;
    else if (this->kspType()=="bicgstab")   return BICGSTAB;
    else if (this->kspType()=="minres")     return MINRES;
    else if (this->kspType()=="gmres")      return GMRES;
    else if (this->kspType()=="richardson") return RICHARDSON;
    else if (this->kspType()=="chebyshev")  return CHEBYSHEV;
    else return GMRES;

} // Backend::kspEnumType

template<typename T>
PreconditionerType
Backend<T>::pcEnumType() const
{

    if (this->pcType()=="lu")                return LU_PRECOND;
    else if (this->pcType()=="ilu")          return ILU_PRECOND;
    else if (this->pcType()=="id")           return IDENTITY_PRECOND;
    else if (this->pcType()=="cholesky")     return CHOLESKY_PRECOND;
    else if (this->pcType()=="icc")          return ICC_PRECOND;
    else if (this->pcType()=="asm")          return ASM_PRECOND;
    else if (this->pcType()=="jacobi")       return JACOBI_PRECOND;
    else if (this->pcType()=="block_jacobi") return BLOCK_JACOBI_PRECOND;
    else if (this->pcType()=="sor")          return SOR_PRECOND;
    else if (this->pcType()=="eisenstat")    return EISENSTAT_PRECOND;
    else if (this->pcType()=="shell")        return SHELL_PRECOND;
    else if (this->pcType()=="fieldsplit")   return FIELDSPLIT_PRECOND;
    else return LU_PRECOND;

} // Backend::pcEnumType

template<typename T>
FieldSplitType
Backend<T>::fieldSplitEnumType() const
{
    if (this->fieldsplitType()=="additive")  return ADDITIVE;
    else if (this->fieldsplitType()=="multiplicative")  return MULTIPLICATIVE;
    else if (this->fieldsplitType()=="schur")  return SCHUR;
    else return ADDITIVE;

} //Backend fieldSplitEnumType

/*
 * Explicit instantiations
 */
template class Backend<double>;

/**
 * \return the command lines options of the petsc backend
 */
po::options_description backend_options( std::string const& prefix )
{
    std::string _prefix = prefix;
    if ( !_prefix.empty() )
        _prefix += "-";

    po::options_description _options( "Linear and NonLinear Solvers Backend " + prefix + " options");
    _options.add_options()
        // solver options
#if defined( HAVE_PETSC_H )

        ((_prefix+"backend").c_str(), Feel::po::value<std::string>()->default_value( "petsc" ), "backend type: PETSc, trilinos, gmm")
#else
        ((_prefix+"backend").c_str(), Feel::po::value<std::string>()->default_value( "gmm" ), "backend type: PETSc, trilinos, gmm")
#endif
        ((_prefix+"ksp-rtol").c_str(), Feel::po::value<double>()->default_value( 1e-13 ), "relative tolerance")
        ((_prefix+"ksp-atol").c_str(), Feel::po::value<double>()->default_value( 1e-50 ), "absolute tolerance")
        ((_prefix+"ksp-dtol").c_str(), Feel::po::value<double>()->default_value( 1e5 ), "divergence tolerance")
        ((_prefix+"ksp-maxit").c_str(), Feel::po::value<size_type>()->default_value( 1000 ), "maximum number of iterations")

        ((_prefix+"ksp-type").c_str(), Feel::po::value<std::string>()->default_value( "gmres" ), "cg, bicgstab, gmres")

        // preconditioner options
        ((_prefix+"pc-type").c_str(), Feel::po::value<std::string>()->default_value( "lu" ), "type of preconditioners (lu, ilut, ilutp, diag, id,...)")
        ((_prefix+"ilu-threshold").c_str(), Feel::po::value<double>()->default_value( 1e-3 ), "threshold value for preconditioners")
        ((_prefix+"ilu-fillin").c_str(), Feel::po::value<int>()->default_value( 2 ), "fill-in level value for preconditioners")

        // solver control options
        ((_prefix+"gmres-restart").c_str(), Feel::po::value<int>()->default_value( 20 ), "number of iterations before solver restarts (gmres)")
        ((_prefix+"ksp-verbose").c_str(), Feel::po::value<int>()->default_value( 0 ), "(=0,1,2) print solver iterations")
        ((_prefix+"fieldsplit-type").c_str(), Feel::po::value<std::string>()->default_value( "additive" ), "type of fieldsplit (additive, multiplicative, schur)")
        ;

    for ( uint16_type i=0;i<5;++i)
        {
            _options.add_options()
                ((boost::format("%1%fieldsplit-%2%-pc-type") %_prefix%i).str().c_str(), Feel::po::value<std::string>()->default_value( "lu" ), "type of fieldsplit preconditioners")
                ((boost::format("%1%fieldsplit-%2%-ksp-type") %_prefix%i).str().c_str(), Feel::po::value<std::string>()->default_value( "gmres" ), "type of fieldsplit solver")
                ;

        }

    return _options;
}

}

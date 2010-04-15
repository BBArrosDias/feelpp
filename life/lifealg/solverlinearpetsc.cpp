/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-11-27

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2008, 2009 Université de Grenoble 1

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
   \file solverlinearpetsc.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-11-27
 */
// $Id: petsc_linear_solver.C,v 1.5 2005/05/11 23:12:00 benkirk Exp $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2005  Benjamin S. Kirk, John W. Peterson

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <life/lifecore/life.hpp>

#ifdef HAVE_PETSC_H

#include <life/lifealg/solverlinearpetsc.hpp>

namespace Life
{

/*----------------------- functions ----------------------------------*/
template <typename T>
void
SolverLinearPetsc<T>::clear ()
{
    if (this->initialized())
    {
        this->setInitialized( false );

        int ierr=0;

// 2.1.x & earlier style
#if (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR <= 1)

        ierr = SLESDestroy(_M_sles);
        CHKERRABORT(Application::COMM_WORLD,ierr);

// 2.2.0 & newer style
#else

        ierr = KSPDestroy(_M_ksp);
        CHKERRABORT(Application::COMM_WORLD,ierr);
#endif

        // Mimic PETSc default solver and preconditioner
        this->setSolverType(  GMRES );

        if (Application::nProcess() == 1)
            this->setPreconditionerType( LU_PRECOND );
        else
            this->setPreconditionerType( BLOCK_JACOBI_PRECOND );
    }
}



template <typename T>
void SolverLinearPetsc<T>::init ()
{
    // Initialize the data structures if not done so already.
    if (!this->initialized())
    {
        this->setInitialized(  true );

        int ierr=0;

// 2.1.x & earlier style
#if (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR <= 1)

        // Create the linear solver context
        ierr = SLESCreate (Application::COMM_WORLD, &_M_sles);
        CHKERRABORT(Application::COMM_WORLD,ierr);

        // Create the Krylov subspace & preconditioner contexts
        ierr = SLESGetKSP       (_M_sles, &_M_ksp);
        CHKERRABORT(Application::COMM_WORLD,ierr);
        ierr = SLESGetPC        (_M_sles, &_M_pc);
        CHKERRABORT(Application::COMM_WORLD,ierr);

        // Have the Krylov subspace method use our good initial guess rather than 0
        ierr = KSPSetInitialGuessNonzero (_M_ksp, PETSC_TRUE);
        CHKERRABORT(Application::COMM_WORLD,ierr);

        // Set user-specified  solver and preconditioner types
        this->setPetscSolverType();
        this->setPetscPreconditionerType();

        // Set the options from user-input
        // Set runtime options, e.g.,
        //      -ksp_type <type> -pc_type <type> -ksp_monitor -ksp_rtol <rtol>
        //  These options will override those specified above as long as
        //  SLESSetFromOptions() is called _after_ any other customization
        //  routines.

        ierr = SLESSetFromOptions (_M_sles);
        CHKERRABORT(Application::COMM_WORLD,ierr);

// 2.2.0 & newer style
#else

        // Create the linear solver context
        ierr = KSPCreate (Application::COMM_WORLD, &_M_ksp);
        CHKERRABORT(Application::COMM_WORLD,ierr);

        // Create the preconditioner context
        ierr = KSPGetPC        (_M_ksp, &_M_pc);
        CHKERRABORT(Application::COMM_WORLD,ierr);

        // Have the Krylov subspace method use our good initial guess rather than 0
        ierr = KSPSetInitialGuessNonzero (_M_ksp, PETSC_TRUE);
        CHKERRABORT(Application::COMM_WORLD,ierr);

        // Set user-specified  solver and preconditioner types
        this->setPetscSolverType();
        this->setPetscPreconditionerType();

        // Set the options from user-input
        // Set runtime options, e.g.,
        //      -ksp_type <type> -pc_type <type> -ksp_monitor -ksp_rtol <rtol>
        //  These options will override those specified above as long as
        //  KSPSetFromOptions() is called _after_ any other customization
        //  routines.
        ierr = PCSetFromOptions (_M_pc);
        CHKERRABORT(Application::COMM_WORLD,ierr);
        ierr = KSPSetFromOptions (_M_ksp);
        CHKERRABORT(Application::COMM_WORLD,ierr);


#endif

        // Notify PETSc of location to store residual history.
        // This needs to be called before any solves, since
        // it sets the residual history length to zero.  The default
        // behavior is for PETSc to allocate (internally) an array
        // of size 1000 to hold the residual norm history.
        ierr = KSPSetResidualHistory(_M_ksp,
                                     PETSC_NULL,   // pointer to the array which holds the history
                                     PETSC_DECIDE, // size of the array holding the history
                                     PETSC_TRUE);  // Whether or not to reset the history for each solve.
        CHKERRABORT(Application::COMM_WORLD,ierr);
    }
}








template <typename T>
std::pair<unsigned int, typename SolverLinearPetsc<T>::real_type>
SolverLinearPetsc<T>::solve (MatrixSparse<T> const&  matrix_in,
                             MatrixSparse<T> const&  precond_in,
                             Vector<T> & solution_in,
                             Vector<T> const& rhs_in,
                             const double tol,
                             const unsigned int m_its,
                             bool transpose )
{
    this->init ();

    MatrixPetsc<T> * matrix   = const_cast<MatrixPetsc<T> *>( dynamic_cast<MatrixPetsc<T> const*>(&matrix_in) );
    MatrixPetsc<T> * precond  = const_cast<MatrixPetsc<T> *>( dynamic_cast<MatrixPetsc<T> const*>(&precond_in) );
    VectorPetsc<T> * solution = dynamic_cast<VectorPetsc<T>*>(&solution_in);
    VectorPetsc<T> * rhs      = const_cast<VectorPetsc<T> *>( dynamic_cast<VectorPetsc<T> const*>(&rhs_in) );

    // We cast to pointers so we can be sure that they succeeded
    // by comparing the result against NULL.
    LIFE_ASSERT(matrix   != NULL).error( "non petsc matrix structure" );
    LIFE_ASSERT(precond  != NULL).error( "non petsc matrix structure" );
    LIFE_ASSERT(solution != NULL).error( "non petsc vector structure" );
    LIFE_ASSERT(rhs      != NULL).error( "non petsc vector structure" );

    int ierr=0;
    int its=0, max_its = 10*static_cast<int>(m_its);
    PetscReal final_resid=0.;

    // Close the matrices and vectors in case this wasn't already done.
    matrix->close ();
    precond->close ();
    solution->close ();
    rhs->close ();

//   // If matrix != precond, then this means we have specified a
//   // special preconditioner, so reset preconditioner type to PCMAT.
//   if (matrix != precond)
//     {
//       this->_preconditioner_type = USER_PRECOND;
//       this->set_petsc_preconditioner_type ();
//     }

// 2.1.x & earlier style
#if (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR <= 1)

    // Set operators. The input matrix works as the preconditioning matrix
    ierr = SLESSetOperators(_M_sles, matrix->mat(), precond->mat(),
                            SAME_NONZERO_PATTERN);
    CHKERRABORT(Application::COMM_WORLD,ierr);


    // Set the tolerances for the iterative solver.  Use the user-supplied
    // tolerance for the relative residual & leave the others at default values.
    ierr = KSPSetTolerances (_M_ksp, tol, PETSC_DEFAULT,
                             PETSC_DEFAULT, max_its);
    CHKERRABORT(Application::COMM_WORLD,ierr);


    // Solve the linear system
    ierr = SLESSolve (_M_sles, rhs->vec(), solution->vec(), &its);
    CHKERRABORT(Application::COMM_WORLD,ierr);


    // Get the norm of the final residual to return to the user.
    ierr = KSPGetResidualNorm (_M_ksp, &final_resid);
    CHKERRABORT(Application::COMM_WORLD,ierr);

// 2.2.0
#elif (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR == 2) && (PETSC_VERSION_SUBMINOR == 0)

    // Set operators. The input matrix works as the preconditioning matrix
    ierr = KSPSetOperators(_M_ksp, matrix->mat(), precond->mat(),
                           SAME_NONZERO_PATTERN);
    CHKERRABORT(Application::COMM_WORLD,ierr);


    // Set the tolerances for the iterative solver.  Use the user-supplied
    // tolerance for the relative residual & leave the others at default values.
    // Convergence is detected at iteration k if
    // ||r_k||_2 < max(rtol*||b||_2 , abstol)
    // where r_k is the residual vector and b is the right-hand side.  Note that
    // it is the *maximum* of the two values, the larger of which will almost
    // always be rtol*||b||_2.
    ierr = KSPSetTolerances (_M_ksp,
                             tol,           // rtol   = relative decrease in residual  (1.e-5)
                             PETSC_DEFAULT, // abstol = absolute convergence tolerance (1.e-50)
                             PETSC_DEFAULT, // dtol   = divergence tolerance           (1.e+5)
                             max_its);
    CHKERRABORT(Application::COMM_WORLD,ierr);


    // Set the solution vector to use
    ierr = KSPSetSolution (_M_ksp, solution->vec());
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Set the RHS vector to use
    ierr = KSPSetRhs (_M_ksp, rhs->vec());
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Solve the linear system
    if ( transpose )
        ierr = KSPSolveTranspose (_M_ksp);
    else
        ierr = KSPSolve (_M_ksp);
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Get the number of iterations required for convergence
    ierr = KSPGetIterationNumber (_M_ksp, &its);
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Get the norm of the final residual to return to the user.
    ierr = KSPGetResidualNorm (_M_ksp, &final_resid);
    CHKERRABORT(Application::COMM_WORLD,ierr);

// 2.2.1 & newer style
#else

    // Set operators. The input matrix works as the preconditioning matrix
    ierr = KSPSetOperators(_M_ksp, matrix->mat(), precond->mat(),
                           (MatStructure) this->precMatrixStructure() );
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Set the tolerances for the iterative solver.  Use the user-supplied
    // tolerance for the relative residual & leave the others at default values.
    ierr = KSPSetTolerances (_M_ksp,
                             1e-25,
                             //PETSC_DEFAULT,
                             PETSC_DEFAULT,
                             //PETSC_DEFAULT,
                             1e30,
                             max_its);
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Solve the linear system
    if ( transpose )
        ierr = KSPSolveTranspose (_M_ksp, rhs->vec(), solution->vec());
    else
        ierr = KSPSolve (_M_ksp, rhs->vec(), solution->vec());
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Get the number of iterations required for convergence
    ierr = KSPGetIterationNumber (_M_ksp, &its);
    CHKERRABORT(Application::COMM_WORLD,ierr);

    // Get the norm of the final residual to return to the user.
    ierr = KSPGetResidualNorm (_M_ksp, &final_resid);
    //std::cout << "final residual = " << final_resid << "\n";
    CHKERRABORT(Application::COMM_WORLD,ierr);


    KSPConvergedReason reason;
    KSPGetConvergedReason(_M_ksp,&reason);
    if (reason==KSP_DIVERGED_INDEFINITE_PC)
    {
        Log() << "[solverlinearpetsc] Divergence because of indefinite preconditioner;\n";
        Log() << "[solverlinearpetsc] Run the executable again but with '-pc_factor_shift_type POSITIVE_DEFINITE' option.\n";
    }
    else if (reason<0)
    {
        Log() <<"[solverlinearpetsc] Other kind of divergence: this should not happen.\n";
    }
#endif
 // return the # of its. and the final residual norm.
    return std::make_pair(its, final_resid);


}



template <typename T>
void
SolverLinearPetsc<T>::getResidualHistory(std::vector<double>& hist)
{
  int ierr = 0;
  int its  = 0;

  // Fill the residual history vector with the residual norms
  // Note that GetResidualHistory() does not copy any values, it
  // simply sets the pointer p.  Note that for some Krylov subspace
  // methods, the number of residuals returned in the history
  // vector may be different from what you are expecting.  For
  // example, TFQMR returns two residual values per iteration step.
  double* p;
  ierr = KSPGetResidualHistory(_M_ksp, &p, &its);
  CHKERRABORT(Application::COMM_WORLD,ierr);

  // Check for early return
  if (its == 0) return;

  // Create space to store the result
  hist.resize(its);

  // Copy history into the vector provided by the user.
  for (int i=0; i<its; ++i)
    {
      hist[i] = *p;
      p++;
    }
}




template <typename T>
typename SolverLinearPetsc<T>::real_type
SolverLinearPetsc<T>::getInitialResidual()
{
  int ierr = 0;
  int its  = 0;

  // Fill the residual history vector with the residual norms
  // Note that GetResidualHistory() does not copy any values, it
  // simply sets the pointer p.  Note that for some Krylov subspace
  // methods, the number of residuals returned in the history
  // vector may be different from what you are expecting.  For
  // example, TFQMR returns two residual values per iteration step.
  double* p;
  ierr = KSPGetResidualHistory(_M_ksp, &p, &its);
  CHKERRABORT(Application::COMM_WORLD,ierr);

  // Check no residual history
  if (its == 0)
    {
      std::cerr << "No iterations have been performed, returning 0." << std::endl;
      return 0.;
    }

  // Otherwise, return the value pointed to by p.
  return *p;
}




template <typename T>
void
SolverLinearPetsc<T>::setPetscSolverType()
{
  int ierr = 0;
  Debug(7010) << "[SolverLinearPetsc] solver type:  " << this->solverType() << "\n";
  switch (this->solverType())
    {

    case CG:
      ierr = KSPSetType (_M_ksp, (char*) KSPCG);         CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case CR:
      ierr = KSPSetType (_M_ksp, (char*) KSPCR);         CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case CGS:
      ierr = KSPSetType (_M_ksp, (char*) KSPCGS);        CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case BICG:
      ierr = KSPSetType (_M_ksp, (char*) KSPBICG);       CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case TCQMR:
      ierr = KSPSetType (_M_ksp, (char*) KSPTCQMR);      CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case TFQMR:
      ierr = KSPSetType (_M_ksp, (char*) KSPTFQMR);      CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case LSQR:
      ierr = KSPSetType (_M_ksp, (char*) KSPLSQR);       CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case BICGSTAB:
      ierr = KSPSetType (_M_ksp, (char*) KSPBCGS);       CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case MINRES:
      ierr = KSPSetType (_M_ksp, (char*) KSPMINRES);     CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case GMRES:
      ierr = KSPSetType (_M_ksp, (char*) KSPGMRES);      CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case RICHARDSON:
      ierr = KSPSetType (_M_ksp, (char*) KSPRICHARDSON); CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case CHEBYSHEV:
      ierr = KSPSetType (_M_ksp, (char*) KSPCHEBYCHEV);  CHKERRABORT(Application::COMM_WORLD,ierr); return;

    default:
      std::cerr << "ERROR:  Unsupported PETSC Solver: "
		<< this->solverType()               << std::endl
		<< "Continuing with PETSC defaults" << std::endl;
    }
}








template <typename T>
void
SolverLinearPetsc<T>::setPetscPreconditionerType()
{

  int ierr = 0;
  ierr = PCFactorSetMatSolverPackage(_M_pc,MAT_SOLVER_UMFPACK);
  if ( ierr )
  {
      ierr = PCFactorSetMatSolverPackage(_M_pc,MAT_SOLVER_SUPERLU );
      if ( ierr )
      {
          ierr = PCFactorSetMatSolverPackage(_M_pc,MAT_SOLVER_PETSC );
      }
  }
  Debug(7010) << "[SolverLinearPetsc] preconditioner type:  " << this->preconditionerType() << "\n";
  switch (this->preconditionerType())
    {
    case IDENTITY_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCNONE);      CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case CHOLESKY_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCCHOLESKY);  CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case ICC_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCICC);       CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case ILU_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCILU);       CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case LU_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCLU);        CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case ASM_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCASM);       CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case JACOBI_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCJACOBI);    CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case BLOCK_JACOBI_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCBJACOBI);   CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case SOR_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCSOR);       CHKERRABORT(Application::COMM_WORLD,ierr); return;

    case EISENSTAT_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCEISENSTAT); CHKERRABORT(Application::COMM_WORLD,ierr); return;

#if !((PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR <= 1) && (PETSC_VERSION_SUBMINOR <= 1))
    case USER_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCMAT);       CHKERRABORT(Application::COMM_WORLD,ierr); return;
#endif

    case SHELL_PRECOND:
      ierr = PCSetType (_M_pc, (char*) PCSHELL);     CHKERRABORT(Application::COMM_WORLD,ierr); return;

    default:
      std::cerr << "ERROR:  Unsupported PETSC Preconditioner: "
		<< this->preconditionerType()       << std::endl
		<< "Continuing with PETSC defaults" << std::endl;
    }

}




//------------------------------------------------------------------
// Explicit instantiations
template class SolverLinearPetsc<double>;

} // Life

#endif // #ifdef HAVE_PETSC

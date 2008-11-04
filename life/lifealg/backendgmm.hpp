/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christoph Winkelmann <christoph.winkelmann@epfl.ch>
             Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2006-11-16

  Copyright (C) 2006 EPFL
  Copyright (C) 2007 Universit� Joseph Fourier (Grenoble I)

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
   \file backendgmm.hpp
   \author Christoph Winkelmann <christoph.winkelmann@epfl.ch>
   \date 2006-11-16
 */

#ifndef _BACKENDGMM_HPP_
#define _BACKENDGMM_HPP_

#include <boost/program_options/variables_map.hpp>

#include <life/lifecore/application.hpp>
#include <life/lifealg/vectorublas.hpp>
#include <life/lifealg/matrixgmm.hpp>
#include <life/lifealg/matrixtriplet.hpp>

#include <gmm_dense_Householder.h>
#include <gmm_iter_solvers.h>
#include <gmm_superlu_interface.h>
#include <life/lifealg/solverumfpack.hpp>
#include <life/lifealg/backend.hpp>

namespace Life
{
namespace po = boost::program_options;

// struct to hold default values for Backendgmm
struct BackendGmmDefaults
{
    // Default constructor with default default values
    BackendGmmDefaults()
        :
        solver_type( "umfpack" ),
        pc_type( "ilut" ),
        threshold( 1e-3 ),
        fillin( 2 ),
        restart( 20 ),
        verbose( 0 ),
        maxiter( 1000 ),
        tolerance( 2e-10 )
    {}

    std::string solver_type;
    std::string pc_type;
    double threshold;
    int fillin;
    int restart;
    int verbose;
    int maxiter;
    double tolerance;
};

/**
 * \class BackendGmm
 *
 * this class provides an interface to the GMM linear algebra library
 */
template<typename T>
class BackendGmm : public Backend<T>
{
    typedef Backend<T> super;
public:

    // -- TYPEDEFS --
    typedef typename super::value_type value_type;

    /* defaults */
    typedef BackendGmmDefaults defaults_type;

    /* matrix */
    typedef typename super::sparse_matrix_type sparse_matrix_type;
    typedef typename super::sparse_matrix_ptrtype sparse_matrix_ptrtype;
    typedef MatrixGmm<value_type, gmm::row_major> gmm_sparse_matrix_type;
    typedef boost::shared_ptr<gmm_sparse_matrix_type> gmm_sparse_matrix_ptrtype;

    /* vector */
    typedef typename super::vector_type vector_type;
    typedef typename super::vector_ptrtype vector_ptrtype;
    typedef VectorUblas<value_type> gmm_vector_type;
    typedef boost::shared_ptr<vector_type> gmm_vector_ptrtype;

    typedef typename super::solve_return_type solve_return_type;
    typedef typename super::nl_solve_return_type nl_solve_return_type;

    // -- CONSTRUCTOR --
    BackendGmm();

    BackendGmm( po::variables_map const& vm, std::string const& prefix = "" );

    // -- FACTORY METHODS --
    template<typename DomainSpace, typename DualImageSpace>
    static sparse_matrix_ptrtype newMatrix( boost::shared_ptr<DomainSpace> const& space1,
                                            boost::shared_ptr<DualImageSpace> const& space2 )
    {
        return sparse_matrix_ptrtype( new gmm_sparse_matrix_type( space1->nDof(), space2->nDof() ) );
    }

    sparse_matrix_ptrtype
    newMatrix( DataMap const& d1, DataMap const& d2 )
    {
        return sparse_matrix_ptrtype( new gmm_sparse_matrix_type( d1.nGlobalElements(), d2.nGlobalElements() ) );
    }

    template<typename SpaceT>
    static vector_ptrtype newVector( boost::shared_ptr<SpaceT> const& space )
    {
        return vector_ptrtype( new gmm_vector_type( space->nDof() ) );
    }

    template<typename SpaceT>
    static vector_ptrtype newVector( SpaceT const& space )
    {
        return vector_ptrtype( new gmm_vector_type( space.nDof() ) );
    }

    vector_ptrtype
    newVector( DataMap const& d )
    {
        return vector_ptrtype( new gmm_vector_type( d.nGlobalElements() ) );
    }


    // -- SETTING OF OPTIONS --
    void set_noisy    ( int noisy )        { M_iter.set_noisy( noisy );     }
    void set_maxiter  ( int maxiter )      { M_iter.set_maxiter( maxiter ); }
    void set_fillin   ( int fillin )       { M_fillin    = fillin;          }
    void set_threshold( double threshold ) { M_threshold = threshold;       }
    void set_tol      ( double tol )       { M_iter.set_resmax( tol );      }

    void set_symmetric( bool isSymmetric )
    {
        M_isSymmetric = isSymmetric;
        if ( isSymmetric )
        {
            M_ilut.reset( 0 );
            M_ilutp.reset( 0 );
        }
    }

    void set_direct( bool isDirect )
    {
#if defined(GMM_USES_SUPERLU) || defined(HAVE_UMFPACK)
        M_isDirect = isDirect;
        if ( isDirect )
        {
            M_solver_type = "umfpack";
            M_ilut.reset( 0 );
            M_ilutp.reset( 0 );
        }
#else
        if ( isDirect )
            std::cerr << "[BackendGmm] direct solver is not available\n";
#endif
    }

    void set_solver_type( std::string const& solver )
    {
        M_solver_type = solver;
    }

    void set_preconditioner_type( std::string const& prec )
    {
        M_precond_type = prec;
    }

    void set_restart( int restart )
    {
        M_restart = restart;
    }

    // -- LINEAR ALGEBRA INTERFACE --
    void prod( sparse_matrix_type const& A,
               vector_type const& x,
               vector_type& b ) const
    {
        gmm_sparse_matrix_type const& _A = dynamic_cast<gmm_sparse_matrix_type const&>( A );
        gmm_vector_type const& _x = dynamic_cast<gmm_vector_type const&>( x );
        gmm_vector_type& _b = dynamic_cast<gmm_vector_type&>( b );
        gmm::mult( _A.mat(), _x.vec(), _b.vec());
    }

    solve_return_type solve( sparse_matrix_type const& A,
                             vector_type& x,
                             const vector_type& b );

    solve_return_type solve( sparse_matrix_ptrtype const& A,
                             vector_ptrtype& x,
                             const vector_ptrtype& b )
    {
        return this->solve( *A, *x, *b );
    }

    solve_return_type solve( sparse_matrix_ptrtype const& A,
                             sparse_matrix_ptrtype const& P,
                             vector_ptrtype& x,
                             const vector_ptrtype& b )
    {
        return this->solve( *A, *x, *b );
    }


    value_type dot( const gmm_vector_type& f,
                    const gmm_vector_type& x ) const
    {
        value_type result(0);
        typename gmm_vector_type::const_iterator fi;
        typename gmm_vector_type::const_iterator xi;
        for( fi=f.begin(), xi=x.begin();
             ( fi!=f.end() ) && ( xi!=x.end() );
             ++fi, ++xi )
            {
                result += (*xi) * (*fi);
            }
        return result;
    }

    // -- GETTING DETAILS ABOUT SOLVING --
    bool converged() { return M_iter.converged(); }

    size_type get_iteration() { return M_iter.get_iteration(); }

    boost::shared_ptr<MatrixTriplet<T> > toTriplet( sparse_matrix_type const& m );


private:

    /* typedefs preconditioners */
    typedef gmm::ilut_precond<typename gmm_sparse_matrix_type::matrix_type>
    ilut_type;
    typedef gmm::ilutp_precond<typename gmm_sparse_matrix_type::matrix_type>
    ilutp_type;
    typedef std::auto_ptr<ilut_type> ilut_ptrtype;
    typedef std::auto_ptr<ilutp_type> ilutp_ptrtype;
    typedef gmm::diagonal_precond<typename gmm_sparse_matrix_type::matrix_type>
    diagprec_type;

    /* private member data */
    gmm::iteration M_iter;
    int            M_fillin;
    double         M_threshold;
    bool           M_isSymmetric;
    bool           M_isDirect;
    ilut_ptrtype   M_ilut;
    ilutp_ptrtype  M_ilutp;
    std::string    M_solver_type;
    std::string    M_precond_type;
    int            M_restart;

#if defined( HAVE_UMFPACK )
    SolverUMFPACK  M_umfpack;
#endif // HAVE_UMFPACK

}; // class BackendGmm



po::options_description backendgmm_options( std::string const& prefix = "",
                                            BackendGmmDefaults defaults = BackendGmmDefaults() );

} // Life

#endif /* _BACKENDGMM_HPP_ */

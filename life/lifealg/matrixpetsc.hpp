/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2005-10-18

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2007 Université Joseph Fourier (Grenoble I)

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
   \file matrixpetsc.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2005-10-18
 */
#ifndef __MatrixPetsc_H
#define __MatrixPetsc_H 1

#include <lifeconfig.h>

#if defined(HAVE_PETSC_H)


#include <life/lifecore/application.hpp>

#include <life/lifealg/matrixsparse.hpp>
#include <life/lifealg/graphcsr.hpp>


extern "C"
{
#include <petscmat.h>
}


namespace Life
{
template<typename T> class VectorPetsc;

/**
 * \class MatrixPetsc
 * \brief Wrapper for petsc matrices
 *
 * Petsc matrix. Provides a nice interface to the
 * Petsc C-based data structures for parallel,
 * sparse matrices.
 *
 * @author Benjamin S. Kirk, 2002
 * @author Christophe Prud'homme
 * @see
 */
template<typename T>
class MatrixPetsc : public MatrixSparse<T>
{
    typedef MatrixSparse<T> super;
public:
    /** @name Typedefs
     */
    //@{

    static const bool is_row_major = true;

    //@{

    /** @name Typedefs
     */
    //@{

    typedef typename super::value_type value_type;
    typedef typename super::real_type real_type;

    typedef std::vector<std::set<size_type> > pattern_type;

    typedef typename super::graph_type graph_type;
    typedef typename super::graph_ptrtype graph_ptrtype;
    //@}

    /** @name Constructors, destructor
     */
    //@{

    /**
     * Constructor; initializes the matrix to
     * be empty, without any structure, i.e.
     * the matrix is not usable at all. This
     * constructor is therefore only useful
     * for matrices which are members of a
     * class. All other matrices should be
     * created at a point in the data flow
     * where all necessary information is
     * available.
     *
     * You have to initialize
     * the matrix before usage with
     * \p init(...).
     */
    MatrixPetsc();

    /**
     * Constructor.  Creates a PetscMatrix assuming you already
     * have a valid Mat object.  In this case, m is NOT destroyed
     * by the PetscMatrix destructor when this object goes out of scope.
     * This allows ownership of m to remain with the original creator,
     * and to simply provide additional functionality with the PetscMatrix.
     */
    MatrixPetsc (Mat m);

    /**
     * Destructor. Free all memory, but do not
     * release the memory of the sparsity
     * structure.
     */
    ~MatrixPetsc();

    //@}

    /** @name Operator overloads
     */
    //@{

    /**
     * Return the value of the entry \p (i,j).  This may be an
     * expensive operation and you should always take care where to
     * call this function.  In order to avoid abuse, this function
     * throws an exception if the required element does not exist in
     * the matrix.
     *
     * In case you want a function that returns zero instead (for
     * entries that are not in the sparsity pattern of the matrix),
     * use the \p el function.
     */
    value_type operator () (const size_type i,
                            const size_type j) const;



    //@}

    /** @name Accessors
     */
    //@{

    /**
     * @returns \p m, the row-dimension of
     * the matrix where the marix is \f$ M \times N \f$.
     */
    size_type size1 () const;

    /**
     * @returns \p n, the column-dimension of
     * the matrix where the marix is \f$ M \times N \f$.
     */
    size_type size2 () const;

    /**
     * return row_start, the index of the first
     * matrix row stored on this processor
     */
    size_type rowStart () const;

    /**
     * return row_stop, the index of the last
     * matrix row (+1) stored on this processor
     */
    size_type rowStop () const;

    //@}

    /** @name  Mutators
     */
    //@{


    //@}

    /** @name  Methods
     */
    //@{

    /**
     * Initialize a Petsc matrix that is of global
     * dimension \f$ m \times  n \f$ with local dimensions
     * \f$ m_l \times n_l \f$.  \p nnz is the number of on-processor
     * nonzeros per row (defaults to 30).
     * \p noz is the number of on-processor
     * nonzeros per row (defaults to 30).
     */
    void init (const size_type m,
               const size_type n,
               const size_type m_l,
               const size_type n_l,
               const size_type nnz=30,
               const size_type noz=10);

    /**
     * Initialize using sparsity structure computed by \p dof_map.
     */
    void init ( const size_type m,
                const size_type n,
                const size_type m_l,
                const size_type n_l,
                graph_ptrtype const& graph );

    /**
     * reinitialize the matrix
     */
    void resize( size_type m, size_type n, bool /*preserve*/ )
    {
        this->setInitialized( false );
        init( m, n, m, n );
    }

    void fill( pattern_type const& patt )
    {}
    /**
     * Release all memory and return
     * to a state just like after
     * having called the default
     * constructor.
     */
    void clear ();

    /**
     * Set all entries to 0. This method retains
     * sparsity structure.
     */
    void zero ();

    /**
     * Set all entries to 0 in the range
     * [start1-stop1,start2-stop2]. This method retains sparsity
     * structure.
     */
    void zero ( size_type start1, size_type stop1, size_type start2, size_type stop2 );

    /**
     * Call the Petsc assemble routines.
     * sends necessary messages to other
     * processors
     */
    void close () const;


    /**
     * see if Petsc matrix has been closed
     * and fully assembled yet
     */
    bool closed() const;

    /**
     * Return the l1-norm of the matrix, that is
     * \f$|M|_1=max_{all columns j}\sum_{all rows i} |M_ij|\f$, (max. sum of columns).
     *
     * This is the natural matrix norm that is compatible to the
     * l1-norm for vectors, i.e.  \f$|Mv|_1\leq |M|_1 |v|_1\f$.
     */
    real_type l1Norm () const;

    /**
     * Return the linfty-norm of the matrix, that is
     *
     * \f$|M|_\infty=max_{all rows i}\sum_{all columns j} |M_ij|\f$,
     *
     * (max. sum of rows).
     * This is the natural matrix norm that is
     * compatible to the linfty-norm of vectors, i.e.
     * \f$|Mv|_\infty \leq |M|_\infty |v|_\infty\f$.
     */
    real_type linftyNorm () const;

    /**
     * Set the element \p (i,j) to \p value.
     * Throws an error if the entry does
     * not exist. Still, it is allowed to store
     * zero values in non-existent fields.
     */
    void set (const size_type i,
              const size_type j,
              const value_type& value);

    /**
     * Add \p value to the element
     * \p (i,j).  Throws an error if
     * the entry does not
     * exist. Still, it is allowed to
     * store zero values in
     * non-existent fields.
     */
    void add (const size_type i,
              const size_type j,
              const value_type& value);

    /**
     * Add the full matrix to the
     * Petsc matrix.  This is useful
     * for adding an element matrix
     * at assembly time
     */
    void addMatrix (const ublas::matrix<value_type> &dm,
                    const std::vector<size_type> &rows,
                    const std::vector<size_type> &cols);

    /**
     * Same, but assumes the row and column maps are the same.
     * Thus the matrix \p dm must be square.
     */
    void addMatrix (const ublas::matrix<value_type> &dm,
                    const std::vector<size_type> &dof_indices)
    {
        this->addMatrix (dm, dof_indices, dof_indices);
    }

    /**
     * Add a Sparse matrix \p X, scaled with \p a, to \p this,
     * stores the result in \p this:
     * \f$\texttt{this} = a*X + \texttt{this} \f$.
     * Use this with caution, the sparse matrices need to have the
     * same nonzero pattern, otherwise \p PETSc will crash!
     * It is advisable to not only allocate appropriate memory with
     * \p init() , but also explicitly zero the terms of \p this
     * whenever you add a non-zero value to \p X.  Note: \p X will
     * be closed, if not already done, before performing any work.
     */
    void addMatrix (const T a, MatrixSparse<T> &X);

    void scale( const T a ){}
    /**
     * Returns the raw PETSc matrix context pointer.  Note this is generally
     * not required in user-level code. Just don't do anything crazy like
     * calling MatDestroy()!
     */
    Mat mat () const { LIFE_ASSERT (_M_mat != NULL).error("null petsc matrix"); return _M_mat; }

    /**
     * Print the contents of the matrix in Matlab's
     * sparse matrix format. Optionally prints the
     * matrix to the file named \p name.  If \p name
     * is not specified it is dumped to the screen.
     */
    void printMatlab(const std::string name="NULL") const;

    /**
     * \return \f$ v^T M u \f$
     */
    value_type
    energy( Vector<value_type> const& __v,
            Vector<value_type> const& __u ) const
    {
        VectorPetsc<T> const& v   = dynamic_cast<VectorPetsc<T> const&>( __v );
        VectorPetsc<T> const& u   = dynamic_cast<VectorPetsc<T> const&>( __u );
        VectorPetsc<value_type> z( __u.size(), __u.localSize() );
        MatMult( _M_mat, u.vec(), z.vec() );
        PetscScalar e;
        VecDot( v.vec(), z.vec(), &e );
        return e;
    }


    /**
     * eliminate row without change pattern, and put 1 on the diagonal
     * entry
     *
     *\warning if the matrix was symmetric before this operation, it
     * won't be afterwards. So use the proper solver (nonsymmetric)
     */
    void zeroRows( std::vector<int> const& rows, std::vector<value_type> const& values, Vector<value_type>& rhs, Context const& on_context );


    //@}

private:

    // disable
    MatrixPetsc( MatrixPetsc const & );

private:

    /**
     * Petsc matrix datatype to store values
     */
    Mat _M_mat;

    /**
     * This boolean value should only be set to false
     * for the constructor which takes a PETSc Mat object.
     */
    const bool _M_destroy_mat_on_exit;
};



} // Life
#endif /* HAVE_PETSC */
#endif /* __MatrixPetsc_H */

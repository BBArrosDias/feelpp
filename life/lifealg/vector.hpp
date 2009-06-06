// $Id: numeric_vector.h,v 1.11 2005/02/22 22:17:34 jwpeterson Exp $

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

#ifndef __numeric_vector_h__
#define __numeric_vector_h__

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <life/lifecore/traits.hpp>
#include <life/lifecore/application.hpp>

#include <life/lifealg/datamap.hpp>

namespace Life
{
namespace ublas = boost::numeric::ublas;

// forward declarations
template <typename T> class Vector;
template <typename T> class MatrixSparse;

/**
 * Numeric vector. Provides a uniform interface
 * to vector storage schemes for different linear
 * algebra libraries.
 *
 * @author Benjamin S. Kirk, 2003
 * @author Christophe Prud'homme 2005
 */
template <typename T>
class Vector
{
public:

    typedef T value_type;
    typedef typename type_traits<T>::real_type real_type;

    typedef Vector<T> self_type;
    typedef boost::shared_ptr<Vector<T> > self_ptrtype;
    typedef boost::shared_ptr<Vector<T> > clone_ptrtype;

    /**
     *  Dummy-Constructor. Dimension=0
     */
    Vector ();

    Vector ( DataMap const& n );

    /**
     * Constructor. Set dimension to \p n and initialize all elements with zero.
     */
    Vector (const size_type n);

    /**
     * Constructor. Set local dimension to \p n_local, the global dimension
     * to \p n, and initialize all elements with zero.
     */
    Vector (const size_type n,
            const size_type n_local);

    /**
     * Destructor, deallocates memory. Made virtual to allow
     * for derived classes to behave properly.
     */
    virtual ~Vector ();

    DataMap const& map() const { return M_map; }

    /**
     * @returns true if the vector has been initialized,
     * false otherwise.
     */
    virtual bool isInitialized() const { return M_is_initialized; }

    /**
     * @returns true if the vector is closed and ready for
     * computation, false otherwise.
     */
    virtual bool closed() const { return M_is_closed; }

    /**
     * Call the assemble functions
     */
    virtual void close () = 0;

    /**
     * @returns the \p Vector<T> to a pristine state.
     */
    virtual void clear ();

    /**
     * Set all entries to zero. Equivalent to \p v = 0, but more obvious and
     * faster.
     */
    virtual void zero () = 0;

    /**
     * Set entries to zero between \p start and \p stop
     */
    virtual void zero ( size_type /*start*/,  size_type /*stop*/ ) = 0;

    /**
     * Creates a copy of this vector and returns it in an \p shared_ptr<>.
     * This must be overloaded in the derived classes.
     */
    virtual clone_ptrtype clone () const = 0;

    /**
     * Change the dimension of the vector to \p N. The reserved memory for
     * this vector remains unchanged if possible, to make things faster, but
     * this may waste some memory, so take this in the back of your head.
     * However, if \p N==0 all memory is freed, i.e. if you want to resize
     * the vector and release the memory not needed, you have to first call
     * \p init(0) and then \p init(N). This cited behaviour is analogous
     * to that of the STL containers.
     *
     * On \p fast==false, the vector is filled by
     * zeros.
     */

    virtual void init (const size_type,
                       const size_type,
                       const bool = false);

    /**
     * call init with n_local = N,
     */
    virtual void init (const size_type,
                       const bool = false);

    //   /**
    //    * Change the dimension to that of the
    //    * vector \p V. The same applies as for
    //    * the other \p init function.
    //    *
    //    * The elements of \p V are not copied, i.e.
    //    * this function is the same as calling
    //    * \p init(V.size(),fast).
    //    */
    //   virtual void init (const Vector<T>&,
    // 		     const bool = false) {}

    /**
     * \f$U(0-N) = s\f$: fill all components.
     */
    virtual Vector<T> & operator= (const T s);

    /**
     *  \f$U = V\f$: copy all components.
     */
    virtual Vector<T> & operator= (const Vector<T> &V);

    /**
     *  \f$U = V\f$: copy all components.
     */
    virtual Vector<T> & operator= (const std::vector<T> &v);

    /**
     * \return the sum of the components of the vector
     */
    virtual value_type sum() const = 0;

    /**
     * @returns the minimum element in the vector.
     * In case of complex numbers, this returns the minimum
     * Real part.
     */
    virtual real_type min () const = 0;

    /**
     * @returns the maximum element in the vector.
     * In case of complex numbers, this returns the maximum
     * Real part.
     */
    virtual real_type max () const = 0;

    /**
     * @returns the \f$l_1\f$-norm of the vector, i.e.
     * the sum of the absolute values.
     */
    virtual real_type l1Norm () const = 0;

    /**
     * @returns the \f$l_2\f$-norm of the vector, i.e.
     * the square root of the sum of the
     * squares of the elements.
     */
    virtual real_type l2Norm () const = 0;

    /**
     * @returns the maximum absolute value of the
     * elements of this vector, which is the
     * \f$l_\infty\f$-norm of a vector.
     */
    virtual real_type linftyNorm () const = 0;

    /**
     * @returns dimension of the vector. This
     * function was formerly called \p n(), but
     * was renamed to get the \p Vector<T> class
     * closer to the C++ standard library's
     * \p std::vector container.
     */
    virtual size_type size () const
    {
        return M_map.nGlobalElements();
    }

    /**
     * @returns the local size of the vector
     * (index_stop-index_start)
     */
    virtual size_type localSize() const
    {
        return M_map.nMyElements();
    }

    /**
     * @returns the index of the first vector element
     * actually stored on this processor.  Hint: the
     * minimum for this index is \p 0.
     */
    virtual size_type firstLocalIndex() const
    {
        return M_map.minMyGID();
    }

    /**
     * @returns the index+1 of the last vector element
     * actually stored on this processor.  Hint: the
     * maximum for this index is \p size().
     */
    virtual size_type lastLocalIndex() const
    {
        return M_map.maxMyGID()+1;
    }


    /**
     * Access components, returns \p U(i).
     */
    virtual T operator() (const size_type i) const = 0;

    /**
     * Addition operator.
     * Fast equivalent to \p U.add(1, V).
     */
    virtual Vector<T> & operator += (const Vector<value_type> &V) = 0;

    /**
     * Subtraction operator.
     * Fast equivalent to \p U.add(-1, V).
     */
    virtual Vector<T> & operator -= (const Vector<value_type> &V) = 0;

    /**
     * v(i) = value
     */
    virtual void set (const size_type i, const value_type& value) = 0;

    /**
     * v(i) += value
     */
    virtual void add (const size_type i, const value_type& value) = 0;

    /**
     * \f$U(0-DIM)+=s\f$.
     * Addition of \p s to all components. Note
     * that \p s is a scalar and not a vector.
     */
    virtual void add (const value_type& s) = 0;

    /**
     * \f$U+=V\f$:
     * Simple vector addition, equal to the
     * \p operator +=.
     */
    virtual void add (const Vector<value_type>& V) = 0;

    /**
     * \f$U+=a*V\f$.
     * Simple vector addition, equal to the
     * \p operator +=.
     */
    virtual void add (const value_type& a, const Vector<value_type>& v) = 0;

    /**
     * \f$ U+=v \f$ where v is a ublas::vector<T>
     * and you
     * want to specify WHERE to add it
     */
    virtual void addVector (const std::vector<T>& v,
                            const std::vector<size_type>& dof_indices) = 0;

    /**
     * \f$U+=V\f$, where U and V are type
     * Vector<T> and you
     * want to specify WHERE to add
     * the Vector<T> V
     */
    virtual void addVector (const Vector<T>& V,
                            const std::vector<size_type>& dof_indices) = 0;

    /**
     * \f$U+=A*V\f$, add the product of a \p SparseMatrix \p A
     * and a \p Vector \p V to \p this, where \p this=U.
     */
    virtual void addVector (const Vector<T>&,
                            const MatrixSparse<T>&) = 0;

#if 0
    /**
     * \f$ U+=V \f$ where U and V are type
     * DenseVector<T> and you
     * want to specify WHERE to add
     * the DenseVector<T> V
     */
    virtual void addVector (const DenseVector<T>& V,
                            const std::vector<size_type>& dof_indices) = 0;
#endif

    /**
     * \f$ U=v \f$ where v is a DenseVector<T>
     * and you want to specify WHERE to insert it
     */
    virtual void insert (const std::vector<T>& v,
                         const std::vector<size_type>& dof_indices) = 0;

    /**
     * \f$U=V\f$, where U and V are type
     * Vector<T> and you
     * want to specify WHERE to insert
     * the Vector<T> V
     */
    virtual void insert (const Vector<T>& V,
                         const std::vector<size_type>& dof_indices) = 0;

    /**
     * \f$ U+=V \f$ where U and V are type
     * DenseVector<T> and you
     * want to specify WHERE to insert
     * the DenseVector<T> V
     */
    virtual void insert (const ublas::vector<T>& V,
                         const std::vector<size_type>& dof_indices) = 0;

    /**
     * Scale each element of the
     * vector by the given factor.
     */
    virtual void scale (const T factor) = 0;

    /**
     * Creates a copy of the global vector in the
     * local vector \p v_local.
     */
    virtual void localize (std::vector<T>& v_local) const = 0;

    /**
     * Same, but fills a \p Vector<T> instead of
     * a \p std::vector.
     */
    virtual void localize (Vector<T>& v_local) const = 0;

    /**
     * Creates a local vector \p v_local containing
     * only information relevant to this processor, as
     * defined by the \p send_list.
     */
    virtual void localize (Vector<T>& v_local,
                           const std::vector<size_type>& send_list) const = 0;

    /**
     * Updates a local vector with selected values from neighboring
     * processors, as defined by \p send_list.
     */
    virtual void localize (const size_type first_local_idx,
                           const size_type last_local_idx,
                           const std::vector<size_type>& send_list) = 0;

    /**
     * Creates a local copy of the global vector in
     * \p v_local only on processor \p proc_id.  By
     * default the data is sent to processor 0.  This method
     * is useful for outputting data from one processor.
     */
    virtual void localizeToOneProcessor (std::vector<T>& v_local,
                                         const size_type proc_id=0) const = 0;

    /**
     * @returns \p -1 when \p this is equivalent to \p other_vector,
     * up to the given \p threshold.  When differences occur,
     * the return value contains the first index where
     * the difference exceeded the threshold.  When
     * no threshold is given, the \p Application \p TOLERANCE
     * is used.
     */
    virtual int compare (const Vector<T> &other_vector,
                         const real_type threshold = 1e-10 ) const;



    /**
     * Prints the contents of the vector to the screen.
     */
    virtual void print(std::ostream& os=std::cout) const;

    /**
     * Same as above but allows you to use stream syntax.
     */
    friend std::ostream& operator << (std::ostream& os, const Vector<T>& v)
    {
        v.print(os);
        return os;
    }

    /**
     * Print the contents of the matrix in Matlab's
     * sparse matrix format. Optionally prints the
     * matrix to the file named \p name.  If \p name
     * is not specified it is dumped to the screen.
     */
    virtual void printMatlab(const std::string name="NULL") const
    {
        std::cerr << "ERROR: Not Implemented in base class yet!" << std::endl;
        std::cerr << "ERROR writing MATLAB file " << name << std::endl;
        LIFE_ASSERT( 0 ).error( "invalid call" );
    }

    /**
     * Creates the subvector "subvector" from the indices in the
     * "rows" array.  Similar to the create_submatrix routine for
     * the SparseMatrix class, it is currently only implemented for
     * PetscVectors.
     */
    virtual void createSubvector( Vector<T>& ,
                                  const std::vector<size_type>& ) const
    {
        std::cerr << "ERROR: Not Implemented in base class yet!" << std::endl;
        LIFE_ASSERT( 0 ).error( "invalid call" );
    }

protected:

    /**
     * Flag to see if the Numeric
     * assemble routines have been called yet
     */
    bool M_is_closed;

    /**
     * Flag to tell if init
     * has been called yet
     */
    bool M_is_initialized;

    /**
     * data distribution map of the vector over the processors
     */
    DataMap M_map;
};


/*----------------------- Inline functions ----------------------------------*/

/**
 * Computes the inner product of two vectors and eventually in parallel
 * \param v1 vector (eventually distributed)
 * \param v2 vector (eventually distributed)
 *
 * \return the inner product of \p v1 and \p v2
 */
template <typename T>
typename type_traits<T>::real_type
inner_product( Vector<T> const& v1, Vector<T> const& v2 )
{
    LIFE_ASSERT( v1.localSize() == v2.localSize() &&
                 v1.size() == v2.size() )
        ( v1.localSize() )( v2.localSize() )
        ( v1.size() )( v2.size() ).error( "incompatible vector sizes" );

    typedef typename type_traits<T>::real_type real_type;

    size_type s = v1.localSize();
    real_type res = 0;
    size_type start = v1.firstLocalIndex();
    for( size_type i = 0; i < s; ++i )
        res += v1( start + i )* v2( start + i );

    real_type global_res = res;
#if defined( HAVE_MPI )
    if ( Application::nProcess() > 1 )
        mpi::all_reduce( Application::comm(), res, global_res, std::plus<real_type>() );
#endif
    return global_res;
}







} // Life

#endif  // #ifdef __numeric_vector_h__

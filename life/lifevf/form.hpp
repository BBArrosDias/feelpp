/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2007-05-30

  Copyright (C) 2007 Universit� Joseph Fourier (Grenoble I)

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
   \file form.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2007-05-30
 */
#ifndef __Form_H
#define __Form_H 1

#include <life/lifecore/parameter.hpp>
#include <life/lifealg/vector.hpp>
#include <life/lifealg/matrixsparse.hpp>
#include <life/lifediscr/functionspace.hpp>
#include <life/lifevf/bilinearform.hpp>
#include <life/lifevf/linearform.hpp>

namespace Life
{
template<typename T> class Vector;

//
// free form functions
//
template<typename X1, typename X2>
inline
vf::detail::BilinearForm<X1, X2>
form( boost::shared_ptr<X1> const& __X1,
      boost::shared_ptr<X2> const& __X2,
      MatrixSparse<double>& __M,
      bool init = false,
      bool do_threshold = false,
      typename X1::value_type threshold = type_traits<double>::epsilon(),
      size_type pattern  = vf::DOF_PATTERN_COUPLED )
{
    return vf::detail::BilinearForm<X1, X2>( __X1, __X2, __M, init, do_threshold, threshold, pattern );
}

template<typename X1, typename RepType>
inline
vf::detail::LinearForm<X1, RepType, RepType>
form( boost::shared_ptr<X1> const& __X1,
      RepType& __M,
      bool init = false,
      bool do_threshold = false,
      typename X1::value_type threshold = type_traits<typename RepType::value_type>::epsilon() )
{
    return vf::detail::LinearForm<X1, RepType, RepType>( __X1, __M, init, do_threshold, threshold );
}


namespace detail
{
template <class T>
struct is_shared_ptr : mpl::false_ {};

template <class T>
struct is_shared_ptr<boost::shared_ptr<T> > : mpl::true_ {};

template <class MatrixType>
struct is_matrix_ptr : mpl::false_ {};

template <class MatrixType>
struct is_matrix_ptr<boost::shared_ptr<MatrixType> >
    :
        boost::is_base_of<MatrixSparse<typename MatrixType::value_type>,
                          MatrixType>
{};

template <class VectorType>
struct is_vector_ptr : mpl::false_ {};

template <class VectorType>
struct is_vector_ptr<boost::shared_ptr<VectorType> >
    :
        boost::is_base_of<Vector<typename VectorType::value_type>,
                          VectorType>
{};


template<typename FuncSpaceType>
struct is_function_space_ptr : mpl::false_ {};

template<typename FuncSpaceType>
struct is_function_space_ptr<boost::shared_ptr<FuncSpaceType> > : mpl::true_ {};
} // detail


/// \cond detail
template<typename Args>
struct compute_form1_return
{
#if 1
    typedef typename boost::remove_reference<typename parameter::binding<Args, tag::test>::type>::type::element_type test_type;
    typedef typename boost::remove_reference<typename parameter::binding<Args, tag::vector>::type>::type::element_type vector_type;
    typedef vf::detail::LinearForm<test_type,
                                   vector_type,
                                   vector_type> type;
#else
    typedef typename parameter::value_type<Args, tag::test>::type test_type;
    typedef typename parameter::value_type<Args, tag::vector>::type vector_type;


    typedef vf::detail::LinearForm<test_type,vector_type,vector_type> type;
#endif
};
/// \endcond
//boost::is_convertible<mpl::_,boost::shared_ptr<FunctionSpaceBase> >
BOOST_PARAMETER_FUNCTION(
                         (typename compute_form1_return<Args>::type), // 1. return type
                         form1,                                       // 2. name of the function template
                         tag,                                        // 3. namespace of tag types
                         (required                                   // 4. one required parameter, and
                          (test,             *(boost::is_convertible<mpl::_,boost::shared_ptr<FunctionSpaceBase> >))
                          (in_out(vector),   *( detail::is_vector_ptr<mpl::_> ) ) ) // required
                         (optional                                   //    four optional parameters, with defaults
                          (init,             *(boost::is_integral<mpl::_>), false )
                          (do_threshold,     *(boost::is_integral<mpl::_>), bool(false) )
                          (threshold,        *(boost::is_floating_point<mpl::_>), type_traits<double>::epsilon() )
                          )
                         )
{
    //Life::detail::ignore_unused_variable_warning(boost_parameter_enabler_argument);
    Life::detail::ignore_unused_variable_warning(args);
    //return form( test, *vector, init, false, 1e-16 );
    return form( test, *vector, init, do_threshold, threshold );
} // form

BOOST_PARAMETER_FUNCTION(
                         (typename compute_form1_return<Args>::type), // 1. return type
                         lform,                                       // 2. name of the function template
                         tag,                                        // 3. namespace of tag types
                         (required                                   // 4. one required parameter, and
                          (test,             *(boost::is_convertible<mpl::_,boost::shared_ptr<FunctionSpaceBase> >))
                          (in_out(vector),   *( detail::is_vector_ptr<mpl::_> ) ) ) // required
                         (optional                                   //    four optional parameters, with defaults
                          (init,             *(boost::is_integral<mpl::_>), false )
                          (do_threshold,     *(boost::is_integral<mpl::_>), bool(false) )
                          (threshold,        *(boost::is_floating_point<mpl::_>), type_traits<double>::epsilon() )
                          )
                         )
{
    //return form( test, *vector, init, false, 1e-16 );
    return form( test, *vector, init, do_threshold, threshold );
} // form

/// \cond detail
template<typename Args, typename T>
struct compute_form2_return
{};

template<typename Args>
struct compute_form2_return<Args, mpl::false_>
{
    typedef typename parameter::value_type<Args, tag::test>::type::element_type::value_type value_type;
    typedef vf::detail::BilinearForm<typename parameter::value_type<Args, tag::test>::type::element_type,
                                     typename parameter::value_type<Args, tag::trial>::type::element_type,
                                     //typename parameter::value_type<Args, tag::matrix>::type::element_type,
                                     VectorUblas<value_type> > type;
};
template<typename Args>
struct compute_form2_return<Args, mpl::true_>
{
    typedef typename parameter::value_type<Args, tag::test>::type::element_type::value_type value_type;
    typedef vf::detail::BilinearForm<typename parameter::value_type<Args, tag::test>::type::element_type,
                                     typename parameter::value_type<Args, tag::test>::type::element_type,
                                     //typename parameter::value_type<Args, tag::vector>::type::element_type,
                                     VectorUblas<value_type> > type;
};
/// \endcond

#if 0
BOOST_PARAMETER_FUNCTION(
                         (typename compute_form2_return<Args,mpl::bool_<boost::is_same<typename parameter::value_type<Args, tag::trial>::type, boost::parameter::void_>::value> >::type), // 1. return type
                         form2,                                       // 2. name of the function template
                         tag,                                        // 3. namespace of tag types
                         (required                                   // 4. one required parameter, and
                          (test,             *(boost::is_convertible<mpl::_,boost::shared_ptr<FunctionSpaceBase> >))
                          (trial,            *(boost::is_convertible<mpl::_,boost::shared_ptr<FunctionSpaceBase> >))
                          (in_out(matrix),   *( detail::is_matrix_ptr<mpl::_> ) ) ) // required
                         (optional                                   //    four optional parameters, with defaults
                          (init,             *(boost::is_integral<mpl::_>), false )
                          (do_threshold,     *(boost::is_integral<mpl::_>), bool(false) )
                          (threshold,        *(boost::is_floating_point<mpl::_>), type_traits<double>::epsilon() )
                          (pattern,          *(boost::is_integral<mpl::_>), size_type(vf::DOF_PATTERN_COUPLED) )
                          (do_threshold,     *(boost::is_integral<mpl::_>), bool(false) )
                          (threshold,        *(boost::is_floating_point<mpl::_>), type_traits<double>::epsilon() )
                          )
                         )
{
    Life::detail::ignore_unused_variable_warning(args);
    //return form( test, trial, *matrix, init, false, 1e-16, pattern );
    return form( test, trial, *matrix, init, do_threshold, threshold, pattern );
    //return form( test, trial, *matrix, init, false, threshold, pattern );
    //return form( test, trial, *matrix, init, false, threshold, 0 );
} //
#else
BOOST_PARAMETER_FUNCTION((typename compute_form2_return<Args,mpl::bool_<boost::is_same<typename parameter::value_type<Args, tag::trial>::type, boost::parameter::void_>::value> >::type), // 1. return type
                         form2,                                       // 2. name of the function template
                         tag,                                        // 3. namespace of tag types
                         (required                                   // 4. one required parameter, and
                          (test,             *)
                          (trial,            *)
                          (in_out(matrix),   *)
                          ) // required
                         (optional                                   //    four optional parameters, with defaults
                          (init,             *(boost::is_integral<mpl::_>), false )
                          (pattern,          *(boost::is_integral<mpl::_>), size_type(vf::DOF_PATTERN_COUPLED) )
                          ) // optional
                         )
{
    Life::detail::ignore_unused_variable_warning(args);
    //return form( test, trial, *matrix, init, false, 1e-16, pattern );
    bool do_threshold = false;
    double threshold = 1e-16;
    return form( test, trial, *matrix, init, do_threshold, threshold, pattern );
    //return form( test, trial, *matrix, init, false, threshold, pattern );
    //return form( test, trial, *matrix, init, false, threshold, 0 );
} //

#endif

#if 0
BOOST_PARAMETER_FUNCTION(
                         (typename compute_form2_return<Args,mpl::bool_<boost::is_same<typename parameter::value_type<Args, tag::trial>::type, boost::parameter::void_>::value> >::type), // 1. return type
                         blform,                                       // 2. name of the function template
                         tag,                                        // 3. namespace of tag types
                         (required                                   // 4. one required parameter, and
                          (test,             *(boost::is_convertible<mpl::_,boost::shared_ptr<FunctionSpaceBase> >))
                          (trial,            *(boost::is_convertible<mpl::_,boost::shared_ptr<FunctionSpaceBase> >))
                          (in_out(matrix),   *( detail::is_matrix_ptr<mpl::_> ) ) ) // required
                         (optional                                   //    four optional parameters, with defaults
                          (init,             *(boost::is_integral<mpl::_>), false )
                          (do_threshold,     *(boost::is_integral<mpl::_>), bool(false) )
                          (threshold,        *(boost::is_floating_point<mpl::_>), type_traits<double>::epsilon() )
                          (pattern,          *(boost::is_integral<mpl::_>), size_type(vf::DOF_PATTERN_COUPLED) )
                          )
                         )
{
    return form( test, trial, *matrix, init, do_threshold, threshold, pattern );
    //return form( test, trial, *matrix, init, false, 1e-16, pattern );
} //
#endif



} // Life
#endif /* __Form_H */

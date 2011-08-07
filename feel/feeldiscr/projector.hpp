/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*-

  This file is part of the Feel library

  Author(s): Vincent Doyeux <vincent.doyeux@ujf-grenoble.fr>
       Date: 2011-04-25

  Copyright (C) 2011 Université Joseph Fourier (Grenoble I)

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
   \file projector.cpp
   \author Vincent Doyeux <vincent.doyeux@ujf-grenoble.fr>
   \date 2011-04-25
 */
#ifndef _PROJECTOR_HPP_
#define _PROJECTOR_HPP_

#include <feel/feeldiscr/operatorlinear.hpp>
#include <feel/feelvf/vf.hpp>

namespace Feel
{

    enum ProjectorType{L2=0, H1=1, DIFF=2};

/**
 * \class Projector
 * \brief Projection made easy
 *
 * @author Vincent Doyeux
 * @see OperatorLinear
 */
template<class DomainSpace, class DualImageSpace>
class Projector : public OperatorLinear<DomainSpace, DualImageSpace>
{
    typedef Projector<DomainSpace,DualImageSpace> super;

public :

    /** @name Typedefs
     */
    //@{

    // typedef Operator<DomainSpace, DualImageSpace> super_type;
    typedef OperatorLinear<DomainSpace, DualImageSpace> ol_type;

    typedef typename super::domain_space_type domain_space_type;
    typedef typename super::dual_image_space_type  dual_image_space_type;
    typedef typename super::domain_space_ptrtype domain_space_ptrtype;
    typedef typename super::dual_image_space_ptrtype  dual_image_space_ptrtype;
    typedef typename domain_space_type::element_type domain_element_type;
    typedef typename dual_image_space_type::element_type dual_image_element_type;

    typedef typename super::backend_type backend_type;
    typedef typename super::backend_ptrtype backend_ptrtype;
    typedef typename backend_type::sparse_matrix_type matrix_type;
    typedef typename backend_type::vector_type vector_type;
    typedef typename backend_type::vector_ptrtype vector_ptrtype;
    typedef boost::shared_ptr<matrix_type> matrix_ptrtype;

    typedef FsFunctionalLinear<DualImageSpace> image_element_type;

    //@}
    /** @name Constructors, destructor
     */
    //@{

    Projector(domain_space_ptrtype     domainSpace,
              dual_image_space_ptrtype dualImageSpace,
              backend_ptrtype backend = Backend<double>::build(BACKEND_PETSC),
              ProjectorType proj_type=L2,
              double epsilon = 0.01,
              double gamma = 0.5
                )
        :
        ol_type(domainSpace, dualImageSpace, backend),
        M_backend(backend),
        M_epsilon(epsilon),
        M_gamma(gamma),
        M_proj_type(proj_type),
        M_matrix(M_backend->newMatrix( domainSpace, dualImageSpace ))
        {  initMatrix();  }

    ~Projector() {}
    //@}

    /** @name  Methods
     */
    //@{
    template<typename Args,typename IntEltsDefault>
    struct integrate_type
    {
        typedef typename vf::detail::clean_type<Args,tag::expr>::type _expr_type;
        typedef typename vf::detail::clean2_type<Args,tag::range,IntEltsDefault>::type _range_type;
        //typedef _Q< ExpressionOrder<_range_type,_expr_type>::value > the_quad_type;
        typedef typename vf::detail::clean2_type<Args,tag::quad, _Q< vf::ExpressionOrder<_range_type,_expr_type>::value > >::type _quad_type;
        typedef typename vf::detail::clean2_type<Args,tag::quad1, _Q< vf::ExpressionOrder<_range_type,_expr_type>::value_1 > >::type _quad1_type;
    };

    BOOST_PARAMETER_MEMBER_FUNCTION((domain_element_type),
                                    project,
                                    tag,
                                    (required
                                     (expr,   *))
                                    (optional
                                     (range,   *, elements(this->domainSpace()->mesh()))
                                     (quad,   *, (typename integrate_type<Args,decltype(elements(this->domainSpace()->mesh()))>::_quad_type()) )
                                     (quad1,   *, (typename integrate_type<Args,decltype(elements(this->domainSpace()->mesh()))>::_quad1_type()) )
                                     (geomap, *, GeomapStrategyType::GEOMAP_OPT )
                                        ))
        {
            using namespace vf;
            domain_element_type de = this->domainSpace()->element();

            auto ie = M_backend->newVector(this->dualImageSpace());
            form1(_test=this->dualImageSpace(), _vector=ie, _init=true) =
                integrate(_range=range, _expr=expr * id( this->dualImageSpace()->element() ),
                          _quad=quad, _quad1=quad1, _geomap=geomap );

            //weak boundary conditions
            if (M_proj_type == DIFF)
                {
                    form1(_test=this->dualImageSpace(), _vector=ie) +=
                        integrate(_range=boundaryfaces(this->domainSpace()->mesh()),
                                  _expr=expr*M_epsilon*(-grad(this->domainSpace()->element() )*vf::N() +
                                                        M_gamma / vf::hFace() *id( this->dualImageSpace()->element() ) ),
                                  _quad=quad );
                }

            M_backend->solve(M_matrix, de, ie);

            return de;
        }


    template<typename RhsExpr>
    domain_element_type
    operator()(RhsExpr const& rhs_expr)
        {   return this->project(rhs_expr);   }



    template<typename RhsExpr>
    void
    operator()(domain_element_type& de, RhsExpr const& rhs_expr)
        {   de = this->project(rhs_expr);   }



    domain_element_type
    operator()(image_element_type const& ie)
        {
            domain_element_type de = this->domainSpace()->element();
            M_backend->solve(M_matrix, de, ie);
            return de ;
        }



    void
    operator()(domain_element_type &de, image_element_type const& ie)
        {   M_backend->solve(M_matrix, de, ie);  }



    void
    apply(domain_element_type& de,
          image_element_type const& ie)
        {     M_backend->solve(M_matrix, de, ie);    }



    template<typename RhsExpr>
    void
    apply(domain_element_type& de,
          RhsExpr const& rhs_expr)
        {    de=this->project(rhs_expr);  }

    //@}


private :

    void initMatrix()
        {
            using namespace vf;
            if (M_proj_type == L2)
                {
                    form2 (_trial=this->domainSpace(),
                           _test=this->dualImageSpace(),
                           _matrix=M_matrix,
                           _init=true) =
                        integrate(elements(this->domainSpace()->mesh()),
                                  trans(idt( this->domainSpace()->element() )) /*trial*/
                                  *id( this->domainSpace()->element() ) /*test*/
                                  );
                }
            else if (M_proj_type == H1)
                {
                    form2 (_trial=this->domainSpace(),
                           _test=this->dualImageSpace(),
                           _matrix=M_matrix,
                           _init=true) =
                        integrate(elements(this->domainSpace()->mesh()),
                                  trans(idt( this->domainSpace()->element() )) /*trial*/
                                  *id( this->domainSpace()->element() ) /*test*/
                                  +
                                  trace( gradt(this->domainSpace()->element())
                                         * trans(grad(this->domainSpace()->element())))
                                  );
                }
            else if (M_proj_type == DIFF)
                {
                    form2 (_trial=this->domainSpace(),
                           _test=this->dualImageSpace(),
                           _matrix=M_matrix,
                           _init=true) =
                        integrate(elements(this->domainSpace()->mesh()),
                                  trans(idt( this->domainSpace()->element() )) /*trial*/
                                  *id( this->domainSpace()->element() ) /*test*/
                                  +
                                  M_epsilon *
                                  trace( gradt(this->domainSpace()->element())
                                         * trans(grad(this->domainSpace()->element())))
                                  );
                    //weak boundary conditions
                    form2 (_trial=this->domainSpace(),
                           _test=this->dualImageSpace(),
                           _matrix=M_matrix) +=
                        integrate( boundaryfaces(this->domainSpace()->mesh()),
                                   M_epsilon*(-trans(id(this->domainSpace()->element() ))*gradt(this->domainSpace()->element())*vf::N()
                                              -trans(idt(this->domainSpace()->element() ))* grad(this->domainSpace()->element())*vf::N()
                                              + M_gamma * trans(idt( this->domainSpace()->element() )) /*trial*/
                                              *id( this->domainSpace()->element() ) / vf::hFace()   /*test*/
                                       ));
                }

            M_matrix->close();
        }

    backend_ptrtype M_backend;
    matrix_ptrtype M_matrix;
    const double M_epsilon;
    const double M_gamma;
    ProjectorType M_proj_type;

};//Projector

#if 1
/**
 * this function returns a \c Projector \c shared_ptr with
 *
 * \param domainSpace
 * \param imageSpace
 * \param backend
 */

template<typename TDomainSpace, typename TDualImageSpace>
boost::shared_ptr< Projector<TDomainSpace, TDualImageSpace> >
projector( boost::shared_ptr<TDomainSpace> const& domainspace,
             boost::shared_ptr<TDualImageSpace> const& imagespace,
             typename Projector<TDomainSpace, TDualImageSpace>::backend_ptrtype const& backend = Backend<double>::build(BACKEND_PETSC),
           ProjectorType proj_type=L2, double epsilon=0.01, double gamma = 0.5)
{
    typedef Projector<TDomainSpace, TDualImageSpace> Proj_type;
    boost::shared_ptr<Proj_type> proj( new Proj_type(domainspace, imagespace, backend, proj_type, epsilon, gamma) );
    return proj;
}

#endif



} //namespace Feel


#endif

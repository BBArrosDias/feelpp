/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2008-01-04

  Copyright (C) 2008 Christophe Prud'homme
  Copyright (C) 2008 Universit� Joseph Fourier (Grenoble I)

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
   \file elaxi.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2008-01-04
 */
#include <life/options.hpp>
#include <life/lifecore/application.hpp>
#include <life/lifediscr/functionspace.hpp>
#include <life/lifepoly/im.hpp>
#include <life/lifealg/backend.hpp>

#include <life/lifefilters/importergmsh.hpp>
#include <life/lifefilters/exporter.hpp>
#include <life/lifefilters/gmshtensorizeddomain.hpp>
#include <life/lifepoly/polynomialset.hpp>

#include <life/lifevf/vf.hpp>

inline
Life::po::options_description
makeOptions()
{
    Life::po::options_description elaxioptions("Elaxi options");
    elaxioptions.add_options()
        ("hsize", Life::po::value<double>()->default_value( 0.25 ), "first h value to start convergence")
        ("bctype", Life::po::value<int>()->default_value( 1 ), "0 = strong Dirichlet, 1 = weak Dirichlet")
        ("bccoeff", Life::po::value<double>()->default_value( 1.0e+5 ), "coeff for weak Dirichlet conditions")
        ("export", "export results(ensight, data file(1D)")
        ("export-matlab", "export matrix and vectors in matlab" )
        ;
    return elaxioptions.add( Life::life_options() ) ;
}
inline
Life::AboutData
makeAbout()
{
    Life::AboutData about( "elaxi" ,
                           "elaxi" ,
                           "0.1",
                           "Elasticity axisym  on simplices or simplex products",
                           Life::AboutData::License_GPL,
                           "Copyright (c) 2007 University Joseph Fourier Grenoble 1");

    about.addAuthor("Christophe Prud'homme", "developer", "christophe.prudhomme@ujf-grenoble.fr", "");
    about.addAuthor("Vuk Milisic", "developer", "vuk.milisic@imag.fr", "");
   return about;

}


namespace Life
{
template<typename A, uint16_type i>
class mytag : public A
{
public:
    static const uint16_type TAG = i;

};
/**
 * Diffussion Advection Reaction Solver
 *
 * solve \f$-\epsilon \Delta u -\beta\cdot\nabla u + \mu u = f\f$ on \f$\Omega\f$ and \f$u= g\f$ on \f$\Gamma_{in}\f$
 */
template<int Order,
         template<uint16_type,uint16_type,uint16_type> class Entity = Simplex>
class Elaxi
    :
        public Application
{
    typedef Application super;
public:
    static const uint16_type Dim=2;
    // -- TYPEDEFS --
    static const uint16_type imOrder = 4*Order;

    typedef double value_type;

    typedef Application application_type;
    typedef Backend<value_type> backend_type;
    typedef boost::shared_ptr<backend_type> backend_ptrtype;

    /*matrix*/
    typedef typename backend_type::sparse_matrix_ptrtype sparse_matrix_ptrtype;
    typedef typename backend_type::vector_ptrtype vector_ptrtype;

    /*mesh*/
    typedef Entity<Dim,1,Dim> entity_type;
    typedef Mesh<entity_type> mesh_type;
    typedef boost::shared_ptr<mesh_type> mesh_ptrtype;

    /*basis*/
    //typedef mytag<fem::Lagrange<Dim, Order, Vectorial, Continuous, double, Entity>,0> basis_u_type;
    typedef mytag<Lagrange<Order, Scalar>,0> basis_scalar_type_0;
    typedef mytag<Lagrange<Order, Scalar>,1> basis_scalar_type_1;
    typedef fusion::vector<basis_scalar_type_0,basis_scalar_type_1> basis_type;
    /*space*/
    typedef FunctionSpace<mesh_type, basis_type, value_type> space_type;
    typedef boost::shared_ptr<space_type> space_ptrtype;
    typedef typename space_type::element_type element_type;
    typedef typename element_type::template sub_element<0>::type element_0_type;
    typedef typename element_type::template sub_element<1>::type element_1_type;


    /* export */
    typedef Exporter<mesh_type> export_type;
    typedef typename Exporter<mesh_type>::timeset_type timeset_type;


    Elaxi( int argc, char** argv, AboutData const& ad, po::options_description const& od )
        :
        super( argc, argv, ad, od ),
        M_backend( backend_type::build( this->vm() )),
        meshSize( this->vm()["hsize"].template as<double>() ),
        bcCoeff( this->vm()["bccoeff"].template as<double>() ),
        exporter( Exporter<mesh_type>::New( this->vm()["exporter"].template as<std::string>() )->setOptions( this->vm() ) ),
        timeSet( new timeset_type( "elaxi" ) ),
        timers(),
        stats()
    {
        Log() << "[Elaxi] hsize = " << meshSize << "\n";
        Log() << "[Elaxi] bccoeff = " << bcCoeff << "\n";
        Log() << "[Elaxi] export = " << this->vm().count("export") << "\n";

        timeSet->setTimeIncrement( 1.0 );
        exporter->addTimeSet( timeSet );
    }


    /**
     * create the mesh using mesh size \c meshSize
     */
    mesh_ptrtype createMesh( double meshSize );


    /**
     * run the convergence test
     */
    void run();

private:

    /**
     * solve system
     */
    void solve( sparse_matrix_ptrtype const& D, element_type& u, vector_ptrtype const& F, bool is_sym );

    /**
     * export results to ensight format (enabled by  --export cmd line options)
     */
    void exportResults( double ,element_type& u );

private:

    backend_ptrtype M_backend;

    double meshSize;
    double bcCoeff;

    boost::shared_ptr<export_type> exporter;
    typename export_type::timeset_ptrtype timeSet;

    std::map<std::string,std::pair<boost::timer,double> > timers;
    std::map<std::string,double> stats;
}; // Elaxi
template<int Order, template<uint16_type,uint16_type,uint16_type> class Entity>
typename Elaxi<Order,Entity>::mesh_ptrtype
Elaxi<Order,Entity>::createMesh( double meshSize )
{
    timers["mesh"].first.restart();
    mesh_ptrtype mesh( new mesh_type );

    GmshTensorizedDomain<entity_type::nDim,entity_type::nOrder,entity_type::nRealDim,Entity> td;
    td.setCharacteristicLength( meshSize );
    td.setY(std::pair<double,double>(1,2));
    std::string fname = td.generate( entity_type::name().c_str() );

    ImporterGmsh<mesh_type> import( fname );
    mesh->accept( import );
    timers["mesh"].second = timers["mesh"].first.elapsed();
    Log() << "[timer] createMesh(): " << timers["mesh"].second << "\n";
    return mesh;
} // Elaxi::createMesh


template<int Order, template<uint16_type,uint16_type,uint16_type> class Entity>
void
Elaxi<Order, Entity>::run()
{
    if ( this->vm().count( "help" ) )
        {
            std::cout << this->optionsDescription() << "\n";
            return;
        }

    //    int maxIter = 10.0/meshSize;
    using namespace Life::vf;

    this->changeRepository( boost::format( "%1%/%2%/P%3%/h_%4%/" )
                            % this->about().appName()
                            % entity_type::name()
                            % Order
                            % this->vm()["hsize"].template as<double>()
                            );
    /*
     * logs will be in <life repo>/<app name>/<entity>/P<p>/h_<h>
     */
    this->setLogs();

    /*
     * First we create the mesh
     */
    mesh_ptrtype mesh = createMesh( meshSize );
    stats["nelt"] = mesh->elements().size();


    /*
     * The function space and some associate elements are then defined
     */
    timers["init"].first.restart();
    space_ptrtype Xh = space_type::New( mesh );

    //Xh->dof()->showMe();
    element_type U( Xh, "u" );
    element_type Uk( Xh, "uk" );
    element_type V( Xh, "v" );
    element_type Phi( Xh, "Phi" );
    Uk.zero();
    element_type poids(Xh, "p");

    element_0_type u0 = U.template element<0>();
    element_0_type v0 = V.template element<0>();
    element_0_type phi0 = Phi.template element<0>();

    element_1_type u1 = U.template element<1>();
    element_1_type v1 = V.template element<1>();
    element_1_type phi1 = Phi.template element<1>();


    phi0=project(Xh->template functionSpace<0>(),elements(mesh), Py());
    phi1.zero();




    uint16_type counter_it_newt=0;
    uint16_type max_it_newt=10000;
    Vector<double>::real_type error=1.0e+10;


    /*
     * Data associated with the simulation
     */
    const double tol = 1e-5;
    const double E = 21*1e5;
    const double sigma = 0.28;
    const double mu = E/(2*(1+sigma));
    const double lambda = E*sigma/((1+sigma)*(1-2*sigma));
    const double density = 50;
    //    const double gravity = -density*9.81;
    const double gravity = -1.0;
    Log() << "lambda = " << lambda << "\n"
          << "mu     = " << mu << "\n"
          << "gravity= " << gravity << "\n";
    std::cout << "lambda = " << lambda << "\n"
              << "mu     = " << mu << "\n"
              << "gravity= " << gravity << "\n";
    /*
     * Construction of the constant right hand side
     *
     * \f$ f = \int_\Omega g * v \f$ where \f$ g \f$ is a vector
     * directed in the \f$ z \f$ direction.
     */

    vector_ptrtype rhs( M_backend->newVector( Xh ) );
    vector_ptrtype vec_gravity( M_backend->newVector( Xh ) );
    vector_ptrtype newt_nl_source_term( M_backend->newVector( Xh ) );
    newt_nl_source_term->zero();


    timers["init"].second = timers["init"].first.elapsed();
    stats["ndof"] = Xh->nDof();


    Log() << "Data Summary:\n";
    size_type pattern = DOF_PATTERN_COUPLED;

    timers["assembly"].first.restart();
    sparse_matrix_ptrtype D( M_backend->newMatrix( Xh, Xh ) );
    std::cout << "====================Newton========================\n---->Start\n";
    Log() << "====================Newton========================\n---->Start\n";

    while ((error>tol) && (counter_it_newt <max_it_newt)){

        std::cout << "iteration #" << counter_it_newt << "\n";
        std::cout << "error=     " << error << "\n";
        std::cout << "==================================================\n\n";

        Log()<<  "iteration #" << counter_it_newt << "\n";
        Log()<<  "error=     " << error << "\n";
        Log()<<  "==================================================\n\n";




        timers["assembly"].first.restart();

        //size_type pattern = DOF_PATTERN_COUPLED|DOF_PATTERN_NEIGHBOR;
        form2( Xh, Xh, D, _init=true, _pattern=pattern ) =
            integrate( elements(mesh), _Q<2*Order>(), 2.0*(
                                                              //idt(u1)*id(v1)/Py()
                                                              idt(u0)*id(v0)/Py()
                                                              +dyt(u0)*dy(v0)*Py()
                                                              +dxt(u0)*dx(v0)*Py()
                                                              +dyt(u1)*dy(v1)*Py()
                                                              +dxt(u1)*dx(v1)*Py()
                                                              ));


        Log() << "[elaxi] matrix local assembly done\n";
        D->close();
        Log() << "[elaxi] vector/matrix global assembly done\n";


        /*
         * Construction of the variable right hand side
         *  (depending on the newton iterations)
         */
        //(*rhs)=(*vec_gravity);
        (*rhs).zero();
#if 1
        std::cout << "phi0"<< phi0.l2Norm()<<"\n";
        //      phi0.print();
        std::cout << "phi1"<< phi1.l2Norm()<<"\n";
        //      Phi.print();
#endif

        form1( Xh, newt_nl_source_term,_init=true ) =
            integrate( elements(mesh), _Q<2*Order-1>(),  2.0*(
                                                              //idv(phi1)*id(v1)/Py()
                                                              dyv(phi1)*dy(v1)*val(Py())
                                                              ));

#if 0
        newt_nl_source_term->print();
#endif
        rhs->add(-1.0,*newt_nl_source_term);
        rhs->close();

        std::cout << "rhs->l2Norm= " << rhs->l2Norm() << "\n";

        std::cout << "----> Block marked dofs\n";

        form2( Xh, Xh, D ) += on( boundaryfaces(mesh), u0, *rhs, constant(0))+
            on( boundaryfaces(mesh), u1, *rhs, constant(0));

        std::cout << "rhs->l2Norm= " << rhs->l2Norm() << "\n";

        error=rhs->l2Norm();


        Log() << "[elaxi] starting solve for D\n";
        this->solve( D, U, rhs, true );
        std::cout << "rhs->l2Norm= " << rhs->l2Norm() << "\n";

        u1.zero();

        Log() << "[elaxi] solve for D done\n";


        error=rhs->l2Norm();


        /*
          Preparing data for the next step
        */
        std::cout << "U->l2Norm= " << U.l2Norm() << "\n";
        std::cout << "Uk->l2Norm= " << Uk.l2Norm() << "\n";
        Phi+=U;
        Uk+=U;
        counter_it_newt++;
        this->exportResults( counter_it_newt, Uk );
    } //while



} //run

template<int Order, template<uint16_type,uint16_type,uint16_type> class Entity>
void
Elaxi<Order, Entity>::solve( sparse_matrix_ptrtype const& D,
                             element_type& u,
                             vector_ptrtype const& F,
                             bool is_sym )
{
    timers["solver"].first.restart();

    vector_ptrtype U( M_backend->newVector( u.functionSpace() ) );
    M_backend->solve( D, D, U, F, is_sym );
    u = *U;

    //Log() << "u = " << u.container() << "\n";
    timers["solver"].second = timers["solver"].first.elapsed();
    Log() << "[timer] solve(): " << timers["solver"].second << "\n";
} // Elaxi::solve

template<int Order, template<uint16_type,uint16_type,uint16_type> class Entity>
void
Elaxi<Order, Entity>::exportResults( double time, element_type& U )
{
    timers["export"].first.restart();

    typename timeset_type::step_ptrtype timeStep = timeSet->step( time );
    timeStep->setMesh( U.functionSpace()->mesh() );
    timeStep->add( "u0", U.template element<0>());
    timeStep->add( "u1", U.template element<1>());
    exporter->save();

    timers["export"].second = timers["export"].first.elapsed();
    Log() << "[timer] exportResults(): " << timers["export"].second << "\n";
} // Elaxi::export


} // Life




int
main( int argc, char** argv )
{
    using namespace Life;

    /* change parameters below */
    const int nOrder = 2;

    typedef Life::Elaxi<nOrder, Simplex> elaxi_type;

    /* assertions handling */
    Life::Assert::setLog( "elaxi.assert");

    /* define and run application */
    elaxi_type elaxi( argc, argv, makeAbout(), makeOptions() );
    elaxi.run();
}



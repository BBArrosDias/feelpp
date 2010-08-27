

#define BOOST_TEST_MODULE auto_order_integration tests
#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;


#include <feel/options.hpp>
#include <feel/feeldiscr/functionspace.hpp>
#include <feel/feelpoly/im.hpp>
#include <feel/feelfilters/gmsh.hpp>
#include <feel/feelfilters/gmshtensorizeddomain.hpp>
#include <feel/feelvf/vf.hpp>

using namespace Feel;
using namespace Feel::vf;

//_____________________________________________________________________________________________________//
//_____________________________________________________________________________________________________//


inline
po::options_description
makeOptions()
{
    po::options_description code1doptions("Test_AOI options");
    code1doptions.add_options()
        ("hsize", po::value<double>()->default_value( 0.1 ), "mesh size")
        ;
    return code1doptions.add( Feel::feel_options() );
}

//_____________________________________________________________________________________________________//
//_____________________________________________________________________________________________________//

inline
AboutData
makeAbout()
{
    AboutData about( "Test_AOI" ,
                     "Test_AOI" ,
                     "0.1",
                     "Test : automatic order integration (AOI)",
                     Feel::AboutData::License_GPL,
                     "Copyright (c) 2010 Universite Joseph Fourier");

    about.addAuthor("Vincent Chabannes", "developer", "vincent.chabannes@imag.fr", "");
    return about;

}

//_____________________________________________________________________________________________________//
//_____________________________________________________________________________________________________//


class Test_AOI
    :
    public Application
{
    typedef Application super;

public:

    typedef double value_type;

    //-----------------------------------------------------------------------------------//

    typedef Simplex<2,1,2> convex_type;
    typedef Mesh<convex_type> mesh_type;
    typedef boost::shared_ptr<mesh_type> mesh_ptrtype;

    //-----------------------------------------------------------------------------------//

    // scalar basis
    typedef bases<Lagrange<5,Scalar> > basis_scalar_type;
    typedef FunctionSpace<mesh_type, basis_scalar_type> space_scalar_type;
    typedef boost::shared_ptr<space_scalar_type> space_scalar_ptrtype;
    typedef  space_scalar_type::element_type element_scalar_type;

    //-----------------------------------------------------------------------------------//

    // mixed FunctionSpace
    typedef Lagrange<2, Vectorial> basis_u_type;
    typedef Lagrange<4, Scalar> basis_p_type;
    typedef Lagrange<0, Scalar> basis_l_type;
    typedef bases<basis_u_type,basis_p_type, basis_l_type> basis_mixed_type;
    typedef FunctionSpace<mesh_type, basis_mixed_type> space_mixed_type;
    BOOST_MPL_ASSERT( ( boost::is_same< space_mixed_type::bases_list, basis_mixed_type> ) );
    BOOST_MPL_ASSERT( ( boost::is_same< mpl::at< space_mixed_type::bases_list,mpl::int_<0> >::type, basis_u_type> ) );
    BOOST_MPL_ASSERT( ( boost::is_same< mpl::at< space_mixed_type::bases_list,mpl::int_<1> >::type, basis_p_type> ) );
    BOOST_MPL_ASSERT( ( boost::is_same< mpl::at< space_mixed_type::bases_list,mpl::int_<2> >::type, basis_l_type> ) );
    typedef boost::shared_ptr<space_mixed_type> space_mixed_ptrtype;
    // functions
    typedef space_mixed_type::element_type element_mixed_type;
    typedef element_mixed_type::sub_element<0>::type element_mixed_0_type;
    typedef element_mixed_type::sub_element<1>::type element_mixed_1_type;
    typedef element_mixed_type::sub_element<2>::type element_mixed_2_type;

    //-----------------------------------------------------------------------------------//

    Test_AOI( int argc, char** argv, AboutData const& ad,
                  po::options_description const& od )
        :
        super( argc, argv, ad, od ),
        meshSize( this->vm()["hsize"].as<double>() ) { }

    mesh_ptrtype createMesh( double meshSize );

    void run();

    //-----------------------------------------------------------------------------------//

private:

    // mesh characteristic size
    double meshSize;

};

//_____________________________________________________________________________________________________//
//_____________________________________________________________________________________________________//

Test_AOI::mesh_ptrtype
Test_AOI::createMesh( double meshSize ) {

    mesh_ptrtype mesh( new mesh_type );

    GmshTensorizedDomain< convex_type::nDim,convex_type::nOrder,convex_type::nRealDim, Simplex> td;
    td.setCharacteristicLength( meshSize );
    td.setX( std::make_pair( 0, 1 ) );
    td.setY( std::make_pair( 0, 1 ) );

    std::string fname = td.generate( convex_type::name().c_str() );

    ImporterGmsh<mesh_type> import( fname );
    mesh->accept( import );

    return mesh;

}

//_____________________________________________________________________________________________________//
//_____________________________________________________________________________________________________//

void
Test_AOI::run()
{

    if ( this->vm().count( "help" ) )
        {
            std::cout << this->optionsDescription() << "\n";
            return;
        }

    using namespace Feel::vf;

    //-----------------------------------------------------------------------------------//

    this->changeRepository( boost::format( "%1%/%2%/h_%3%/" )
                            % this->about().appName()
                            % convex_type::name()
                            % this->vm()["hsize"].as<double>()
                            );

    //-----------------------------------------------------------------------------------//

    mesh_ptrtype mesh = createMesh( meshSize );

    //-----------------------------------------------------------------------------------//

    space_scalar_ptrtype Xh = space_scalar_type::New( mesh );
    element_scalar_type us( Xh, "us" );

    space_mixed_ptrtype Xh_Mixed = space_mixed_type::New( mesh );
    element_mixed_type U_mixed(Xh_Mixed,"U_mixed");
    element_mixed_0_type u_mixed = U_mixed.element<0>();
    element_mixed_1_type p_mixed = U_mixed.element<1>();

    //-----------------------------------------------------------------------------------//

    AUTO (f , sin(0.5*M_PI*Px())*(cos(0.5*M_PI*Py())));
    AUTO (g , Px()*(Px()-1)*0.5*Py() );

    us = vf::project( Xh, elements(mesh), g );
    p_mixed = vf::project( p_mixed.functionSpace() , elements(mesh), f);

    //-----------------------------------------------------------------------------------//

    BOOST_CHECK( cst(1.0).imorder == 0 );
    BOOST_CHECK( cos(cst(M_PI)).imorder == 0 );

    //-----------------------------------------------------------------------------------//

    const uint16_type us_order = element_scalar_type::functionspace_type::basis_type::nOrder;
    BOOST_CHECK( id(us).imorder == us_order );
    BOOST_CHECK( grad(us).imorder == us_order-1 );
    BOOST_CHECK( hess(us).imorder == us_order-2 );
    BOOST_CHECK( (id(us)+id(us)).imorder == us_order );
    BOOST_CHECK( (id(us)*id(us)).imorder == 2*us_order );

    //-----------------------------------------------------------------------------------//

    const uint16_type u_mixed_order = element_mixed_0_type::functionspace_type::basis_type::nOrder;
    BOOST_CHECK( idt(u_mixed).imorder == u_mixed_order );
    BOOST_CHECK( gradt(u_mixed).imorder == u_mixed_order-1 );
    BOOST_CHECK( hesst(u_mixed).imorder == u_mixed_order-2 );
    BOOST_CHECK( (idt(u_mixed)+idt(u_mixed)).imorder == u_mixed_order );
    BOOST_CHECK( (idt(u_mixed)*idt(u_mixed)).imorder == 2*u_mixed_order );

    const uint16_type p_mixed_order = element_mixed_1_type::functionspace_type::basis_type::nOrder;
    BOOST_CHECK( idt(p_mixed).imorder == p_mixed_order );
    BOOST_CHECK( gradt(p_mixed).imorder == p_mixed_order-1 );
    BOOST_CHECK( hesst(p_mixed).imorder == p_mixed_order-2 );
    BOOST_CHECK( (idt(p_mixed)+idt(p_mixed)).imorder == p_mixed_order );
    BOOST_CHECK( (idt(p_mixed)*idt(p_mixed)).imorder == 2*p_mixed_order );

    //-----------------------------------------------------------------------------------//

    BOOST_CHECK( (idt(us)+idt(p_mixed)).imorder == std::max(us_order,p_mixed_order) );
    BOOST_CHECK( (idt(us)*idt(p_mixed)).imorder == us_order+p_mixed_order );

    //-----------------------------------------------------------------------------------//

    BOOST_CHECK( vec(idv(us),idv(p_mixed)).imorder == std::max(us_order,p_mixed_order) );

    BOOST_CHECK( (mat<2,2>( idv(us),cst(1),idv(us),idv(p_mixed))).imorder == std::max(us_order,p_mixed_order) );

    //-----------------------------------------------------------------------------------//

    BOOST_CHECK( exp(Px()).imorder == 2 );
    BOOST_CHECK( chi(Px()>0.5).imorder == 0 );

    //-----------------------------------------------------------------------------------//

    double int11=integrate(elements(mesh),_Q<4>(), f ).evaluate()(0,0);
    double int12=integrate(elements(mesh), f ).evaluate()(0,0);
    BOOST_CHECK_EQUAL(int11,int12);
    //BOOST_CHECK(std::abs(int11-int12)<1e-15);

    double int21=integrate(elements(mesh),_Q<3>(), g ).evaluate()(0,0);
    double int22=integrate(elements(mesh), g ).evaluate()(0,0);
    BOOST_CHECK_EQUAL(int21,int22);

    double int31=integrate(elements(mesh),_Q<us_order+p_mixed_order-1 >(), gradv(p_mixed)*idv(u_mixed) ).evaluate()(0,0);
    double int32=integrate(elements(mesh), gradv(p_mixed)*idv(u_mixed)  ).evaluate()(0,0);
    BOOST_CHECK_EQUAL(int31,int32);


}


//________________________________________________________________________________//
//________________________________________________________________________________//
//________________________________________________________________________________//


BOOST_AUTO_TEST_SUITE( auto_order_integration_testsuite )

BOOST_AUTO_TEST_CASE( auto_order_integration )
{

    Test_AOI leTest( boost::unit_test::framework::master_test_suite().argc,
                     boost::unit_test::framework::master_test_suite().argv,
                     makeAbout(), makeOptions() );

    leTest.run();
}
BOOST_AUTO_TEST_SUITE_END()

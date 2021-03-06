#include "../feel/feelmodels/hdg/mixedpoisson.hpp"

using namespace Feel;

inline
AboutData
makeAbout()
{
    AboutData about( "mixed-poisson-model" ,
                     "mixed-poisson-model" ,
                     "0.1",
                     "Mixed-Poisson-Model",
                     AboutData::License_GPL,
                     "Copyright (c) 2016 Feel++ Consortium" );
    about.addAuthor( "Christophe Prud'homme", "developer", "christophe.prudhomme@feelpp.org", "" );
    about.addAuthor( "Romain Hild", "developer", "", "" );
    about.addAuthor( "Daniele Prada", "developer", "", "" );
    return about;
}

void printCvg( std::ostream& out, std::vector<double> h, std::vector<std::map<std::string,std::map<std::string,double> > > errors )
{
    boost::format fmter("%1% %|14t|");
    boost::format fmter_endl("%1%\n");
    std::vector<std::string> fields = {"P","PP","U"};
    std::vector<std::string> norms = {"L2","L2_rel"};//,"H1","H1_rel"};

    out << fmter % "h";
    for(auto const& f : fields )
        for( auto const& n : norms )
            out << fmter % ("err"+f+"_"+n);
    out << "\n";

    int maxiter = h.size();
    for( int i = 0; i < maxiter; ++i )
    {
        out << fmter % h[i];
        for(auto const& f : fields )
            for( auto const& n : norms )
                out << fmter % errors[i][f][n];
        out << "\n";
    }
}

template<typename ExpF>
void
computeErrors( ExpF const& F, Expr<GinacExVF<15>> const& T, std::map<std::string, double> & m)
{
    int quadError = ioption("benchmark.quad");
    m["L2"] = normL2( _range=F.functionSpace()->template rangeElements<0>(),
                      _expr=idv(F)-T,
                      _quad=quadError, _quad1=quadError );
    // m["H1"]= normH1( _range=F.functionSpace()->template rangeElements<0>(),
    //                  _expr=idv(F)-T,
    //                  _grad_expr=gradv(F)-grad<3>(T),
    //                  _quad=quadError, _quad1=quadError );
}

template<typename ExpF, int M>
void
computeErrors( ExpF const& F, Expr<GinacMatrix<M,1,15>> const& T, std::map<std::string, double> & m)
{
    int quadError = ioption("benchmark.quad");
    m["L2"] = normL2( _range=F.functionSpace()->template rangeElements<0>(),
                      _expr=idv(F)-T,
                      _quad=quadError, _quad1=quadError );
    // m["H1"] = normH1( _range=F.functionSpace()->template rangeElements<0>(),
    //                   _expr=idv(F)-T,
    //                   _grad_expr=gradv(F)-grad(T),
    //                   _quad=quadError, _quad1=quadError );
}

template<int Dim, int OrderT, int GOrder = 1>
int
runApplicationMixedPoisson( std::string  const& prefix )
{
    using namespace Feel;

    typedef FeelModels::MixedPoisson<Dim,OrderT,GOrder> mp_type;
    using mesh_type = typename mp_type::mesh_type;
    using exporter_type = Exporter<mesh_type>;
    using exporter_ptrtype = std::shared_ptr<exporter_type>;

    std::string p = "hdg.poisson";
    auto MP = mp_type::New(p);
    auto model = MP->modelProperties().models().model();
    auto paramValues = MP->modelProperties().parameters().toParameterValues();

    auto pString = model.ptree().template get<std::string>("p_expr");
    auto pExpr = expr<15>(pString);
    pExpr.setParameterValues(paramValues);
    auto uString = model.ptree().get("u_expr","");
    auto uExpr = expr<Dim,1,15>(uString);
    uExpr.setParameterValues(paramValues);

    int maxiter = ioption("benchmark.nlevels");
    double factor = doption("benchmark.refine");
    double size = doption("benchmark.hsize");

    exporter_ptrtype e( exporter_type::New("convergence") );

    std::vector<double> h(maxiter);
    std::vector<std::map<std::string,std::map<std::string,double> > > errors(maxiter, std::map<std::string,std::map<std::string,double> >());
    std::map<std::string, double> mapP, mapU, mapPP;

    for( int i = 0; i < maxiter; ++i )
    {
        h[i] = size;
        auto mesh = loadMesh( _mesh=new mesh_type, _h=size );
        if( e->doExport() )
            e->step(i)->setMesh(mesh);
        MP->init(mesh);
        MP->assembleAll();
        MP->solve();
        MP->postProcess();

        auto P_h = MP->potentialField();
        auto PP_h = MP->postPotentialField();
        auto Ph = MP->potentialSpace();
        auto P_ex = Ph->element(pExpr);
        computeErrors(P_h, pExpr, mapP);
        computeErrors(PP_h, pExpr, mapPP);
        errors[i]["P"] = mapP;
        errors[i]["PP"] = mapPP;
        if( e->doExport() )
        {
            e->step(i)->add("P_h", P_h);
            e->step(i)->add("PP_h", PP_h);
            e->step(i)->add("P_ex", P_ex);
        }

        auto U_h = MP->fluxField();
        auto Uh = MP->fluxSpace();
        auto U_ex = Uh->element(uExpr);
        computeErrors(U_h, uExpr, mapU);
        errors[i]["U"] = mapU;
        if( e->doExport() )
        {
            e->step(i)->add("U_h", U_h);
            e->step(i)->add("U_ex", U_ex);
        }

        if( e->doExport() )
            e->save();

        size /= factor;

        if( i == maxiter-1 )
        {
            int quadError = ioption("benchmark.quad");
            double nPL2 = normL2( _range=Ph->template rangeElements<0>(),
                                  _expr=pExpr,
                                  _quad=quadError, _quad1=quadError );
            // double nPH1 = normH1( _range=Ph->template rangeElements<0>(),
            //                            _expr=pExpr, _grad_expr=grad<Dim>(pExpr),
            //                            _quad=quadError, _quad1=quadError );
            double nUL2 = normL2( _range=Uh->template rangeElements<0>(),
                                  _expr=uExpr,
                                  _quad=quadError, _quad1=quadError );
            // double nUH1 = normH1( _range=Uh->template rangeElements<0>(),
            //                            _expr=uExpr, _grad_expr=grad(uExpr),
            //                            _quad=quadError, _quad1=quadError );
            for( int j = 0; j < maxiter; ++j)
            {
                errors[j]["P"]["L2_rel"] = errors[j]["P"]["L2"]/nPL2;
                errors[j]["PP"]["L2_rel"] = errors[j]["PP"]["L2"]/nPL2;
                errors[j]["U"]["L2_rel"] = errors[j]["U"]["L2"]/nUL2;
                // errors[j]["P"]["H1_rel"] = errors[j]["P"]["H1"]/nPH1;
                // errors[j]["PP"]["H1_rel"] = errors[j]["PP"]["H1"]/nPH1;
                // errors[j]["U"]["H1_rel"] = errors[j]["U"]["H1"]/nUH1;
            }
        }
    }

    Feel::cout << std::endl;
    if( Environment::isMasterRank() )
    {
        printCvg( std::cout, h, errors );
        std::ofstream file ( soption("benchmark.filename") );
        if( file )
        {
            printCvg( file, h, errors );
            file.close();
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    using namespace Feel;

    po::options_description mpoptions( "hdg.poisson options" );
    mpoptions.add( FeelModels::makeMixedPoissonOptions("","hdg.poisson") );
    mpoptions.add_options()
        ("benchmark.filename", Feel::po::value<std::string>()->default_value( "cvg.csv" ), "file to save the results" )
        ("benchmark.quad", Feel::po::value<int>()->default_value( 5 ), "quadrature order for the error")
        ("case.dimension", Feel::po::value<int>()->default_value( 3 ), "dimension")
        ("case.discretization", Feel::po::value<std::string>()->default_value( "P1" ), "discretization : P1,P2,P3 ")
        ;

    Feel::Environment env( _argc=argc,
                           _argv=argv,
                           _about=makeAbout(),
                           _desc=mpoptions,
                           _desc_lib=FeelModels::makeMixedPoissonLibOptions("","hdg.poisson").add(feel_options())
                           );

    int dimension = ioption(_name="case.dimension");
    std::string discretization = soption(_name="case.discretization");

    auto dimt = hana::make_tuple(hana::int_c<2>,hana::int_c<3>);
#if FEELPP_INSTANTIATION_ORDER_MAX >= 3
    auto discretizationt = hana::make_tuple( hana::make_tuple("P1", hana::int_c<1>, hana::int_c<1> ),
                                             hana::make_tuple("P2", hana::int_c<2>, hana::int_c<1> ),
                                             hana::make_tuple("P3", hana::int_c<3>, hana::int_c<1> ),
                                             hana::make_tuple("P1G2", hana::int_c<1>, hana::int_c<2> ),
                                             hana::make_tuple("P2G2", hana::int_c<2>, hana::int_c<2> ),
                                             hana::make_tuple("P3G2", hana::int_c<3>, hana::int_c<2> ) );
#elif FEELPP_INSTANTIATION_ORDER_MAX >= 2
    auto discretizationt = hana::make_tuple( hana::make_tuple("P1", hana::int_c<1>, hana::int_c<1> ),
                                             hana::make_tuple("P2", hana::int_c<2>, hana::int_c<1> ),
                                             hana::make_tuple("P1G2", hana::int_c<1>, hana::int_c<2> ),
                                             hana::make_tuple("P2G2", hana::int_c<2>, hana::int_c<2> ) );
#else
    auto discretizationt = hana::make_tuple( hana::make_tuple("P1", hana::int_c<1>, hana::int_c<1> ),
                                             hana::make_tuple("P1G2", hana::int_c<1>, hana::int_c<2> ) );
#endif

    hana::for_each( hana::cartesian_product(hana::make_tuple(dimt,discretizationt)),
                    [&discretization,&dimension]( auto const& d )
                        {
                            constexpr int _dim = std::decay_t<decltype(hana::at_c<0>(d))>::value;
                            std::string const& _discretization = hana::at_c<0>( hana::at_c<1>(d) );
                            constexpr int _torder = std::decay_t<decltype(hana::at_c<1>( hana::at_c<1>(d) ))>::value;
                            constexpr int _gorder = std::decay_t<decltype(hana::at_c<2>( hana::at_c<1>(d) ))>::value;
                            if ( dimension == _dim && discretization == _discretization )
                                runApplicationMixedPoisson<_dim,_torder,_gorder>( "" );
                        } );

    return 0;
}

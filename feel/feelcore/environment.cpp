/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=tcl:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2010-04-14

  Copyright (C) 2010,2011 Université Joseph Fourier (Grenoble I)

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
   \file environment.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2010-04-14
 */
#include <feel/feelconfig.h>

#include <boost/program_options.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <feel/feelcore/environment.hpp>

#if defined( HAVE_PETSC_H )
extern "C"
{
#include <petsc.h>
#include <petscerror.h>
}
#if defined( HAVE_SLEPC )
# include <slepc/slepc.h>
#endif /* HAVE_SLEPC */

#endif /* HAVE_PETSC_H */
#if defined( HAVE_MADLIB_H )

#endif
namespace Feel
{
Environment::Environment()
    :
    M_env()
{
#if defined( HAVE_TBB )
    int n = tbb::task_scheduler_init::default_num_threads();
#else
    int n = 1 ;
#endif
    Log() << "[Feel++] TBB running with " << n << " threads\n";
    //tbb::task_scheduler_init init(1);

    mpi::communicator world;
#if defined ( HAVE_PETSC_H )
    PetscTruth is_petsc_initialized;
    PetscInitialized( &is_petsc_initialized );
    if ( !is_petsc_initialized )
    {
        i_initialized = true;
        int ierr = PetscInitializeNoArguments();

        boost::ignore_unused_variable_warning(ierr);
        CHKERRABORT(world,ierr);
    }
#endif // HAVE_PETSC_H
}
Environment::Environment( int& argc, char**& argv )
    :
    M_env( argc, argv )
{
#if defined( HAVE_TBB )
    //int n = tbb::task_scheduler_init::default_num_threads();
    int n = 2;
#else
    int n = 1 ;
#endif
    //Log() << "[Feel++] TBB running with " << n << " threads\n";
    //tbb::task_scheduler_init init(2);

    mpi::communicator world;
#if defined ( HAVE_PETSC_H )
    PetscTruth is_petsc_initialized;
    PetscInitialized( &is_petsc_initialized );
    if ( !is_petsc_initialized )
    {
        i_initialized = true;
#if defined( HAVE_SLEPC )
        int ierr = SlepcInitialize(&argc,&argv, PETSC_NULL, PETSC_NULL );
#else
        int ierr = PetscInitialize( &argc, &argv, PETSC_NULL, PETSC_NULL );
#endif
        boost::ignore_unused_variable_warning(ierr);
        CHKERRABORT(world,ierr);
    }
#endif // HAVE_PETSC_H

#if defined( HAVE_MADLIB_H )
    // \warning Madlib initializes MPI too (may generate warnings)
    //MAdLibInitialize( &argc, &argv );
#endif // HAVE_MADLIB_H

    if ( argc >= 1 )
    {
        std::ostringstream ostr;
        ostr << argv[0] << ".assertions";
        Assert::setLog( ostr.str().c_str() );
    }
}

Environment::~Environment()
{
    if ( i_initialized )
    {
#if defined ( HAVE_PETSC_H )
        PetscTruth is_petsc_initialized;
        PetscInitialized( &is_petsc_initialized );
        if ( is_petsc_initialized )
        {
#if defined( HAVE_SLEPC )
            SlepcFinalize();
#else
            PetscFinalize();
#endif // HAVE_SLEPC
        }
#endif // HAVE_PETSC_H
    }

#if defined( HAVE_MADLIB_H )
    //MAdLibFinalize();
#endif // HAVE_MADLIB_H
}



bool
Environment::initialized()
{

#if defined( HAVE_PETSC_H )
    PetscTruth is_petsc_initialized;
    PetscInitialized( &is_petsc_initialized );
    return mpi::environment::initialized() && is_petsc_initialized ;
#else
    return mpi::environment::initialized() ;
#endif
}

bool
Environment::finalized()
{
#if defined( HAVE_PETSC_H )
    PetscTruth is_petsc_initialized;
    PetscInitialized( &is_petsc_initialized );
    return mpi::environment::finalized() && !is_petsc_initialized;
#else
    return mpi::environment::finalized();
#endif
}


std::string
Environment::rootRepository()
{
    std::string env;
    if ( ::getenv( "FEEL_REPOSITORY" ) )
    {
        env = ::getenv( "FEEL_REPOSITORY" );
    }
    else
    {
        // by default create $HOME/feel
        env = ::getenv( "HOME" );
        env += "/feel";
    }
    return env;
}
std::string
Environment::localGeoRepository()
{
    fs::path rep_path;

    rep_path = Environment::rootRepository();
    rep_path /= "geo";
    if ( !fs::exists( rep_path ) )
        fs::create_directory( rep_path );
    return rep_path.string();
}
boost::tuple<std::string,bool>
Environment::systemGeoRepository()
{
    fs::path rep_path;

    rep_path = BOOST_PP_STRINGIZE(INSTALL_PREFIX);
    rep_path /= "share/feel/geo";
    return boost::make_tuple( rep_path.string(), fs::exists( rep_path ) );
}

std::string
Environment::localConfigRepository()
{
    fs::path rep_path;

    rep_path = Environment::rootRepository();
    rep_path /= "config";
    if ( !fs::exists( rep_path ) )
        fs::create_directory( rep_path );
    return rep_path.string();
}
boost::tuple<std::string,bool>
Environment::systemConfigRepository()
{
    fs::path rep_path;

    rep_path = BOOST_PP_STRINGIZE(INSTALL_PREFIX);
    rep_path /= "share/feel/config";
    return boost::make_tuple( rep_path.string(), fs::exists( rep_path ) );
}

void
Environment::changeRepository( boost::format fmt )
{
    fs::path rep_path;

    rep_path = Environment::rootRepository();
    if ( !fs::exists( rep_path ) )
        fs::create_directory( rep_path );

    typedef std::vector< std::string > split_vector_type;

    split_vector_type dirs; // #2: Search for tokens
    std::string fmtstr = fmt.str();
    boost::split( dirs, fmtstr, boost::is_any_of("/") );

    BOOST_FOREACH( std::string const& dir, dirs )
        {
            //Debug( 1000 ) << "[Application::Application] option: " << s << "\n";
            rep_path = rep_path / dir;
            if (!fs::exists( rep_path ) )
                fs::create_directory( rep_path );
        }

    ::chdir( rep_path.string().c_str() );
}

po::variables_map
Environment::vm( po::options_description const& desc )
{
    po::variables_map vm;
	po::store(po::parse_command_line(0, (char**)0, desc), vm);
	po::notify(vm);

    return vm;
}
}

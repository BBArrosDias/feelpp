/* -*- mode: c++ -*-

  This file is part of the Life library

  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
       Date: 2006-04-25

  Copyright (C) 2006 EPFL

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
   \file application.cpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2006-04-25
 */
#include <boost/concept_check.hpp>

#include <life/lifecore/life.hpp>
#include <life/lifecore/application.hpp>

namespace Life
{
#if defined( HAVE_TRILINOS_EPETRA )

po::options_description
makeMLEpetraOptions()
{
    po::options_description mlEpetraOptions( "ML Epetra options" );
    mlEpetraOptions.add_options()
        ("max-levels", po::value<int>()->default_value( 6 ), "max levels number")
        ("aggregation-type", po::value<std::string>()->default_value( "MIS" ), "aggregation type")
        ("coarse-type", po::value<std::string>()->default_value( "Amesos_KLU" ), "coarse type")
        ("increasing-or-decreasing", po::value<std::string>()->default_value( "decreasing" ), "increasing or decreasing level indices");
    return mlEpetraOptions;
}
LIFE_NO_EXPORT
po::options_description
epetraOptions()
{
    po::options_description epetra("EPETRA options");
    epetra.add_options()
        ("disable-epetra", "disable epetra")
        ;
    return epetra;
}

void
Application::init( MPI_Comm& _comm )
{
    if ( _S_is_Initialized == false )
        {
            _S_comm = boost::shared_ptr<comm_type>( new Epetra_MpiComm(_comm) );

            _S_is_Initialized = true;
        }
}


#if defined(HAVE_MPI)
Application::Application( int argc,
                                      char** argv,
                                      AboutData const& ad,
                                      MPI_Comm comm )
    :
    super( argc, argv, ad, epetraOptions().add( makeMLEpetraOptions() ), comm )
{
    init(comm);
}


#else
Application::Application( int argc,
                                      char** argv,
                                      AboutData const& ad )
    :
    super( argc, argv, ad, epetraOptions().add( makeMLEpetraOptions() ) )
{
    init(comm);
}
#endif /* HAVE_MPI */


#if defined(HAVE_MPI)
Application::Application( int argc,
                                      char** argv,
                                      AboutData const& ad,
                                      po::options_description const& od,
                                      MPI_Comm comm )
    :
    super( argc, argv, ad, epetraOptions().add( makeMLEpetraOptions() ).add(od), comm )
{
//cout << "hallo HAVE_MPI2\n" << endl;
    init(comm);
}

#else
Application::Application( int argc,
                                      char** argv,
                                      AboutData const& ad,
                                      po::options_description const& od )
    :
    super( argc, argv, ad, epetraOptions().add( makeMLEpetraOptions() ).add(od) )
{
    init(comm);
}



#endif /* HAVE_MPI */


Application::~Application()
{
}

boost::shared_ptr<Epetra_MpiComm> Application::_S_comm;

bool Application::_S_is_Initialized = false;

#endif // HAVE_TRILINOS_EPETRA



}


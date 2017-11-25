//! -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
//!
//! This file is part of the Feel++ library
//!
//! This library is free software; you can redistribute it and/or
//! modify it under the terms of the GNU Lesser General Public
//! License as published by the Free Software Foundation; either
//! version 2.1 of the License, or (at your option) any later version.
//!
//! This library is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//! Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public
//! License along with this library; if not, write to the Free Software
//! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//!
//! @file
//! @author Christophe Prud'homme <christophe.prudhomme@feelpp.org>
//! @date 25 Nov 2017
//! @copyright 2017 Feel++ Consortium
//!

namespace Feel
{
namespace FeelModels
{
po::options_description makeMixedElasticityLibOptions( std::string prefix = "mixedelasticity" )
{
    po::options_description mpLibOptions( "Mixed Elasticity HDG Lib options");
    // if ( !prefix.empty() )
    //     mpLibOptions.add( backend_options( prefix ) );
    return mpLibOptions;
}

po::options_description
makeMixedElasticityOptions( std::string prefix = "mixedelasticity" )
{
    po::options_description mpOptions( "Mixed Elasticity HDG options");
    mpOptions.add_options()
        ( prefixvm( prefix, "gmsh.submesh").c_str(), po::value<std::string>()->default_value( "" ), "submesh extraction" )
        // ( "gmsh.submesh2", po::value<std::string>()->default_value( "" ), "submesh extraction" )
        ( prefixvm( prefix, "hface").c_str(), po::value<int>()->default_value( 0 ), "hface" )
        ( "lambda", po::value<std::string>()->default_value( "1" ), "lambda" )
        ( "mu", po::value<std::string>()->default_value( "1" ), "mu" )
        ( prefixvm( prefix, "model_json").c_str(), po::value<std::string>()->default_value("model.json"), "json file for the model")
        ( prefixvm( prefix,"tau_constant").c_str(), po::value<double>()->default_value( 1.0 ), "stabilization constant for hybrid methods" )
        ( prefixvm( prefix,"tau_order").c_str(), po::value<int>()->default_value( 0 ), "order of the stabilization function on the selected edges"  ) // -1, 0, 1 ==> h^-1, h^0, h^1
        ( prefixvm( prefix, "use-sc").c_str(), po::value<bool>()->default_value(true), "use static condensation")           
        ;
    mpOptions.add ( envfeelmodels_options( prefix ) ).add( modelnumerical_options( prefix ) );
	mpOptions.add ( feel_options() ).add( backend_options( prefixvm(prefix, "sc") ) );
    return mpOptions;
}

} // FeelModels
} // Feel

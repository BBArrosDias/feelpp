/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*-
 
 This file is part of the Feel++ library
 
 Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
 Date: 11 Apr 2015
 
 Copyright (C) 2015 Feel++ Consortium
 
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
#include <iostream>
#include <feel/feelcore/feel.hpp>
#include <feel/feelcore/environment.hpp>

#include <feel/feelmodels/modelpostprocess.hpp>

namespace Feel {

template <typename T>
std::vector<T> as_vector(pt::ptree const& pt, pt::ptree::key_type const& key)
{
    std::vector<T> r;
    for (auto& item : pt.get_child(key))
        r.push_back(item.second.template get_value<T>());
    return r;
}

void
ModelPostprocessPointPosition::setup( std::string const& name )
{

    // fields is necessary
    if ( !M_p.get_child_optional("fields") )
        return;

    bool hasCoord = (M_p.get_child_optional("coord"))?true:false;
    bool hasMarker = (M_p.get_child_optional("marker"))?true:false;
    // coord or marker is necessary
    if ( !hasCoord && !hasMarker )
        return;


    this->pointPosition().setName( name );
    if ( hasMarker )
    {
        std::string marker = M_p.get<std::string>( "marker" );
        this->pointPosition().setMeshMarker( marker );
    }
    if ( hasCoord )
    {
        std::string coordExpr = M_p.get<std::string>( "coord" );
        //std::cout << "coordExpr : "<< coordExpr << "\n";

        auto parseExpr = GiNaC::parse(coordExpr);
        auto const& coordMatExpr = parseExpr.first.evalm();
        auto const& coordExprSymbol = parseExpr.second;
        int nComp = coordMatExpr.nops();
        ModelPointPosition::coord_value_type coordData = ModelPointPosition::coord_value_type::Zero(3,1);
        //for ( auto symbb : coordExprSymbol )
        //    std::cout << "symbb " << symbb << "\n";
        if ( coordExprSymbol.empty() || ( coordExprSymbol.size() == 1 && coordExprSymbol[0].get_name() == "0" ) )
        {
            int nComp = GiNaC::parse(coordExpr).first.evalm().nops();
            //std::cout << "ncomp " << nComp << "\n";
            for (int comp=0;comp<nComp;++comp )
            {
                std::string compExpr = str( coordMatExpr.op(comp) );
                try
                {
                    coordData(comp) = std::stod( compExpr );
                }
                catch (std::invalid_argument& err)
                {
                    LOG(WARNING) << "cast fail from expr to double\n";
                    coordData(comp) = 0;
                }
            }
            LOG(INFO) << "point coord is a cst expr : " << coordData(0) << "," << coordData(1) << "," << coordData(2);
            this->pointPosition().setValue( coordData );
        }
        else
        {
            LOG(INFO) << "point coord is a symbolic expr : " << coordExpr;
            this->pointPosition().setExpression( coordExpr, M_directoryLibExpr, M_worldComm );
        }
    } // hasCoord

    // store fields name
    std::vector<std::string> fieldList = as_vector<std::string>( M_p, "fields" );
    if ( fieldList.empty() )
    {
        std::string fieldUnique = M_p.get<std::string>( "fields" );
        if ( !fieldUnique.empty() )
            fieldList = { fieldUnique };
    }
    for( std::string const& field : fieldList )
    {
        // std::cout << "add field = " << field << "\n";
        this->addFields( field );
    }

}


ModelPostprocess::ModelPostprocess( WorldComm const& world )
    :
    M_worldComm( world )
{}

ModelPostprocess::ModelPostprocess(pt::ptree const& p, WorldComm const& world )
    :
    M_worldComm( world )
{}

ModelPostprocess::~ModelPostprocess()
{}

void
ModelPostprocess::setPTree( pt::ptree const& p )
{
    M_p = p;
    setup();
}

void
ModelPostprocess::setup()
{
    auto fields = M_p.get_child_optional("Fields");
    if ( fields )
    {
        
        for (auto i : as_vector<std::string>(M_p, "Fields"))
        {
            this->operator[]("Fields").push_back( i );
            LOG(INFO) << "add to postprocess field  " << i;
        }
        
    }
    auto forces = M_p.get_child_optional("Force");
    if ( forces )
    {
        
        for (auto i : as_vector<std::string>(M_p, "Force"))
        {
            this->operator[]("Force").push_back( i );
            LOG(INFO) << "add to postprocess force  " << i;
            
        }
        
    }
    auto stresses = M_p.get_child_optional("Stresses");
    if ( stresses )
    {
        
        for (auto i : as_vector<std::string>(M_p, "Stresses"))
        {
            this->operator[]("Stresses").push_back( i );
            LOG(INFO) << "add to postprocess stresses  " << i;
            
        }
        
    }

    auto eval = M_p.get_child_optional("Eval");
    if ( eval )
    {
        auto evalPoints = eval->get_child_optional("Points");
        if ( evalPoints )
        {
            for( auto const& evalPoint : *evalPoints )
            {
                ModelPostprocessPointPosition myPpPtPos( M_worldComm );
                myPpPtPos.setDirectoryLibExpr( M_directoryLibExpr );
                myPpPtPos.setPTree( evalPoint.second, evalPoint.first );
                if ( !myPpPtPos.fields().empty() )
                    M_evalPoints.push_back( myPpPtPos );
            }

            if ( !M_evalPoints.empty() )
                this->operator[]("Eval").push_back( "Points" );
        }
    }


}

void
ModelPostprocess::saveMD(std::ostream &os)
{
  os << "### PostProcess\n";
  os << "|Kind | data |\n";
  os << "|---|---|\n";
  for(auto it = this->begin(); it != this->end(); it++)
  {
    os << "|" << it->first;
    os << "|<ul>";
    for(auto iit = it->second.begin(); iit != it->second.end(); iit++)
      os << "<li>" << *iit << "</li>";
    os << "</ul>|\n";
  }
}

void
ModelPostprocess::setParameterValues( std::map<std::string,double> const& mp )
{
    for( auto & p : M_evalPoints )
        p.setParameterValues( mp );
}


}

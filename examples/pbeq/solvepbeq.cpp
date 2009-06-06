/* -*- mode: c++ -*-

This file is part of the Life library

Author(s): Simone Deparis <simone.deparis@epfl.ch>
Date: 2007-07-10

Copyright (C) 2007 Unil

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
   \file molecule.cpp
   \author Simone Deparis <simone.deparis@epfl.ch>
   \date 2007-07-10
*/

#include "pbeq.hpp"

//#include <iostream>
#include <fstream>
#include <string>

#include <life/options.hpp>


std::vector<std::string> parseList( const std::string& list )
{
    std::string stringList = list;
    std::vector<std::string> vectorList;
    if ( list == "" )
    {
        return vectorList;
    }
    std::string::size_type commaPos = 0;
    while ( commaPos != std::string::npos )
    {
        commaPos = stringList.find( " " );
        vectorList.push_back( stringList.substr( 0, commaPos ).c_str() );
        stringList = stringList.substr( commaPos+1 );
    }
    //vectorList.push_back( stringList.c_str() );
    return vectorList;
}


inline
Life::po::options_description
makeOptions()
{
    Life::po::options_description moleculeoptions("Molecule options");
    moleculeoptions.add_options()
        ("receptor", Life::po::value<std::string>()->default_value( "receptor" ), "basename for the PQR/CRD file that describe the receptor (without .pqr/-chr.crd extension")
        ("ligand",   Life::po::value<std::string>()->default_value( "NONE" ),   "basename for the PQR/CRD file that describe the ligand   (without .pqr/-chr.crd extension")
        ("dock",   Life::po::value<std::string>()->default_value( "NONE" ),   "basename for the pdb.dock4 file that describe the ligands   (without pdb.dock4 extension")
        ("ligands",   Life::po::value<std::string>()->default_value( "NONE" ),   "basename for the PQR/CRD files that describe the ligands   (without .pqr extension, separated by space, inside \" \")")
        ("molDir", Life::po::value<std::string>()->default_value( "./" ), "path to PQR/CRD files")
        ("crd", "files are in CRD format (default)")
        ("pqr", "files are in PQR format")
        ("geoOnly", "just create the geo file and exit")
        ("meshReuse", "force Reuse of the mesh (unless it doesn't exist), even if geo is different")
        ("hsize", Life::po::value<double>()->default_value( 0.5 ), "Mesh size h value near the molecule")
        ("farfactor", Life::po::value<double>()->default_value( 20 ), "Mesh size at far boundary domain w.r.t hsize")
        ("farBnd", Life::po::value<double>()->default_value( 5.  ), "factor to set the far boundary of domain")
        ("Kb", Life::po::value<double>()->default_value( 1.985  ), "Bolzmann constant [cal/(mol K)]")
        ("IonStr", Life::po::value<double>()->default_value( 0.1  ), "Ionic Strength")
        ("pdie", Life::po::value<double>()->default_value( 2.  ), "biomolecular dielectric constant")
        ("sdie", Life::po::value<double>()->default_value( 78.54  ), "solvent dielectric constant")
        ("temp", Life::po::value<double>()->default_value( 298.15  ), "temperature")
        ("smooth", Life::po::value<double>()->default_value( 0.3  ), "soothing window")

        ("penalbc", Life::po::value<double>()->default_value( 50 ), "penalisation parameter for the weak boundary conditions")
        ("export", "export results(ensight)")
        ("export-matlab", "export matrix and vectors in matlab" )
        ;

    return moleculeoptions.add( Life::life_options() );

}
inline
Life::AboutData
makeAbout()
{
    Life::AboutData about( "pbeq" ,
                           "pbeq" ,
                           "0.2",
                           "nD(n=3) Molecule on simplices or simplex products",
                           Life::AboutData::License_GPL,
                           "Copyright (c) 2006, 2007 Universit� Joseph Fourier and Unil");

    about.addAuthor("Simone Deparis", "developer", "simone.deparis@epfl.ch", "");
    return about;

}


int
main( int argc, char** argv )
{

    using namespace Life;

    /* assertions handling */
    Assert::setLog( "heavyside.assert");

    /* define and run application */
    Pbeq  pbeq( argc, argv, makeAbout(), makeOptions() );

    pbeq.loadFE();

    if ( pbeq.vm().count( "geoOnly" ) ) return 0;

    std::vector<value_type> outputs;
    int  ret(0);

    pbeq.setUseCRD();
    if ( pbeq.vm().count( "crd" ) ) pbeq.setUseCRD();
    if ( pbeq.vm().count( "pqr" ) ) pbeq.setUsePQR();

    enum TODO { RECEPTOR, LIGAND, DOCK };
    TODO todo(RECEPTOR);

    std::vector<std::string> receptors = parseList( pbeq.vm()["receptor"].as<std::string>() );
    std::vector<std::string> ligands;

    if ( pbeq.vm()["ligand"].as<std::string>().compare("NONE") != 0 )
        {
            todo = LIGAND;
            ligands = parseList( pbeq.vm()["ligand"].as<std::string>() );
        }

    if ( todo == LIGAND &&
         pbeq.vm()["dock"].as<std::string>().compare("NONE") != 0 )
        todo = DOCK;


    switch (todo) {

    case RECEPTOR:
        {
            Log() << "solving case RECEPTOR\n";
            outputs.reserve(receptors.size());

            for (std::vector<std::string>::const_iterator it = receptors.begin(); it != receptors.end(); ++it)
                {
                    outputs.push_back( pbeq.solveReceptor( *it ) );
                    std::cout << *(outputs.end()-1) << "\n" << std::flush;;
                }
        }
        break;

    case LIGAND:
        {
            Log() << "solving case LIGAND\n";
            if ( receptors.size() > 1)
                {
                    Log() << "warning: case LIGAND: using only first receptor \n";
                }
            pbeq.setReceptor(pbeq.vm()["receptor"].as<std::string>());

            outputs.reserve(ligands.size());

            for (std::vector<std::string>::const_iterator it = ligands.begin(); it != ligands.end(); ++it)
                {
                    pbeq.setLigand(*it);
                    outputs.push_back( pbeq.solveLigand( ) );
                    std::cout << *(outputs.end()-1) << "\n" << std::flush;;
                }
        }
        break;

    case DOCK:
        {
            Log() << "solving case DOCK\n";
            std::vector<std::string> ligands = parseList( pbeq.vm()["ligand"].as<std::string>() );
            if ( ligands.size() > 1)
                {
                    Log() << "warning: case DOCK: using only first ligand \n";
                }
            pbeq.setReceptor(pbeq.vm()["receptor"].as<std::string>());
            pbeq.setLigand  (*ligands.begin());

            Log() << "Solution with ligand in default position: "
                    << pbeq.solveLigand( ) << "\n";

            ret = pbeq.setReclusterFile(pbeq.vm()["dock"].as<std::string>());
            for (ret = pbeq.recluster(); ret == 0; ret = pbeq.recluster())
                {
                    outputs.push_back( pbeq.solveLigand( ) );
                    std::cout << *(outputs.end()-1) << "\n" << std::flush;;
                }
        }
    }
    /*
    for (std::vector<value_type>::const_iterator it = outputs.begin(); it != outputs.end(); ++it)
        std::cout << *it << "\n";
    std::cout << std::flush;
    */
    return 0;

}





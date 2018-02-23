/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4*/

#ifndef FEELPP_TOOLBOXES_THERMALPROPERTIES_DESCRIPTION_H
#define FEELPP_TOOLBOXES_THERMALPROPERTIES_DESCRIPTION_H 1

#include <feel/feelvf/cst.hpp>
#include <feel/feelmodels/modelexpression.hpp>

namespace Feel
{
namespace FeelModels
{

template<class SpaceType>
class ThermalPropertiesDescription
{
    typedef ThermalPropertiesDescription<SpaceType> self_type;
public :
    typedef SpaceType space_type;
    typedef boost::shared_ptr<SpaceType> space_ptrtype;
    typedef typename SpaceType::element_type element_type;
    typedef boost::shared_ptr<element_type> element_ptrtype;
    typedef typename space_type::mesh_type mesh_type;
    typedef boost::shared_ptr<mesh_type> mesh_ptrtype;

    ThermalPropertiesDescription( std::string const& prefix )
        :
        M_thermalConductivityDefaultValue( doption(_name="thermal-conductivity",_prefix=prefix) ),// [ W/(m*K) ]
        M_heatCapacityDefaultValue( doption(_name="heat-capacity",_prefix=prefix) ), // [ J/(kg*K) ]
        M_rhoDefaultValue( doption(_name="rho",_prefix=prefix) ),
        M_thermalExpansionDefaultValue( doption(_name="thermal-expansion",_prefix=prefix) ) // [ 1/K ]
        {}

    ThermalPropertiesDescription( ThermalPropertiesDescription const& ) = default;

    void updateForUse( mesh_ptrtype const& mesh , ModelMaterials const& mats, std::vector<WorldComm> const& worldsComm )
        {
            std::set<std::string> eltMarkersInMesh;
            for (auto const& markPair : mesh->markerNames() )
            {
                std::string meshMarker = markPair.first;
                if ( mesh->hasElementMarker( meshMarker ) )
                    eltMarkersInMesh.insert( meshMarker );
            }

            std::map<std::string,std::set<std::string>> markersByMaterial;
            M_markers.clear();
            for( auto const& m : mats )
            {
                std::string const& matName = m.first;
                auto const& mat = m.second;
                if ( mat.hasPhysics() && !mat.hasPhysics( { "heat-transfer","aerothermal","thermo-electric" } ) )
                    continue;

                for ( std::string const& matmarker : mat.meshMarkers() )
                {
                    if ( eltMarkersInMesh.find( matmarker ) == eltMarkersInMesh.end() )
                        continue;
                    M_markers.insert( matmarker );
                    markersByMaterial[matName].insert( matmarker );
                }
            }

            M_isDefinedOnWholeMesh = ( M_markers.size() == eltMarkersInMesh.size() );
            if ( M_isDefinedOnWholeMesh )
                M_space = space_type::New(_mesh=mesh, _worldscomm=worldsComm );
            else
                M_space = space_type::New(_mesh=mesh, _worldscomm=worldsComm,_range=markedelements(mesh,M_markers) );
            M_fieldThermalConductivity = M_space->elementPtr( vf::cst( this->cstThermalConductivity() ) );
            M_fieldHeatCapacity = M_space->elementPtr( vf::cst( this->cstHeatCapacity() ) );
            M_fieldRho = M_space->elementPtr( vf::cst( this->cstRho() ) );
            M_fieldThermalExpansion = M_space->elementPtr( vf::cst( this->cstThermalExpansion() ) );

            for( auto const& m : mats )
            {
                std::string const& matName = m.first;
                auto const& mat = m.second;
                auto itFindMat = markersByMaterial.find( matName );
                if ( itFindMat == markersByMaterial.end() )
                    continue;
                if ( itFindMat->second.empty() )
                    continue;
                auto const& matmarkers = itFindMat->second;
                auto range = markedelements( mesh,matmarkers );
                M_rangeMeshElementsByMaterial[matName] = range;

                if ( mat.hasPropertyExprScalar("k11") )
                {
                    auto const& expr = mat.propertyExprScalar("k11");
                    M_thermalConductivityByMaterial[matName].setExpr( expr );
                    M_thermalConductivityByMaterial[matName].setValue( 0 );//TODO
                    M_fieldThermalConductivity->on(_range=range,_expr=expr);
                }
                else
                {
                    double value = mat.propertyConstant("k11");
                    M_thermalConductivityByMaterial[matName].setValue( value );
                    M_fieldThermalConductivity->on(_range=range,_expr=cst(value));
                }

                if ( mat.hasPropertyExprScalar("rho") )
                {
                    auto const& expr = mat.propertyExprScalar("rho");
                    M_rhoByMaterial[matName].setExpr( expr );
                    M_rhoByMaterial[matName].setValue( 0 );//TODO
                    M_fieldRho->on(_range=range,_expr=expr);
                }
                else
                {
                    double value = mat.propertyConstant("rho");
                    M_rhoByMaterial[matName].setValue( value );
                    M_fieldRho->on(_range=range,_expr=cst(value));
                }

                if ( mat.hasPropertyExprScalar("Cp") )
                {
                    auto const& expr = mat.propertyExprScalar("Cp");
                    M_heatCapacityByMaterial[matName].setExpr( expr );
                    M_heatCapacityByMaterial[matName].setValue( 0 );//TODO
                    M_fieldHeatCapacity->on(_range=range,_expr=expr);
                }
                else
                {
                    double value = mat.propertyConstant("Cp");
                    M_heatCapacityByMaterial[matName].setValue( value );
                    M_fieldHeatCapacity->on(_range=range,_expr=cst(value));
                }

                if ( mat.hasPropertyExprScalar("beta") )
                {
                    auto const& expr = mat.propertyExprScalar("beta");
                    M_thermalExpansionByMaterial[matName].setExpr( expr );
                    M_thermalExpansionByMaterial[matName].setValue( 0 );//TODO
                    M_fieldThermalExpansion->on(_range=range,_expr=expr);
                }
                else
                {
                    double value = mat.propertyConstant("beta");
                    M_thermalExpansionByMaterial[matName].setValue( value );
                    M_fieldThermalExpansion->on(_range=range,_expr=cst(value));
                }
            }
        }

    std::set<std::string> const& markers() const { return M_markers; }

    bool isDefinedOnWholeMesh() const { return M_isDefinedOnWholeMesh; }

    std::map<std::string, elements_reference_wrapper_t<mesh_type> > const& rangeMeshElementsByMaterial() const { return M_rangeMeshElementsByMaterial; }

    bool hasMaterial( std::string const& matName ) const { return M_rangeMeshElementsByMaterial.find( matName ) != M_rangeMeshElementsByMaterial.end(); }

    element_type const& fieldThermalConductivity() const { return *M_fieldThermalConductivity; }
    element_type const& fieldHeatCapacity() const { return *M_fieldHeatCapacity; }
    element_type const& fieldRho() const { return *M_fieldRho; }
    element_type const& fieldThermalExpansion() const { return *M_fieldThermalExpansion; }
    element_ptrtype const& fieldThermalConductivityPtr() const { return M_fieldThermalConductivity; }
    element_ptrtype const& fieldHeatCapacityPtr() const { return M_fieldHeatCapacity; }
    element_ptrtype const& fieldRhoPtr() const { return M_fieldRho; }
    element_ptrtype const& fieldThermalExpansionPtr() const { return M_fieldThermalExpansion; }

    // thermal conductivity
    bool hasThermalConductivity( std::string const& matName ) const
        {
            return M_thermalConductivityByMaterial.find( matName ) != M_thermalConductivityByMaterial.end();
        }
    ModelExpressionScalar const& thermalConductivity( std::string const& matName ) const
        {
            CHECK( this->hasThermalConductivity( matName ) ) << "material name not registered : " << matName;
            return M_thermalConductivityByMaterial.find( matName )->second;
        }
    double cstThermalConductivity( std::string const& matName = "" ) const
        {
            if ( matName.empty() )
            {
                if ( M_thermalConductivityByMaterial.empty() )
                    return M_thermalConductivityDefaultValue;
                else
                    return M_thermalConductivityByMaterial.begin()->second.value();
            }
            auto itFindMat = M_thermalConductivityByMaterial.find( matName );
            CHECK( itFindMat != M_thermalConductivityByMaterial.end() ) << "material name not registered : " << matName;
            return itFindMat->second.value();
        }
    // rho
    bool hasRho( std::string const& matName ) const
        {
            return M_rhoByMaterial.find( matName ) != M_rhoByMaterial.end();
        }
    ModelExpressionScalar const& rho( std::string const& matName ) const
        {
            CHECK( this->hasRho( matName ) ) << "material name not registered : " << matName;
            return M_rhoByMaterial.find( matName )->second;
        }
    double cstRho( std::string const& matName = "" ) const
        {
            if ( matName.empty() )
            {
                if ( M_rhoByMaterial.empty() )
                    return M_rhoDefaultValue;
                else
                    return M_rhoByMaterial.begin()->second.value();
            }
            auto itFindMat = M_rhoByMaterial.find( matName );
            CHECK( itFindMat != M_rhoByMaterial.end() ) << "material name not registered : " << matName;
            return itFindMat->second.value();
        }
    // heat capacity
    bool hasHeatCapacity( std::string const& matName ) const
        {
            return M_heatCapacityByMaterial.find( matName ) != M_heatCapacityByMaterial.end();
        }
    ModelExpressionScalar const& heatCapacity( std::string const& matName ) const
        {
            CHECK( this->hasHeatCapacity( matName ) ) << "material name not registered : " << matName;
            return M_heatCapacityByMaterial.find( matName )->second;
        }
    double cstHeatCapacity( std::string const& matName = "" ) const
        {
            if ( matName.empty() )
            {
                if ( M_heatCapacityByMaterial.empty() )
                    return M_heatCapacityDefaultValue;
                else
                    return M_heatCapacityByMaterial.begin()->second.value();
            }
            auto itFindMat = M_heatCapacityByMaterial.find( matName );
            CHECK( itFindMat != M_heatCapacityByMaterial.end() ) << "material name not registered : " << matName;
            return itFindMat->second.value();
        }
    // thermal expansion
    bool hasThermalExpansion( std::string const& matName ) const
        {
            return M_thermalExpansionByMaterial.find( matName ) != M_thermalExpansionByMaterial.end();
        }
    ModelExpressionScalar const& thermalExpansion( std::string const& matName ) const
        {
            CHECK( this->hasThermalExpansion( matName ) ) << "material name not registered : " << matName;
            return M_thermalExpansionByMaterial.find( matName )->second;
        }
    double cstThermalExpansion( std::string const& matName = "" ) const
        {
            if ( matName.empty() )
            {
                if ( M_thermalExpansionByMaterial.empty() )
                    return M_thermalExpansionDefaultValue;
                else
                    return M_thermalExpansionByMaterial.begin()->second.value();
            }
            auto itFindMat = M_thermalExpansionByMaterial.find( matName );
            CHECK( itFindMat != M_thermalExpansionByMaterial.end() ) << "material name not registered : " << matName;
            return itFindMat->second.value();
        }


    boost::shared_ptr<std::ostringstream>
    getInfoMaterialParameters() const
        {
            boost::shared_ptr<std::ostringstream> ostr( new std::ostringstream() );
            *ostr << "\n   Materials parameters";
            *ostr << "\n     -- number of materials : " << M_rangeMeshElementsByMaterial.size();
            for ( auto const& matRange : M_rangeMeshElementsByMaterial)
            {
                std::string const& matName = matRange.first;
                *ostr << "\n     -- [" << matName << "] rho : ";
                if ( this->rho(matName).isConstant() )
                    *ostr << this->rho(matName).value();
                else
                    *ostr << str( this->rho(matName).expr().expression() );
                *ostr << "\n     -- [" << matName << "] thermal conductivity : ";
                if ( this->thermalConductivity(matName).isConstant() )
                    *ostr << this->thermalConductivity(matName).value();
                else
                    *ostr << str( this->thermalConductivity(matName).expr().expression() );
                *ostr << "\n     -- [" << matName << "] heat capacity : ";
                if ( this->heatCapacity(matName).isConstant() )
                    *ostr << this->heatCapacity(matName).value();
                else
                    *ostr << str( this->heatCapacity(matName).expr().expression() );
                *ostr << "\n     -- [" << matName << "] thermal expansion : ";
                if ( this->thermalExpansion(matName).isConstant() )
                    *ostr << this->thermalExpansion(matName).value();
                else
                    *ostr << str( this->thermalExpansion(matName).expr().expression() );
            }
            return ostr;
        }

private :
    std::set<std::string> M_markers;
    bool M_isDefinedOnWholeMesh;
    space_ptrtype M_space;
    std::map<std::string, elements_reference_wrapper_t<mesh_type> > M_rangeMeshElementsByMaterial;

    std::map<std::string, ModelExpressionScalar> M_thermalConductivityByMaterial, M_heatCapacityByMaterial, M_rhoByMaterial, M_thermalExpansionByMaterial;
    element_ptrtype M_fieldThermalConductivity, M_fieldHeatCapacity, M_fieldRho, M_fieldThermalExpansion;
    double M_thermalConductivityDefaultValue, M_heatCapacityDefaultValue, M_rhoDefaultValue, M_thermalExpansionDefaultValue;
};


} // namespace FeelModels
} // namespace Feel

#endif // __THERMALPROPERTIES_DESCRIPTION_H

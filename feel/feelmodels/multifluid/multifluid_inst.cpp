
#include "multifluidconfig.h"

#include <feel/feelmodels/multifluid/multifluid.cpp>

#include <feel/feelmodels/multifluid/helfrichforcemodel.hpp>
#include <feel/feelmodels/multifluid/inextensibilityforcemodel.hpp>
#include <feel/feelmodels/multifluid/collisionforcemodel.hpp>
#include <feel/feelmodels/multifluid/wallcontactforcemodel.hpp>

namespace Feel {
namespace FeelModels {

//template class MultiFluid< FLUIDMECHANICS_CLASS_TYPE, LEVELSET_CLASS_TYPE >;
template class MULTIFLUID_CLASS_INSTANTIATION;

// Register interface forces models
const bool helfrich_interfaceforcesmodel = 
    MULTIFLUID_CLASS_INSTANTIATION::interfaceforces_factory_type::instance().registerProduct( 
            "helfrich", 
            &detail::createInterfaceForcesModel<HelfrichForceModel, typename MULTIFLUID_CLASS_INSTANTIATION::levelset_type> );

const bool inextensibility_interfaceforcesmodel = 
    MULTIFLUID_CLASS_INSTANTIATION::interfaceforces_factory_type::instance().registerProduct( 
            "inextensibility-force", 
            &detail::createInterfaceForcesModel<InextensibilityForceModel, typename MULTIFLUID_CLASS_INSTANTIATION::levelset_type> );

const bool collision_interfaceforcesmodel = 
    MULTIFLUID_CLASS_INSTANTIATION::interfaceforces_factory_type::instance().registerProduct( 
            "collision-force", 
            &detail::createInterfaceForcesModel<CollisionForceModel, typename MULTIFLUID_CLASS_INSTANTIATION::levelset_type> );

const bool wallcontact_interfaceforcesmodel = 
    MULTIFLUID_CLASS_INSTANTIATION::interfaceforces_factory_type::instance().registerProduct( 
            "wallcontact-force", 
            &detail::createInterfaceForcesModel<WallContactForceModel, typename MULTIFLUID_CLASS_INSTANTIATION::levelset_type> );

}
}



feelpp_add_application( levelset_2d SRCS main_ls.cpp
  LINK_LIBRARIES
  feelpp_toolbox_levelset_lib_2dP1P1G1
  feelpp_toolbox_levelset_lib_2dP2P2G1
  DEFS FEELPP_DIM=2 LEVELSET_PN_ORDER=${FEELPP_MODELS_LEVELSET_PN_ORDER}
  INSTALL
)

feelpp_add_application( levelset_3d SRCS main_ls.cpp
  LINK_LIBRARIES
  feelpp_toolbox_levelset_lib_3dP1P1G1
  feelpp_toolbox_levelset_lib_3dP2P2G1
  DEFS FEELPP_DIM=3 LEVELSET_PN_ORDER=${FEELPP_MODELS_LEVELSET_PN_ORDER}
  INSTALL
)

foreach( _testcase IN ITEMS "square" "tube" "injector" )
  feelpp_add_testcase(${_testcase} PREFIX feelpp_toolbox_levelset CATEGORY toolboxes/levelset)
endforeach()

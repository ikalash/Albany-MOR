##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//

# SCOREC ##################

add_subdirectory(meshes)
add_subdirectory(Elasticity)
add_subdirectory(ElastAdapt)
add_subdirectory(J2Tet10)
add_subdirectory(J2Adapt)
add_subdirectory(MechWithTemp)
add_subdirectory(ThermoMechanicalCan)
add_subdirectory(BimetallicStrip)
# RCU is broken and likely may not be repaired
#add_subdirectory(RCU)
add_subdirectory(Restart)
add_subdirectory(Dynamics)
add_subdirectory(DOFOffset)
add_subdirectory(Sim)

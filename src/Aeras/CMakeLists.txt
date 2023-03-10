#//*****************************************************************//
#//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
#//    This Software is released under the BSD license detailed     //
#//    in the file "license.txt" in the top-level Albany directory  //
#//*****************************************************************//

  SET(SOURCES 
       evaluators/Aeras_ComputeBasisFunctions.cpp
       evaluators/Aeras_ShallowWaterResid.cpp
       evaluators/Aeras_Atmosphere_Moisture.cpp
       evaluators/Aeras_GatherCoordinateVector.cpp
       evaluators/Aeras_Hydrostatic_VelResid.cpp
       evaluators/Aeras_Hydrostatic_Velocity.cpp
       evaluators/Aeras_XZHydrostatic_Velocity.cpp
       evaluators/Aeras_SurfaceHeight.cpp
       evaluators/Aeras_VorticityLevels.cpp
       problems/Aeras_ShallowWaterProblem.cpp
       problems/Aeras_ShallowWaterProblemNoAD.cpp
       evaluators/Aeras_XZScalarAdvectionResid.cpp
       problems/Aeras_XZScalarAdvectionProblem.cpp
       evaluators/Aeras_XScalarAdvectionResid.cpp
       problems/Aeras_XScalarAdvectionProblem.cpp
       evaluators/Aeras_XZHydrostatic_VelResid.cpp
       evaluators/Aeras_XZHydrostatic_TemperatureResid.cpp
       evaluators/Aeras_XZHydrostatic_SPressureResid.cpp
       evaluators/Aeras_XZHydrostatic_TracerResid.cpp
       evaluators/Aeras_XZHydrostatic_Density.cpp
       evaluators/Aeras_XZHydrostatic_EtaDotPi.cpp
       evaluators/Aeras_Hydrostatic_EtaDot.cpp
       evaluators/Aeras_XZHydrostatic_EtaDot.cpp
       evaluators/Aeras_XZHydrostatic_GeoPotential.cpp
       evaluators/Aeras_XZHydrostatic_Omega.cpp
       evaluators/Aeras_XZHydrostatic_PiVel.cpp
       evaluators/Aeras_XZHydrostatic_Pressure.cpp
       evaluators/Aeras_XZHydrostatic_KineticEnergy.cpp
       evaluators/Aeras_XZHydrostatic_SurfaceGeopotential.cpp
       evaluators/Aeras_Hydrostatic_SurfaceGeopotential.cpp
       evaluators/Aeras_XZHydrostatic_UTracer.cpp
       evaluators/Aeras_XZHydrostatic_VirtualT.cpp
       problems/Aeras_XZHydrostaticProblem.cpp
       problems/Aeras_HydrostaticProblem.cpp
       problems/Aeras_Layouts.cpp
       problems/Aeras_Dimension.cpp
       evaluators/Aeras_GatherSolution.cpp
       evaluators/Aeras_ScatterResidual.cpp
       evaluators/Aeras_ComputeAndScatterJac.cpp
       evaluators/Aeras_SW_ComputeAndScatterJac.cpp
       evaluators/Aeras_DOFInterpolation.cpp
       evaluators/Aeras_DOFInterpolationLevels.cpp
       evaluators/Aeras_DOFVecInterpolationLevels.cpp
       evaluators/Aeras_DOFGradInterpolation.cpp
       evaluators/Aeras_DOFGradInterpolationLevels.cpp
       evaluators/Aeras_DOFDivInterpolationLevels.cpp
       evaluators/Aeras_DOFDivInterpolationLevelsXZ.cpp
       evaluators/Aeras_DOFDInterpolationLevels.cpp
       evaluators/Aeras_Atmosphere_Moisture.cpp
       evaluators/Aeras_ShallowWaterSource.cpp
       evaluators/Aeras_ShallowWaterHyperViscosity.cpp
       evaluators/Aeras_ShallowWaterResponseL2Error.cpp
       evaluators/Aeras_ShallowWaterResponseL2Norm.cpp
       evaluators/Aeras_HydrostaticResponseL2Norm.cpp
       evaluators/Aeras_HydrostaticResponseL2Error.cpp
       responses/Aeras_TotalVolume.cpp
       responses/Aeras_LayeredResponseUtilities.cpp

   )
   

  SET(HEADERS
       evaluators/Aeras_Eta.hpp
       evaluators/Aeras_ComputeBasisFunctions_Def.hpp
       evaluators/Aeras_ComputeBasisFunctions.hpp
       evaluators/Aeras_Atmosphere_Moisture_Def.hpp
       evaluators/Aeras_Atmosphere_Moisture.hpp
       evaluators/Aeras_ShallowWaterConstants.hpp
       evaluators/Aeras_SurfaceHeight.hpp
       evaluators/Aeras_SurfaceHeight_Def.hpp
       evaluators/Aeras_GatherCoordinateVector_Def.hpp
       evaluators/Aeras_GatherCoordinateVector.hpp
       evaluators/Aeras_Hydrostatic_VelResid_Def.hpp
       evaluators/Aeras_Hydrostatic_VelResid.hpp
       evaluators/Aeras_Hydrostatic_Velocity_Def.hpp
       evaluators/Aeras_Hydrostatic_Velocity.hpp
       evaluators/Aeras_XZHydrostatic_Velocity_Def.hpp
       evaluators/Aeras_XZHydrostatic_Velocity.hpp
       problems/Aeras_ShallowWaterProblem.hpp
       problems/Aeras_ShallowWaterProblemNoAD.hpp
       evaluators/Aeras_XZScalarAdvectionResid_Def.hpp
       evaluators/Aeras_XZScalarAdvectionResid.hpp
       problems/Aeras_XZScalarAdvectionProblem.hpp
       evaluators/Aeras_XScalarAdvectionResid_Def.hpp
       evaluators/Aeras_XScalarAdvectionResid.hpp
       problems/Aeras_XScalarAdvectionProblem.hpp
       problems/Aeras_XZHydrostaticProblem.hpp
       problems/Aeras_HydrostaticProblem.hpp
       evaluators/Aeras_VorticityLevels_Def.hpp
       evaluators/Aeras_VorticityLevels.hpp
       evaluators/Aeras_XZHydrostatic_VelResid_Def.hpp
       evaluators/Aeras_XZHydrostatic_VelResid.hpp
       evaluators/Aeras_XZHydrostatic_TemperatureResid_Def.hpp
       evaluators/Aeras_XZHydrostatic_TemperatureResid.hpp
       evaluators/Aeras_XZHydrostatic_SPressureResid_Def.hpp
       evaluators/Aeras_XZHydrostatic_SPressureResid.hpp
       evaluators/Aeras_XZHydrostatic_TracerResid_Def.hpp
       evaluators/Aeras_XZHydrostatic_TracerResid.hpp
       evaluators/Aeras_XZHydrostatic_Pressure_Def.hpp
       evaluators/Aeras_XZHydrostatic_Pressure.hpp
       evaluators/Aeras_XZHydrostatic_Density_Def.hpp
       evaluators/Aeras_XZHydrostatic_Density.hpp
       evaluators/Aeras_XZHydrostatic_EtaDotPi_Def.hpp
       evaluators/Aeras_XZHydrostatic_EtaDotPi.hpp
       evaluators/Aeras_Hydrostatic_EtaDot_Def.hpp
       evaluators/Aeras_Hydrostatic_EtaDot.hpp
       evaluators/Aeras_XZHydrostatic_EtaDot_Def.hpp
       evaluators/Aeras_XZHydrostatic_EtaDot.hpp
       evaluators/Aeras_XZHydrostatic_GeoPotential_Def.hpp
       evaluators/Aeras_XZHydrostatic_GeoPotential.hpp
       evaluators/Aeras_XZHydrostatic_Omega_Def.hpp
       evaluators/Aeras_XZHydrostatic_Omega.hpp
       evaluators/Aeras_XZHydrostatic_PiVel_Def.hpp
       evaluators/Aeras_XZHydrostatic_PiVel.hpp
       evaluators/Aeras_XZHydrostatic_KineticEnergy_Def.hpp
       evaluators/Aeras_XZHydrostatic_KineticEnergy.hpp
       evaluators/Aeras_XZHydrostatic_SurfaceGeopotential_Def.hpp
       evaluators/Aeras_XZHydrostatic_SurfaceGeopotential.hpp
       evaluators/Aeras_Hydrostatic_SurfaceGeopotential_Def.hpp
       evaluators/Aeras_Hydrostatic_SurfaceGeopotential.hpp
       evaluators/Aeras_XZHydrostatic_UTracer_Def.hpp
       evaluators/Aeras_XZHydrostatic_UTracer.hpp
       evaluators/Aeras_XZHydrostatic_VirtualT_Def.hpp
       evaluators/Aeras_XZHydrostatic_VirtualT.hpp
       evaluators/Aeras_ShallowWaterResponseL2Error.hpp
       evaluators/Aeras_ShallowWaterResponseL2Error_Def.hpp
       evaluators/Aeras_ShallowWaterResponseL2Norm.hpp
       evaluators/Aeras_ShallowWaterResponseL2Norm_Def.hpp
       evaluators/Aeras_HydrostaticResponseL2Norm.hpp
       evaluators/Aeras_HydrostaticResponseL2Norm_Def.hpp
       evaluators/Aeras_HydrostaticResponseL2Error.hpp
       evaluators/Aeras_HydrostaticResponseL2Error_Def.hpp
       problems/Aeras_Layouts.hpp
       problems/Aeras_Dimension.hpp
       evaluators/Aeras_GatherSolution.hpp
       evaluators/Aeras_GatherSolution_Def.hpp
       evaluators/Aeras_ScatterResidual.hpp
       evaluators/Aeras_ScatterResidual_Def.hpp
       evaluators/Aeras_ComputeAndScatterJac.hpp
       evaluators/Aeras_ComputeAndScatterJac_Def.hpp
       evaluators/Aeras_SW_ComputeAndScatterJac.hpp
       evaluators/Aeras_SW_ComputeAndScatterJac_Def.hpp
       evaluators/Aeras_DOFInterpolation.hpp
       evaluators/Aeras_DOFInterpolation_Def.hpp
       evaluators/Aeras_DOFInterpolationLevels.hpp
       evaluators/Aeras_DOFInterpolationLevels_Def.hpp
       evaluators/Aeras_DOFVecInterpolationLevels.hpp
       evaluators/Aeras_DOFVecInterpolationLevels_Def.hpp
       evaluators/Aeras_DOFGradInterpolation.hpp
       evaluators/Aeras_DOFGradInterpolation_Def.hpp
       evaluators/Aeras_DOFGradInterpolationLevels.hpp
       evaluators/Aeras_DOFGradInterpolationLevels_Def.hpp
       evaluators/Aeras_DOFDivInterpolationLevels.hpp
       evaluators/Aeras_DOFDivInterpolationLevels_Def.hpp
       evaluators/Aeras_DOFDivInterpolationLevelsXZ.hpp
       evaluators/Aeras_DOFDivInterpolationLevelsXZ_Def.hpp
       evaluators/Aeras_DOFDInterpolationLevels.hpp
       evaluators/Aeras_DOFDInterpolationLevels_Def.hpp
       evaluators/Aeras_Atmosphere_Moisture.hpp
       evaluators/Aeras_Atmosphere_Moisture_Def.hpp
       evaluators/Aeras_ShallowWaterSource_Def.hpp
       evaluators/Aeras_ShallowWaterSource.hpp
       evaluators/Aeras_ShallowWaterHyperViscosity_Def.hpp
       evaluators/Aeras_ShallowWaterHyperViscosity.hpp
       responses/Aeras_TotalVolume.hpp
       responses/Aeras_TotalVolume_Def.hpp
       responses/Aeras_LayeredResponseUtilities.hpp
       responses/Aeras_LayeredResponseUtilities_Def.hpp
  )

SET(HEADERS ${HEADERS}
    Aeras_HVDecorator.hpp
)
SET(SOURCES ${SOURCES}
    Aeras_HVDecorator.cpp
)
  
include_directories (${Trilinos_INCLUDE_DIRS}  ${Trilinos_TPL_INCLUDE_DIRS}
                     ${Albany_SOURCE_DIR}/src ${Albany_SOURCE_DIR}/src/evaluators 
                     ${Albany_SOURCE_DIR}/src/problems ${Albany_SOURCE_DIR}/src/responses 
                     ./problems ./evaluators ./responses
                    )

add_library(Aeras ${Albany_LIBRARY_TYPE} ${SOURCES} ${HEADERS})

set_target_properties(Aeras PROPERTIES PUBLIC_HEADER "${HEADERS}")

IF (INSTALL_ALBANY)
  install(TARGETS Aeras EXPORT albany-export
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}/"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}/"
    PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}")
ENDIF()

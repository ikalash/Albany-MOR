##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//
# LandIce


  SET(SOURCES
       evaluators/LandIce_BasalFrictionCoefficient.cpp
       evaluators/LandIce_BasalFrictionCoefficientGradient.cpp
       evaluators/LandIce_BasalFrictionHeat.cpp
       evaluators/LandIce_BasalMeltRate.cpp
       evaluators/LandIce_DirichletEnthalpySurface.cpp
       evaluators/LandIce_Dissipation.cpp
       evaluators/LandIce_DOFDivInterpolationSide.cpp
       evaluators/LandIce_EffectivePressure.cpp
       evaluators/LandIce_EnthalpyResid.cpp
       evaluators/LandIce_EnthalpyBasalResid.cpp
       evaluators/LandIce_EpsilonL1L2.cpp
       evaluators/LandIce_FlowRate.cpp
       evaluators/LandIce_FluxDiv.cpp
       evaluators/LandIce_Gather2DField.cpp
       evaluators/LandIce_GatherVerticallyAveragedVelocity.cpp
       evaluators/LandIce_GeoFluxHeat.cpp
       evaluators/LandIce_HydraulicPotential.cpp
       evaluators/LandIce_HydrologyBasalGravitationalWaterPotential.cpp
       evaluators/LandIce_HydrologyMeltingRate.cpp
       evaluators/LandIce_HydrologyResidualCavitiesEqn.cpp
       evaluators/LandIce_HydrologyResidualMassEqn.cpp
       evaluators/LandIce_HydrologyResidualTillStorageEqn.cpp
       evaluators/LandIce_HydrologySurfaceWaterInput.cpp
       evaluators/LandIce_HydrologyWaterDischarge.cpp
       evaluators/LandIce_HydrologyWaterThickness.cpp
       evaluators/LandIce_HydrostaticPressure.cpp
       evaluators/LandIce_IceOverburden.cpp
       evaluators/LandIce_IceSoftness.cpp
       evaluators/LandIce_Integral1Dw_Z.cpp
       evaluators/LandIce_LaplacianRegularizationResidual.cpp
       evaluators/LandIce_L2ProjectedBoundaryLaplacianResidual.cpp
       evaluators/LandIce_LiquidWaterFraction.cpp
       evaluators/LandIce_MapThickness.cpp
       evaluators/LandIce_PressureMeltingTemperature.cpp
       evaluators/LandIce_PressureCorrectedTemperature.cpp
       evaluators/LandIce_PressureMeltingEnthalpy.cpp
       evaluators/LandIce_ProlongateVector.cpp
       evaluators/LandIce_ResponseSurfaceVelocityMismatch.cpp
       evaluators/LandIce_ResponseSMBMismatch.cpp
       evaluators/LandIce_ResponseBoundarySquaredL2Norm.cpp
       evaluators/LandIce_ScatterResidual2D.cpp
       evaluators/LandIce_SimpleOperationEvaluator.cpp
       evaluators/LandIce_StackFields.cpp
       evaluators/LandIce_StokesMomentumResid.cpp
       evaluators/LandIce_StokesFOBasalResid.cpp
       evaluators/LandIce_StokesFOLateralResid.cpp
       evaluators/LandIce_StokesFOResid.cpp
       evaluators/LandIce_StokesFOStress.cpp
       evaluators/LandIce_StokesContinuityResid.cpp
       evaluators/LandIce_StokesContravarientMetricTensor.cpp
       evaluators/LandIce_StokesBodyForce.cpp
       evaluators/LandIce_StokesFOBodyForce.cpp
       evaluators/LandIce_StokesFOImplicitThicknessUpdateResid.cpp
       evaluators/LandIce_StokesL1L2BodyForce.cpp
       evaluators/LandIce_StokesL1L2Resid.cpp
       evaluators/LandIce_StokesRm.cpp
       evaluators/LandIce_StokesTauM.cpp
       evaluators/LandIce_Temperature.cpp
       evaluators/LandIce_ThicknessResid.cpp
       evaluators/LandIce_UpdateZCoordinate.cpp
       evaluators/LandIce_VerticalVelocity.cpp
       evaluators/LandIce_Viscosity.cpp
       evaluators/LandIce_ViscosityFO.cpp
       evaluators/LandIce_ViscosityL1L2.cpp
       evaluators/LandIce_w_ZResid.cpp
       evaluators/LandIce_w_Resid.cpp
       evaluators/LandIce_Time.cpp
       problems/LandIce_Enthalpy.cpp
       problems/LandIce_Hydrology.cpp
       problems/LandIce_LaplacianSampling.cpp
       problems/LandIce_ProblemFactory.cpp
       problems/LandIce_SchoofFit.cpp
       problems/LandIce_Stokes.cpp
       problems/LandIce_StokesFO.cpp
       problems/LandIce_StokesFOThermoCoupled.cpp
       problems/LandIce_StokesFOThickness.cpp
       problems/LandIce_StokesL1L2.cpp
   )

IF(ENABLE_MPAS_INTERFACE)
   SET(SOURCESMPAS
       interface_with_mpas/Albany_MpasSTKMeshStruct.cpp
       interface_with_mpas/Interface.cpp
   )
ENDIF()

IF(ENABLE_CISM_INTERFACE)
   SET(SOURCESCISM
       interface_with_cism/Albany_CismSTKMeshStruct.cpp
       interface_with_cism/ali_driver.cpp
       evaluators/LandIce_CismSurfaceGradFO.cpp
   )
ENDIF()

  SET(HEADERS
       evaluators/LandIce_BasalFrictionCoefficient.hpp
       evaluators/LandIce_BasalFrictionCoefficient_Def.hpp
       evaluators/LandIce_BasalFrictionCoefficientGradient.hpp
       evaluators/LandIce_BasalFrictionCoefficientGradient_Def.hpp
       evaluators/LandIce_BasalFrictionHeat.hpp
       evaluators/LandIce_BasalFrictionHeat_Def.hpp
       evaluators/LandIce_BasalMeltRate.hpp
       evaluators/LandIce_BasalMeltRate_Def.hpp
       evaluators/LandIce_DirichletEnthalpySurface.hpp
       evaluators/LandIce_DirichletEnthalpySurface_Def.hpp
       evaluators/LandIce_Dissipation.hpp
       evaluators/LandIce_Dissipation_Def.hpp
       evaluators/LandIce_DOFDivInterpolationSide.hpp
       evaluators/LandIce_DOFDivInterpolationSide_Def.hpp
       evaluators/LandIce_EffectivePressure.hpp
       evaluators/LandIce_EffectivePressure_Def.hpp
       evaluators/LandIce_EnthalpyResid.hpp
       evaluators/LandIce_EnthalpyResid_Def.hpp
       evaluators/LandIce_EnthalpyBasalResid.hpp
       evaluators/LandIce_EnthalpyBasalResid_Def.hpp
       evaluators/LandIce_EpsilonL1L2.hpp
       evaluators/LandIce_EpsilonL1L2_Def.hpp
       evaluators/LandIce_FlowRate.hpp
       evaluators/LandIce_FlowRate_Def.hpp
       evaluators/LandIce_FluxDiv.hpp
       evaluators/LandIce_FluxDiv_Def.hpp
       evaluators/LandIce_Gather2DField.hpp
       evaluators/LandIce_Gather2DField_Def.hpp
       evaluators/LandIce_GatherVerticallyAveragedVelocity.hpp
       evaluators/LandIce_GatherVerticallyAveragedVelocity_Def.hpp
       evaluators/LandIce_GeoFluxHeat.hpp
       evaluators/LandIce_GeoFluxHeat_Def.hpp
       evaluators/LandIce_HydraulicPotential.hpp
       evaluators/LandIce_HydraulicPotential_Def.hpp
       evaluators/LandIce_HydrologyBasalGravitationalWaterPotential.hpp
       evaluators/LandIce_HydrologyBasalGravitationalWaterPotential_Def.hpp
       evaluators/LandIce_HydrologyMeltingRate.hpp
       evaluators/LandIce_HydrologyMeltingRate_Def.hpp
       evaluators/LandIce_HydrologyResidualCavitiesEqn.hpp
       evaluators/LandIce_HydrologyResidualCavitiesEqn_Def.hpp
       evaluators/LandIce_HydrologyResidualMassEqn.hpp
       evaluators/LandIce_HydrologyResidualMassEqn_Def.hpp
       evaluators/LandIce_HydrologyResidualTillStorageEqn.hpp
       evaluators/LandIce_HydrologyResidualTillStorageEqn_Def.hpp
       evaluators/LandIce_HydrologySurfaceWaterInput.hpp
       evaluators/LandIce_HydrologySurfaceWaterInput_Def.hpp
       evaluators/LandIce_HydrologyWaterDischarge.hpp
       evaluators/LandIce_HydrologyWaterDischarge_Def.hpp
       evaluators/LandIce_HydrologyWaterThickness.hpp
       evaluators/LandIce_HydrologyWaterThickness_Def.hpp
       evaluators/LandIce_HydrostaticPressure.hpp
       evaluators/LandIce_HydrostaticPressure_Def.hpp
       evaluators/LandIce_IceOverburden.hpp
       evaluators/LandIce_IceOverburden_Def.hpp
       evaluators/LandIce_IceSoftness.hpp
       evaluators/LandIce_IceSoftness_Def.hpp
       evaluators/LandIce_Integral1Dw_Z.hpp
       evaluators/LandIce_Integral1Dw_Z_Def.hpp
       evaluators/LandIce_LaplacianRegularizationResidual.hpp
       evaluators/LandIce_LaplacianRegularizationResidual_Def.hpp
       evaluators/LandIce_L2ProjectedBoundaryLaplacianResidual.hpp
       evaluators/LandIce_L2ProjectedBoundaryLaplacianResidual_Def.hpp
       evaluators/LandIce_LiquidWaterFraction.hpp
       evaluators/LandIce_LiquidWaterFraction_Def.hpp
       evaluators/LandIce_MapThickness_Def.hpp
       evaluators/LandIce_MapThickness.hpp
       evaluators/LandIce_PressureCorrectedTemperature.hpp
       evaluators/LandIce_PressureCorrectedTemperature_Def.hpp
       evaluators/LandIce_PressureMeltingTemperature.hpp
       evaluators/LandIce_PressureMeltingTemperature_Def.hpp
       evaluators/LandIce_PressureMeltingEnthalpy.hpp
       evaluators/LandIce_PressureMeltingEnthalpy_Def.hpp
       evaluators/LandIce_ProlongateVector.hpp
       evaluators/LandIce_ProlongateVector_Def.hpp
       evaluators/LandIce_ResponseSurfaceVelocityMismatch.hpp
       evaluators/LandIce_ResponseSurfaceVelocityMismatch_Def.hpp
       evaluators/LandIce_ResponseBoundarySquaredL2Norm.hpp
       evaluators/LandIce_ResponseBoundarySquaredL2Norm_Def.hpp
       evaluators/LandIce_ResponseSMBMismatch.hpp
       evaluators/LandIce_ResponseSMBMismatch_Def.hpp
       evaluators/LandIce_ScatterResidual2D.hpp
       evaluators/LandIce_ScatterResidual2D_Def.hpp
       evaluators/LandIce_SharedParameter.hpp
       evaluators/LandIce_SimpleOperation.hpp
       evaluators/LandIce_SimpleOperationEvaluator.hpp
       evaluators/LandIce_SimpleOperationEvaluator_Def.hpp
       evaluators/LandIce_StackFields.hpp
       evaluators/LandIce_StackFields_Def.hpp
       evaluators/LandIce_StokesBodyForce.hpp
       evaluators/LandIce_StokesBodyForce_Def.hpp
       evaluators/LandIce_StokesContinuityResid.hpp
       evaluators/LandIce_StokesContinuityResid_Def.hpp
       evaluators/LandIce_StokesContravarientMetricTensor.hpp
       evaluators/LandIce_StokesContravarientMetricTensor_Def.hpp
       evaluators/LandIce_StokesFOBodyForce.hpp
       evaluators/LandIce_StokesFOBodyForce_Def.hpp
       evaluators/LandIce_StokesFOLateralResid.hpp
       evaluators/LandIce_StokesFOLateralResid_Def.hpp
       evaluators/LandIce_StokesFOBasalResid.hpp
       evaluators/LandIce_StokesFOBasalResid_Def.hpp
       evaluators/LandIce_StokesFOImplicitThicknessUpdateResid.hpp
       evaluators/LandIce_StokesFOImplicitThicknessUpdateResid_Def.hpp
       evaluators/LandIce_StokesFOResid.hpp
       evaluators/LandIce_StokesFOResid_Def.hpp
       evaluators/LandIce_StokesFOStress.hpp
       evaluators/LandIce_StokesFOStress_Def.hpp
       evaluators/LandIce_StokesL1L2Resid_Def.hpp
       evaluators/LandIce_StokesL1L2Resid.hpp
       evaluators/LandIce_StokesMomentumResid.hpp
       evaluators/LandIce_StokesMomentumResid_Def.hpp
       evaluators/LandIce_StokesRm.hpp
       evaluators/LandIce_StokesRm_Def.hpp
       evaluators/LandIce_StokesTauM.hpp
       evaluators/LandIce_StokesTauM_Def.hpp
       evaluators/LandIce_Temperature.hpp
       evaluators/LandIce_Temperature_Def.hpp
       evaluators/LandIce_ThicknessResid_Def.hpp
       evaluators/LandIce_ThicknessResid.hpp
       evaluators/LandIce_UpdateZCoordinate.hpp
       evaluators/LandIce_UpdateZCoordinate_Def.hpp
       evaluators/LandIce_VerticalVelocity.hpp
       evaluators/LandIce_VerticalVelocity_Def.hpp
       evaluators/LandIce_ViscosityFO.hpp
       evaluators/LandIce_ViscosityFO_Def.hpp
       evaluators/LandIce_ViscosityL1L2.hpp
       evaluators/LandIce_ViscosityL1L2_Def.hpp
       evaluators/LandIce_w_ZResid.hpp
       evaluators/LandIce_w_ZResid_Def.hpp
       evaluators/LandIce_w_Resid.hpp
       evaluators/LandIce_w_Resid_Def.hpp
       evaluators/LandIce_Time.hpp
       evaluators/LandIce_Time_Def.hpp
       problems/LandIce_Enthalpy.hpp
       problems/LandIce_Hydrology.hpp
       problems/LandIce_LaplacianSampling.hpp
       problems/LandIce_ParamEnum.hpp
       problems/LandIce_ProblemFactory.hpp
       problems/LandIce_SchoofFit.hpp
       problems/LandIce_Stokes.hpp
       problems/LandIce_StokesFO.hpp
       problems/LandIce_StokesFOThermoCoupled.hpp
       problems/LandIce_StokesFOThickness.hpp
       problems/LandIce_StokesL1L2.hpp
  )

IF(ENABLE_MPAS_INTERFACE)
  SET(HEADERSMPAS
       interface_with_mpas/Interface.hpp
  )
ENDIF()

IF (ENABLE_CISM_INTERFACE)
  SET(HEADERSCISM
       interface_with_cism/ali_driver.H
       evaluators/LandIce_CismSurfaceGradFO_Def.hpp
       evaluators/LandIce_CismSurfaceGradFO.hpp
   )
ENDIF()

IF(ENABLE_CISM_INTERFACE)
  IF (ENABLE_CISM_EPETRA)
    add_library(cismInterfaceE ${Albany_LIBRARY_TYPE} ${SOURCESCISM}
                ${CISM_INCLUDE_DIR}/DyCoreToGlimmer.cpp
                ${HEADERSCISM} ${CISM_INCLUDE_DIR}/DyCoreToGlimmer.H)
    set_target_properties(cismInterfaceE PROPERTIES PUBLIC_HEADER "${HEADERSCISM}")
    include_directories (${Trilinos_INCLUDE_DIRS}  ${Trilinos_TPL_INCLUDE_DIRS}
                         ${CISM_INCLUDE_DIR} ${CISM_INCLUDE_DIR}/ALI
                         ${Albany_SOURCE_DIR}/src ${Albany_SOURCE_DIR}/src/evaluators
                         ${Albany_SOURCE_DIR}/src/problems
                         ${Albany_SOURCE_DIR}/src/LandIce/interface_with_cism ${Albany_SOURCE_DIR}/src/responses
                         ./problems ./evaluators
                        )
    IF (INSTALL_ALBANY)
      install(TARGETS cismInterfaceE EXPORT albany-export
        LIBRARY DESTINATION "${LIB_INSTALL_DIR}/"
        ARCHIVE DESTINATION "${LIB_INSTALL_DIR}/"
        PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}")
    ENDIF()
  ELSE()
    add_library(cismInterface ${Albany_LIBRARY_TYPE} ${SOURCESCISM}
                ${CISM_INCLUDE_DIR}/DyCoreToGlimmer.cpp
                ${HEADERSCISM} ${CISM_INCLUDE_DIR}/DyCoreToGlimmer.H)
    set_target_properties(cismInterface PROPERTIES PUBLIC_HEADER "${HEADERSCISM}")
    include_directories (${Trilinos_INCLUDE_DIRS}  ${Trilinos_TPL_INCLUDE_DIRS}
                         ${CISM_INCLUDE_DIR} ${CISM_INCLUDE_DIR}/ALI
                         ${Albany_SOURCE_DIR}/src ${Albany_SOURCE_DIR}/src/evaluators
                         ${Albany_SOURCE_DIR}/src/problems
                         ${Albany_SOURCE_DIR}/src/LandIce/interface_with_cism ${Albany_SOURCE_DIR}/src/responses
                         ./problems ./evaluators
                        )
    IF (INSTALL_ALBANY)
      install(TARGETS cismInterface EXPORT albany-export
        LIBRARY DESTINATION "${LIB_INSTALL_DIR}/"
        ARCHIVE DESTINATION "${LIB_INSTALL_DIR}/"
        PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}")
    ENDIF()
  ENDIF()
ELSE()
  include_directories (${Trilinos_INCLUDE_DIRS}  ${Trilinos_TPL_INCLUDE_DIRS}
                       ${Albany_SOURCE_DIR}/src ${Albany_SOURCE_DIR}/src/evaluators
                       ${Albany_SOURCE_DIR}/src/problems ${Albany_SOURCE_DIR}/src/responses
                       ./problems ./evaluators
                      )
ENDIF()

add_library(LandIce ${Albany_LIBRARY_TYPE} ${SOURCES} ${HEADERS})
set_target_properties(LandIce PROPERTIES PUBLIC_HEADER "${HEADERS}")

IF(ENABLE_MPAS_INTERFACE)
  add_library(mpasInterface STATIC ${SOURCESMPAS} ${HEADERSMPAS})
  set_target_properties(mpasInterface PROPERTIES PUBLIC_HEADER "${HEADERSMPAS}")
  IF (INSTALL_ALBANY)
    install(TARGETS mpasInterface
      LIBRARY DESTINATION "${LIB_INSTALL_DIR}/"
      ARCHIVE DESTINATION "${LIB_INSTALL_DIR}/"
      PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}")
  ENDIF()
ENDIF()

IF (INSTALL_ALBANY)
  install(TARGETS LandIce EXPORT albany-export
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}/"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}/"
    PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}")
ENDIF()



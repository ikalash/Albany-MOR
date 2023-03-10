##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//

SET(SOURCES
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_StiffnessObjective.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_InterfaceEnergy.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ModalObjective.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_InternalEnergyResponse.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TensorPNormResponse.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TensorAverageResponse.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_HomogenizedConstantsResponse.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_VectorResidual.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Stress.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ScaleVector.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_BodyForce.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_FixedFieldTerm.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_NeumannTerm.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_DirichletTerm.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_CreateField.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_AddForce.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TopologyWeighting.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TopologyFieldWeighting.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Mixture.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Mixture_DistParam.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/LinearElasticityProblem.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/LinearElasticityModalProblem.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/PoissonsEquation.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/ATO_OptimizationProblem.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/ATO_Utils.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_TopoTools.cpp
)

IF (ALBANY_STOKHOS)
  SET (SOURCES ${SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ResidualStrain.cpp
  )
ENDIF()

IF (ALBANY_EPETRA)
  IF (ALBANY_ATO_SOLVER_EPETRA)
    SET(SOURCES ${SOURCES}
      ${CMAKE_SOURCE_DIR}/src/ATO/ATO_SolverEpetra.cpp
    )
  ELSE()
    SET(SOURCES ${SOURCES}
      ${CMAKE_SOURCE_DIR}/src/ATO/ATO_Solver.cpp
    )
  ENDIF()
  SET(SOURCES ${SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Aggregator.cpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Optimizer.cpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Pareto_Optimizer.cpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_XFEM_Preconditioner.cpp
  )
ENDIF()

SET(SOURCES ${SOURCES}
  ${CMAKE_SOURCE_DIR}/src/ATO/ATOT_Solver.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_Aggregator.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_Optimizer.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_Pareto_Optimizer.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_XFEM_Preconditioner.cpp
)

IF (ENABLE_DOTK)
  SET(SOURCES ${SOURCES}
  IF (ALBANY_EPETRA)
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_DOTk_Optimizer.cpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_DOTk_ContinuousOperators.cpp
  ENDIF()
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_DOTk_Optimizer.cpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_DOTk_ContinuousOperators.cpp
  )
ENDIF()

IF (ENABLE_Cogent)
  SET(SOURCES ${SOURCES}
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ComputeBasisFunctions.cpp
  )
ENDIF()

SET(HEADERS
  ${CMAKE_SOURCE_DIR}/src/ATO/ATO_Types.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_StiffnessObjective_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_StiffnessObjective.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_InterfaceEnergy.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_InterfaceEnergy_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ModalObjective_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ModalObjective.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_InternalEnergyResponse_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_InternalEnergyResponse.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TensorPNormResponse_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TensorPNormResponse.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TensorAverageResponse_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TensorAverageResponse.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_HomogenizedConstantsResponse_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_HomogenizedConstantsResponse.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_VectorResidual_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_VectorResidual.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Stress_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Stress.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ScaleVector_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ScaleVector.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_BodyForce_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_BodyForce.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_FixedFieldTerm_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_FixedFieldTerm.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_NeumannTerm_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_NeumannTerm.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_DirichletTerm_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_DirichletTerm.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_CreateField.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_CreateField_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_AddForce_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_AddForce.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TopologyWeighting_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TopologyWeighting.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TopologyFieldWeighting_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_TopologyFieldWeighting.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Mixture_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Mixture.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Mixture_DistParam_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_Mixture_DistParam.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/ATO_Utils.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/ATO_Utils_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/LinearElasticityProblem.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/LinearElasticityModalProblem.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/PoissonsEquation.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/problems/ATO_OptimizationProblem.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_TopoTools.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_TopoTools_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Integrator.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Integrator_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_PenaltyModel.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_PenaltyModel_Def.hpp
)

IF (ALBANY_STOKHOS)
  SET (HEADERS ${HEADERS}
    ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ResidualStrain.hpp
    ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ResidualStrain_Def.hpp
  )
ENDIF()

IF (ALBANY_EPETRA)
  IF (ALBANY_ATO_SOLVER_EPETRA)
    SET (HEADERS
      ${CMAKE_SOURCE_DIR}/src/ATO/ATO_SolverEpetra.hpp
    )
  ELSE()
    SET (HEADERS
      ${CMAKE_SOURCE_DIR}/src/ATO/ATO_Solver.hpp
    )
  ENDIF()
  SET (HEADERS
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Aggregator.hpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Optimizer.hpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_Pareto_Optimizer.hpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_XFEM_Preconditioner.hpp
  )
ENDIF()

SET(HEADERS
  ${CMAKE_SOURCE_DIR}/src/ATO/ATOT_Solver.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_Aggregator.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_Optimizer.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_Pareto_Optimizer.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_XFEM_Preconditioner.hpp
)

IF (ENABLE_DOTK)
  SET(HEADERS ${HEADERS}
    IF (ALBANY_EPETRA)
      ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_DOTk_Optimizer.hpp
      ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_DOTk_ContinuousOperators.hpp
    ENDIF()
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_DOTk_Optimizer.hpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATOT_DOTk_ContinuousOperators.hpp
    ${CMAKE_SOURCE_DIR}/src/ATO/utils/ATO_DOTk_vector.hpp
  )
ENDIF()

IF (ENABLE_Cogent)
  SET(HEADERS ${HEADERS}
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ComputeBasisFunctions_Def.hpp
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators/ATO_ComputeBasisFunctions.hpp
  )
ENDIF()

include_directories (${Trilinos_INCLUDE_DIRS}  ${Trilinos_TPL_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/evaluators
  ${CMAKE_SOURCE_DIR}/src/problems
  ${CMAKE_SOURCE_DIR}/src/responses
  ${CMAKE_SOURCE_DIR}/src/LCM/
  ${CMAKE_SOURCE_DIR}/src/LCM/evaluators
  ${CMAKE_SOURCE_DIR}/src/LCM/utils
  ${CMAKE_SOURCE_DIR}/src/ATO/
  ${CMAKE_SOURCE_DIR}/src/ATO/evaluators
  ${CMAKE_SOURCE_DIR}/src/ATO/problems
  ${CMAKE_SOURCE_DIR}/src/ATO/utils
)

add_library(ATO ${Albany_LIBRARY_TYPE} ${SOURCES} ${HEADERS})

IF ( INSTALL_ALBANY )
    install(TARGETS ATO EXPORT Albany
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}/"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}/"
    PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}")
ENDIF()


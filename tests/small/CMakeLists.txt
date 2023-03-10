##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//

IF(ALBANY_STK) # STK is needed for all these

# Heat Transfer Problems ###############
  add_subdirectory(SteadyHeat1D)
  add_subdirectory(SteadyHeat2D)
  add_subdirectory(SteadyHeatConstrainedOpt2D)
  add_subdirectory(SteadyHeat3D)
  add_subdirectory(SteadyHeat3DTest)
  add_subdirectory(SteadyHeat1DEB)
  add_subdirectory(SteadyHeat2DEB)
  add_subdirectory(ContinuationHeat1D)
  add_subdirectory(Heat1DPeriodic)
  add_subdirectory(Heat2DTriangles)
  add_subdirectory(Heat1DWithSource)
  add_subdirectory(MPNIQuad2D)
  add_subdirectory(TransientHeat1D)
  add_subdirectory(TransientHeat2D)
  add_subdirectory(HeatEigenvalues)
  add_subdirectory(SideSetLaplacian) # Not 100% sure this requires STK, but I think so
  IF(ALBANY_SEACAS)
    IF(ALBANY_PAMGEN)
      add_subdirectory(Heat3DPamgen)
    ENDIF()
    add_subdirectory(Heat2DMMCylWithSource)
    add_subdirectory(HeatQuadTri)
  #  add_subdirectory(TransientHeat2DTableSource)
    add_subdirectory(Ioss2D)
    add_subdirectory(Ioss3D)
    add_subdirectory(IossRestart)
    add_subdirectory(SteadyHeat2DInternalNeumann)
    add_subdirectory(SteadyHeat2DRobin)
    add_subdirectory(SteadyHeat2DSS)
  ENDIF(ALBANY_SEACAS)

ENDIF(ALBANY_STK)

add_subdirectory(Utils)

IF(ALBANY_SCOREC)
  add_subdirectory(Heat3DPUMI)
ENDIF()

IF(ALBANY_STK)
# DEMO PDES ###############
IF(ALBANY_DEMO_PDES)
  add_subdirectory(Helmholtz2D)
  add_subdirectory(LinComprNS)
  add_subdirectory(AdvDiff)
  add_subdirectory(ReactDiffSystem)
  add_subdirectory(ComprNS)
  add_subdirectory(ODE)
  add_subdirectory(TekoPrec)

  # These require exodus, Ioss, stk_io, to read meshes
  IF(ALBANY_SEACAS)
    add_subdirectory(CahnHillElast2D)
    add_subdirectory(NSPoiseuille2D)
    add_subdirectory(NSRayleighBernard2D)
    add_subdirectory(NSVortexShedding2D)
    add_subdirectory(PNP)
  ENDIF(ALBANY_SEACAS)

  if (ALBANY_ASCR)
    add_subdirectory(CoupledReactor)
    add_subdirectory(NSReactor)
  endif(ALBANY_ASCR)

ENDIF(ALBANY_DEMO_PDES)

ENDIF(ALBANY_STK)

# LCM ###############

IF(ALBANY_LCM)
  add_subdirectory(LCM)
ENDIF(ALBANY_LCM)

# GOAL ##############

IF(ALBANY_GOAL)
  add_subdirectory(GOAL)
ENDIF()

IF(ALBANY_STK)

# QCAD ##################

IF(ALBANY_QCAD AND ALBANY_SEACAS)
  add_subdirectory(QCAD)
  add_subdirectory(ThermoElectrostatics2D)
ENDIF()

# ATO ##################

IF(ALBANY_ATO)
  add_subdirectory(ATO)
ENDIF()

# LandIce ##################

IF(ALBANY_LANDICE AND ALBANY_SEACAS )
  add_subdirectory(LandIce)
ENDIF()

# LandIce ##################

IF(ALBANY_TSUNAMI AND ALBANY_SEACAS )
  add_subdirectory(Tsunami)
ENDIF()

# MOR   ##################

IF(ALBANY_MOR AND ALBANY_EPETRA)
  add_subdirectory(MOR)
ENDIF()

# ANISO #################

IF(ALBANY_ANISO)
  add_subdirectory(ANISO)
ENDIF(ALBANY_ANISO)

ENDIF(ALBANY_STK)

###################

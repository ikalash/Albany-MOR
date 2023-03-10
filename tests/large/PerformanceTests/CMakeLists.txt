##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//

# Get machineName for comparison with gold-standard timings in data.perf
execute_process(COMMAND hostname -s COMMAND tr -d '\n' OUTPUT_VARIABLE machineName)
message("-- Performance Test Machine Name = " ${machineName})

# Set test command that cmake will run, in subdirectories
set(performanceTestScript
    python ${CMAKE_CURRENT_SOURCE_DIR}/perfScript.py
     -machine ${machineName}
     -executable "${Albany_BINARY_DIR}/src")

set(performanceTestScript_2
    python ${CMAKE_CURRENT_SOURCE_DIR}/perfScript.py
     -machine ${machineName}_2
     -executable "${Albany_BINARY_DIR}/src")

# Heat Transfer Problems ###############
add_subdirectory(SteadyHeat2D)
IF(ALBANY_SEACAS)
  #add_subdirectory(SteadyHeat2DSS)
ENDIF()

# LCM ###############
IF(ALBANY_LCM)

  IF(ALBANY_SCOREC)
# Not sure if this runs for anyone...
  #  add_subdirectory(Necking3D)
  ENDIF()

  IF(ALBANY_HYDRIDE)
    #add_subdirectory(LaplaceBeltrami)
  ENDIF()

ENDIF(ALBANY_LCM)

# QCAD ##################

IF(ALBANY_QCAD)
  add_subdirectory(Poisson)
ENDIF()

# FELIX ##################

IF(ALBANY_FELIX )
  add_subdirectory(FELIX_FO_MMS)
ENDIF()

# MOR   ##################

IF(ALBANY_MOR)
  #add_subdirectory(MOR)
ENDIF()

###################

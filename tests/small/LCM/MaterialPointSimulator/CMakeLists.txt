##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//

#This test is not currently appropriate for the BGQ
IF(NOT ALBANY_PARALLEL_ONLY AND LCM_TEST_EXES AND ALBANY_BGL)

set(runtest.py ${CMAKE_CURRENT_SOURCE_DIR}/runtest.py)
set(MPS.cmake ${CMAKE_CURRENT_SOURCE_DIR}/MPS.cmake)

add_subdirectory(AnisotropicHyperelasticDamage)
add_subdirectory(AnisotropicDamage-Bifurcation)
add_subdirectory(Gurson)
add_subdirectory(Neohookean)
add_subdirectory(CrystalPlasticity_MPS)


ENDIF()

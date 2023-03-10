

# 1. Copy Input file from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input.xml COPYONLY)
# 2. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# 3. Create the test with this name and standard executable
if (ALBANY_EPETRA) 
add_test(${testName} ${Albany.exe})
endif()

if (ALBANY_IFPACK2)
# 1'. Copy Input file from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rythmos_be_nox_solver.xml
               ${CMAKE_CURRENT_BINARY_DIR}/rythmos_be_nox_solver.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rythmos_be_rythmos_solver.xml
               ${CMAKE_CURRENT_BINARY_DIR}/rythmos_be_rythmos_solver.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rythmos_rk4.xml
               ${CMAKE_CURRENT_BINARY_DIR}/rythmos_rk4.xml COPYONLY)
# 2'. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# 3'. Create the test with this name and standard executable
add_test(${testName}_Tpetra_Rythmos_BackwardEuler_RythmosSolver ${AlbanyT.exe} rythmos_be_rythmos_solver.xml)
add_test(${testName}_Tpetra_Rythmos_BackwardEuler_NOXSolver ${AlbanyT.exe} rythmos_be_nox_solver.xml)
add_test(${testName}_Tpetra_Rythmos_RK4 ${AlbanyT.exe} rythmos_rk4.xml)
if (ALBANY_TEMPUS)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tempus_be_nox_solver.xml
               ${CMAKE_CURRENT_BINARY_DIR}/tempus_be_nox_solver.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tempus_rk4.xml
               ${CMAKE_CURRENT_BINARY_DIR}/tempus_rk4.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tempus_rk4_no_piro.xml
               ${CMAKE_CURRENT_BINARY_DIR}/tempus_rk4_no_piro.xml COPYONLY)
add_test(${testName}_Tpetra_Tempus_BackwardEuler_NOXSolver ${AlbanyT.exe} tempus_be_nox_solver.xml)
add_test(${testName}_Tpetra_Tempus_RK4 ${AlbanyT.exe} tempus_rk4.xml)
add_test(${testName}_Tpetra_Tempus_NoPiro_RK4 ${AlbanyTempus.exe} tempus_rk4_no_piro.xml)
endif () 
endif ()

# 5. Repeat process for Dakota problems if "dakota.in" exists
if (ALBANY_DAKOTA)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dakota.in)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dakota.in
                 ${CMAKE_CURRENT_BINARY_DIR}/dakota.in COPYONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.xml
                 ${CMAKE_CURRENT_BINARY_DIR}/input.xml COPYONLY)
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dakota_nls.data)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dakota_nls.data
                   ${CMAKE_CURRENT_BINARY_DIR}/dakota_nls.data COPYONLY)
  endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dakota_nls.data)
  get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR}_Dakota NAME)
  add_test(${testName} ${AlbanyDakota.exe})
endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dakota.in)
endif()

# 6. Repeat process for Analysis problems if "inputAnalysis.xml" exists
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/inputAnalysis.xml)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputAnalysis.xml
                 ${CMAKE_CURRENT_BINARY_DIR}/inputAnalysis.xml COPYONLY)

  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dakota.in)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dakota.in
                   ${CMAKE_CURRENT_BINARY_DIR}/dakota.in COPYONLY)
  endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dakota.in)

  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Moocho.opt)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Moocho.opt
                   ${CMAKE_CURRENT_BINARY_DIR}/Moocho.opt COPYONLY)
  endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Moocho.opt)

  get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR}_Analysis NAME)
  add_test(${testName} ${AlbanyAnalysis.exe})
endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/inputAnalysis.xml)

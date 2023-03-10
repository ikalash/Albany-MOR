

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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/inputT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputT_RegressFail.xml
               ${CMAKE_CURRENT_BINARY_DIR}/inputT_RegressFail.xml COPYONLY)
# 2'. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# 3'. Create the test with this name and standard executable
add_test(${testName}_SERIAL_Tpetra ${SerialAlbanyT.exe} inputT.xml)
add_test(${testName}_Tpetra_RegressFail ${SerialAlbanyT.exe} inputT_RegressFail.xml)
set_tests_properties(${testName}_Tpetra_RegressFail PROPERTIES WILL_FAIL TRUE)
add_test(${testName}_Tpetra ${AlbanyT.exe} inputT.xml)
endif ()

if (ALBANY_MUELU_EXAMPLES)
# 1'. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR}_Tpetra_MueLu NAME)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputT_MueLu.xml
               ${CMAKE_CURRENT_BINARY_DIR}/inputT_MueLu.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputT_MueLu_RegressFail.xml
               ${CMAKE_CURRENT_BINARY_DIR}/inputT_MueLu_RegressFail.xml COPYONLY)
    # 3'. Create the test with this name and standard executable
    add_test(${testName}_SERIAL ${SerialAlbanyT.exe} inputT_MueLu.xml)
    add_test(${testName}_RegressFail ${SerialAlbanyT.exe} inputT_MueLu_RegressFail.xml)
    set_tests_properties(${testName}_RegressFail PROPERTIES WILL_FAIL TRUE)
    add_test(${testName} ${AlbanyT.exe} inputT_MueLu.xml)
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

# 7. 2D Steady Heat (Poisson) problem with trigonometric source and
# Dirichlet/Neumann BCs.
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_trigSource.xml
#               ${CMAKE_CURRENT_BINARY_DIR}/input_trigSource.xml COPYONLY)
#add_test(${testName}_TrigonometricSource ${Albany.exe} input_trigSource.xml)

# 8. 2D Steady Heat (Poisson) problem with trigonometric source and periodic
# BCs.
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_trigSource_periodicBCs.xml
#               ${CMAKE_CURRENT_BINARY_DIR}/input_trigSource_periodicBCs.xml COPYONLY)
#add_test(${testName}_TrigonometricSource_PeriodicBCs ${Albany.exe} input_trigSource_periodicBCs.xml)

# This test is not setup to run on the BGQ as it sits...
IF((NOT ALBANY_PARALLEL_ONLY) AND SEACAS_EPU AND SEACAS_EXODIFF)

# 1. Copy Input file from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/input.yaml COPYONLY)

# 2. Copy mesh files from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hole.exo
               ${CMAKE_CURRENT_BINARY_DIR}/hole.exo COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hole.exo.4.0
               ${CMAKE_CURRENT_BINARY_DIR}/hole.exo.4.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hole.exo.4.1
               ${CMAKE_CURRENT_BINARY_DIR}/hole.exo.4.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hole.exo.4.2
               ${CMAKE_CURRENT_BINARY_DIR}/hole.exo.4.2 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hole.exo.4.3
               ${CMAKE_CURRENT_BINARY_DIR}/hole.exo.4.3 COPYONLY)

# 3. Copy runtest.cmake from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtest.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/runtest.cmake COPYONLY)

# 4. Copy the reference solution over
if (ALBANY_DTK)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/reference_hole_dtk.exo
               ${CMAKE_CURRENT_BINARY_DIR}/reference_hole_dtk.exo COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtest_dtk.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/runtest_dtk.cmake COPYONLY)
else()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/reference_hole.exo
               ${CMAKE_CURRENT_BINARY_DIR}/reference_hole.exo COPYONLY)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/exodiff_commands
               ${CMAKE_CURRENT_BINARY_DIR}/exodiff_commands COPYONLY)

# 4. Build the test executable to read the ncdump file
#add_executable(TestNcdumpValues TestNcdumpValues.cpp)

# 5. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 6. Run the "runtest.cmake" file in the CWD

if (ALBANY_IFPACK2)
if (ALBANY_DTK)
add_test(NAME ${testName} COMMAND ${CMAKE_COMMAND} "-DTEST_PROG=${AlbanyT.exe};input.yaml"
  -DTEST_NAME=${testName} -DMPIMNP=${MPIMNP} -DSEACAS_EPU=${SEACAS_EPU} -DSEACAS_EXODIFF=${SEACAS_EXODIFF}
  -P runtest_dtk.cmake)
else()
add_test(NAME ${testName} COMMAND ${CMAKE_COMMAND} "-DTEST_PROG=${AlbanyT.exe};input.yaml"
  -DTEST_NAME=${testName} -DMPIMNP=${MPIMNP} -DSEACAS_EPU=${SEACAS_EPU} -DSEACAS_EXODIFF=${SEACAS_EXODIFF}
  -P runtest.cmake)
endif()
endif()

ENDIF()

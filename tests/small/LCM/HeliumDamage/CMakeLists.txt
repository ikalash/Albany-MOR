##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//

if (ALBANY_IFPACK2)
# Create a symlink to exodiff
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${SEACAS_EXODIFF} ${CMAKE_CURRENT_BINARY_DIR}/exodiff)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${SEACAS_ALGEBRA} ${CMAKE_CURRENT_BINARY_DIR}/algebra)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${AlbanyTPath} ${CMAKE_CURRENT_BINARY_DIR}/AlbanyT)

# input files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helium-damage.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/helium-damage.yaml COPYONLY)

# material files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helium-damage-materials.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/helium-damage-materials.yaml COPYONLY)

# exodiff files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helium-damage.exodiff
               ${CMAKE_CURRENT_BINARY_DIR}/helium-damage.exodiff COPYONLY)

# gold files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helium-damage.gold.e
               ${CMAKE_CURRENT_BINARY_DIR}/helium-damage.gold.e COPYONLY)

# cmake runtest file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtestT.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/runtestT.cmake COPYONLY)

# 2. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 3. Create the test with this name and standard executable
IF((NOT ALBANY_PARALLEL_ONLY) AND SEACAS_EXODIFF)
  SET(OUTFILE "helium-damage.e")
  SET(REF_FILE "helium-damage.gold.e")
  add_test(NAME ${testName}
           COMMAND ${CMAKE_COMMAND} "-DTEST_PROG=${SerialAlbanyT.exe}"
           -DTEST_NAME=helium-damage -DTEST_ARGS=helium-damage.yaml -DMPIMNP=1
           -DSEACAS_EXODIFF=${SEACAS_EXODIFF}
           -DREF_FILENAME=${REF_FILE} -DOUTPUT_FILENAME=${OUTFILE}
           -DDATA_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/runtestT.cmake)

ENDIF()
endif()

# Name the test with the directory name
get_filename_component(testDirName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(testName ${testDirName})

# Copy Input file from source to binary dir
IF (ALBANY_EPETRA)  
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fblock.xml ${CMAKE_CURRENT_BINARY_DIR}/fblock.xml COPYONLY)
ENDIF() 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fblockT.xml ${CMAKE_CURRENT_BINARY_DIR}/fblockT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fblock.gen ${CMAKE_CURRENT_BINARY_DIR}/fblock.gen COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fblock.gen.4.0 ${CMAKE_CURRENT_BINARY_DIR}/fblock.gen.4.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fblock.gen.4.1 ${CMAKE_CURRENT_BINARY_DIR}/fblock.gen.4.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fblock.gen.4.2 ${CMAKE_CURRENT_BINARY_DIR}/fblock.gen.4.2 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fblock.gen.4.3 ${CMAKE_CURRENT_BINARY_DIR}/fblock.gen.4.3 COPYONLY)

# Create the test with this name and standard executable
IF (SEACAS_EPU AND SEACAS_EXODIFF)
IF (ALBANY_EPETRA)  
add_test(NAME ATO:${testName} 
         COMMAND ${CMAKE_COMMAND} "-DTEST_PROG=${Albany.exe}"
         -DTEST_NAME=${testName} -DTEST_ARGS=fblock.xml -DMPIMNP=${MPIMNP} 
         -DSEACAS_EPU=${SEACAS_EPU} -DSEACAS_EXODIFF=${SEACAS_EXODIFF} 
         -DDATA_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/runtest.cmake)
ENDIF() 
add_test(NAME ATOT:${testName} 
         COMMAND ${CMAKE_COMMAND} "-DTEST_PROG=${AlbanyT.exe}"
         -DTEST_NAME=${testName} -DTEST_ARGS=fblockT.xml -DMPIMNP=${MPIMNP} 
         -DSEACAS_EPU=${SEACAS_EPU} -DSEACAS_EXODIFF=${SEACAS_EXODIFF} 
         -DDATA_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/runtestT.cmake)
ENDIF()

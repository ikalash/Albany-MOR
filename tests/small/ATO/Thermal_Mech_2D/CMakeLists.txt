# 
# 1. Copy input file from source to binary dir
IF (ALBANY_EPETRA) 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nodal.xml ${CMAKE_CURRENT_BINARY_DIR}/nodal.xml COPYONLY)
ENDIF() 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nodalT.xml ${CMAKE_CURRENT_BINARY_DIR}/nodalT.xml COPYONLY)
# 2. Copy mesh files from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mitchell.gen     ${CMAKE_CURRENT_BINARY_DIR}/mitchell.gen COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mitchell.gen.4.0 ${CMAKE_CURRENT_BINARY_DIR}/mitchell.gen.4.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mitchell.gen.4.1 ${CMAKE_CURRENT_BINARY_DIR}/mitchell.gen.4.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mitchell.gen.4.2 ${CMAKE_CURRENT_BINARY_DIR}/mitchell.gen.4.2 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mitchell.gen.4.3 ${CMAKE_CURRENT_BINARY_DIR}/mitchell.gen.4.3 COPYONLY)

IF (ALBANY_EPETRA) 
# 3. Copy runtest.cmake from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtest.cmake ${CMAKE_CURRENT_BINARY_DIR}/runtest.cmake COPYONLY)
ENDIF() 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtestT.cmake ${CMAKE_CURRENT_BINARY_DIR}/runtestT.cmake COPYONLY)

# 5. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 6. Run the "runtest.cmake" file in the CWD
IF (ALBANY_EPETRA) 
add_test(NAME ATO:${testName} COMMAND ${CMAKE_COMMAND} "-DTEST_PROG=${Albany.exe}"
  -DTEST_NAME=${testName} -P runtest.cmake)
ENDIF() 
add_test(NAME ATOT:${testName} COMMAND ${CMAKE_COMMAND} "-DTEST_PROG=${AlbanyT.exe}"
  -DTEST_NAME=${testName} -P runtestT.cmake)

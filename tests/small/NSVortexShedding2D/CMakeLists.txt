

# 1. Copy Input file from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vortex.exo
               ${CMAKE_CURRENT_BINARY_DIR}/vortex.exo COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vortex.exo.4.0
               ${CMAKE_CURRENT_BINARY_DIR}/vortex.exo.4.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vortex.exo.4.1
               ${CMAKE_CURRENT_BINARY_DIR}/vortex.exo.4.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vortex.exo.4.2
               ${CMAKE_CURRENT_BINARY_DIR}/vortex.exo.4.2 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vortex.exo.4.3
               ${CMAKE_CURRENT_BINARY_DIR}/vortex.exo.4.3 COPYONLY)
# 2. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# 3. Create the test with this name and standard executable
if (ALBANY_EPETRA) 
add_test(${testName} ${Albany.exe})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputBlocked.xml
               ${CMAKE_CURRENT_BINARY_DIR}/inputBlocked.xml COPYONLY)

if (ALBANY_EPETRA) 
add_test(${testName}_Blocked ${Albany.exe} inputBlocked.xml)
endif()

if (ALBANY_EPETRA)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputTransientRK.xml
               ${CMAKE_CURRENT_BINARY_DIR}/inputTransientRK.xml COPYONLY)

add_test(${testName}_TransIRK ${Albany.exe} inputTransientRK.xml)
endif(ALBANY_EPETRA)
if (ALBANY_IFPACK2)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputTransientRKT.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/inputTransientRKT.yaml COPYONLY)

add_test(${testName}_TransIRK_Tpetra ${AlbanyT.exe} inputTransientRKT.yaml)
endif(ALBANY_IFPACK2)

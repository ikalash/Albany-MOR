

# 1. Copy Input file from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/inputT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/materials.xml
               ${CMAKE_CURRENT_BINARY_DIR}/materials.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mesh.exo
               ${CMAKE_CURRENT_BINARY_DIR}/mesh.exo COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mesh.exo.4.0
               ${CMAKE_CURRENT_BINARY_DIR}/mesh.exo.4.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mesh.exo.4.1
               ${CMAKE_CURRENT_BINARY_DIR}/mesh.exo.4.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mesh.exo.4.2
               ${CMAKE_CURRENT_BINARY_DIR}/mesh.exo.4.2 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mesh.exo.4.3
               ${CMAKE_CURRENT_BINARY_DIR}/mesh.exo.4.3 COPYONLY)


# 2. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 3. Create the test with this name and standard executable

if (ALBANY_EPETRA) 
add_test(${testName} ${Albany.exe})
endif()

if (ALBANY_IFPACK2)
  add_test(${testName}_Tpetra ${AlbanyT.exe} inputT.xml)
endif ()

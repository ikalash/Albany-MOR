configure_file(${CMAKE_CURRENT_SOURCE_DIR}/circle.msh
               ${CMAKE_CURRENT_BINARY_DIR}/circle.msh COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xyz
               ${CMAKE_CURRENT_BINARY_DIR}/xyz COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/eles
               ${CMAKE_CURRENT_BINARY_DIR}/eles COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sh
               ${CMAKE_CURRENT_BINARY_DIR}/sh COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bf
               ${CMAKE_CURRENT_BINARY_DIR}/bf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/flwa
               ${CMAKE_CURRENT_BINARY_DIR}/flwa COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/beta
               ${CMAKE_CURRENT_BINARY_DIR}/beta COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_fo_dome_ascii.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_fo_dome_ascii.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_fo_dome_restart.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_fo_dome_restart.xml COPYONLY)

if (ALBANY_EPETRA)
add_test(LandIce_FO_Dome_Ascii ${SerialAlbany.exe} input_fo_dome_ascii.xml)

IF (ALBANY_SEACSAS)
  add_test(LandIce_FO_Dome_Restart ${SerialAlbany.exe} input_fo_dome_restart.xml)
  set_tests_properties(LandIce_FO_Dome_Restart  PROPERTIES DEPENDS LandIce_FO_Dome_Ascii)
ENDIF()

endif()



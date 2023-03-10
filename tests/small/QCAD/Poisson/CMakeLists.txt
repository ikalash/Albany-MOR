
# Copy files used by multiple tests
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/materials.xml
               ${CMAKE_CURRENT_BINARY_DIR}/materials.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/material_gaas.xml
               ${CMAKE_CURRENT_BINARY_DIR}/material_gaas.xml COPYONLY)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)

# Get testRoot as QCAD_<the directory name>
get_filename_component(subRoot ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(testRoot "QCAD_${subRoot}")

IF (ALBANY_EPETRA)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_stk2D.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_stk2D.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pmoscap_1D.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pmoscap_1D.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pmoscap.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pmoscap.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pmoscap_polygate.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pmoscap_polygate.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pmoscap_3D_itrap.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pmoscap_3D_itrap.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_gaascap_onesc.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_gaascap_onesc.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pndiode.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pndiode.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_2D.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_2D.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_2D_tri6.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_2D_tri6.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_2D_qcad.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_2D_qcad.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_3D.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_3D.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_3D_tet10.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_3D_tet10.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pointcharge_3D.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pointcharge_3D.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_cloudcharge_3D.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_cloudcharge_3D.xml COPYONLY)
ENDIF()

if (ALBANY_IFPACK2) 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_gaascap_onescT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_gaascap_onescT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pmoscap_3D_itrapT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pmoscap_3D_itrapT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_stk2DT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_stk2DT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pmoscapT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pmoscapT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pmoscap_polygateT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pmoscap_polygateT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_pndiodeT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_pndiodeT.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_2D_tri6T.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_2D_tri6T.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_3DT.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_3DT.xml COPYONLY)
endif() 

# Create tests with this name and serial executable
IF(NOT ALBANY_PARALLEL_ONLY)
  IF (ALBANY_EPETRA)
    add_test(${testRoot}_test2D_SERIAL ${SerialAlbany.exe} input_stk2D.xml)
    add_test(${testRoot}_pmoscap_1D_SERIAL ${SerialAlbany.exe} input_pmoscap_1D.xml)
  ENDIF()
  if (ALBANY_IFPACK2) 
    add_test(${testRoot}_test2D_SERIAL_Tpetra ${SerialAlbanyT.exe} input_stk2DT.xml)
  endif()
ENDIF()

# Create tests with this name and standard executable
IF (ALBANY_EPETRA AND ALBANY_IOPX)
add_test(${testRoot}_pmoscap ${Albany.exe} input_pmoscap.xml)
add_test(${testRoot}_pmoscap_polygate ${Albany.exe} input_pmoscap_polygate.xml)
add_test(${testRoot}_pmoscap_3D ${Albany.exe} input_pmoscap_3D_itrap.xml)
add_test(${testRoot}_gaascap_onesc ${Albany.exe} input_gaascap_onesc.xml)
add_test(${testRoot}_pndiode ${Albany.exe} input_pndiode.xml)
add_test(${testRoot}_mosdot_2D ${Albany.exe} input_mosdot_2D.xml)
add_test(${testRoot}_mosdot_2D_tri6 ${Albany.exe} input_mosdot_2D_tri6.xml)
add_test(${testRoot}_mosdot_2D_qcad ${Albany.exe} input_mosdot_2D_qcad.xml)
add_test(${testRoot}_mosdot_3D ${Albany.exe} input_mosdot_3D.xml)
add_test(${testRoot}_mosdot_3D_tet10 ${Albany.exe} input_mosdot_3D_tet10.xml)
ENDIF()

## TODO add_test(${testRoot}_pointcharge ${Albany.exe} input_pointcharge_3D.xml)
## TODO add_test(${testRoot}_cloudcharge ${Albany.exe} input_cloudcharge_3D.xml)

if (ALBANY_IFPACK2 AND ALBANY_IOPX) 
add_test(${testRoot}_gaascap_onesc_Tpetra ${AlbanyT.exe} input_gaascap_onescT.xml)
add_test(${testRoot}_pmoscap_3D_Tpetra ${AlbanyT.exe} input_pmoscap_3D_itrapT.xml)
add_test(${testRoot}_mosdot_3D_Tpetra ${AlbanyT.exe} input_mosdot_3DT.xml)
add_test(${testRoot}_pmoscap_Tpetra ${AlbanyT.exe} input_pmoscapT.xml)
add_test(${testRoot}_pmoscap_polygate_Tpetra ${AlbanyT.exe} input_pmoscap_polygateT.xml)
add_test(${testRoot}_pndiode_Tpetra ${AlbanyT.exe} input_pndiodeT.xml)
add_test(${testRoot}_mosdot_2D_tri6_Tpetra ${AlbanyT.exe} input_mosdot_2D_tri6T.xml)
endif()

if (ALBANY_MUELU_EXAMPLES AND ALBANY_IOPX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_mosdot_3D_tet10T.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_mosdot_3D_tet10T.xml COPYONLY)
    add_test(${testRoot}_mosdot_3D_tet10_Tpetra ${AlbanyT.exe} input_mosdot_3D_tet10T.xml)
endif()





 

# 1. Copy Input file from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_spectralT_rythmos.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_spectralT_rythmos.xml COPYONLY)
if (ALBANY_TEMPUS)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_spectralT_tempus.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_spectralT_tempus.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_spectralT_tempus_no_piro.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_spectralT_tempus_no_piro.xml COPYONLY)
endif () 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_with_topography.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_with_topography.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/plot_xz_matlab.m
               ${CMAKE_CURRENT_BINARY_DIR}/plot_xz_matlab.m COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_spectral_ho_RK4_T.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_spectral_ho_RK4_T.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input_spectral_ho_BE_T.xml
               ${CMAKE_CURRENT_BINARY_DIR}/input_spectral_ho_BE_T.xml COPYONLY)
# 2. Name the test with the directory name
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# 3. Create the test with this name and standard executable

add_test(Aeras_${testName}_Spectral_np2_RungeKutta4_Rythmos ${AlbanyT.exe} input_spectralT_rythmos.xml) 
if (ALBANY_TEMPUS)
add_test(Aeras_${testName}_Spectral_np2_RungeKutta4_Tempus ${AlbanyT.exe} input_spectralT_tempus.xml) 
add_test(Aeras_${testName}_Spectral_np2_RungeKutta4_Tempus_No_Piro ${AlbanyTempus.exe} input_spectralT_tempus_no_piro.xml) 
endif () 

# 5. Repeat process for Dakota problems if "dakota.in" exists
if (ALBANY_DAKOTA AND ALBANY_EPETRA)
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
  add_test(Aeras_${testName} ${AlbanyDakota.exe})
endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dakota.in)
endif()

# 6. Repeat process for Analysis problems if "inputAnalysis.xml" exists

if (ALBANY_EPETRA) 
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
  add_test(Aeras_${testName} ${AlbanyAnalysis.exe})
endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/inputAnalysis.xml)
endif()


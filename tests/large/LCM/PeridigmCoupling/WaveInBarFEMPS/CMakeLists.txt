
# Create a symbolic link for exodiff and epu
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SEACAS_EXODIFF} ${CMAKE_CURRENT_BINARY_DIR}/exodiff)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SEACAS_EPU} ${CMAKE_CURRENT_BINARY_DIR}/epu)

# Copy Input files from source to binary dir

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBarMaterials.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBarMaterials.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.g
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.g COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.g.4.0
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.g.4.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.g.4.1
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.g.4.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.g.4.2
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.g.4.2 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.g.4.3
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.g.4.3 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.gold.e
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.gold.e COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WaveInBar.exodiff
               ${CMAKE_CURRENT_BINARY_DIR}/WaveInBar.exodiff COPYONLY)

# Add the tests
if(ALBANY_IFPACK2)
add_test(WaveInBarFEMPS_np1_Epetra python ../run_exodiff_test.py "${SerialAlbany.exe}" WaveInBar.yaml)
add_test(WaveInBarFEMPS_np4_Epetra python ../run_exodiff_test.py "${Albany.exe}" WaveInBar.yaml)
endif()

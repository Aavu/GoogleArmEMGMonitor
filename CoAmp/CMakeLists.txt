add_library(
        ${COAMP_LIB}
        Src/CoAmp.cpp
        )

target_include_directories(${COAMP_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include /usr/local/include/)
target_include_directories(${COAMP_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/3rdparty/${CAN_LIB}/Include ${CMAKE_SOURCE_DIR}/${KEYLOGGER_LIB}/Include)
target_link_libraries(${COAMP_LIB} PUBLIC ${CAN_LIB} ${LOGGER_LIB} ${RINGBUFFER_LIB} ${KEYLOGGER_LIB})
target_precompile_headers(${COAMP_LIB} REUSE_FROM ${LOGGER_LIB})

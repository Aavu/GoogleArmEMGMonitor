add_library(
        ${SERVER_LIB}
        Src/Server.cpp
)

target_include_directories(${SERVER_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include /usr/local/include/)
target_include_directories(${SERVER_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/${TCP_LIB}/Include ${CMAKE_SOURCE_DIR}/${KEYLOGGER_LIB}/Include)
target_link_libraries(${SERVER_LIB} PUBLIC ${TCP_LIB} ${COAMP_LIB} ${LOGGER_LIB} ${KEYLOGGER_LIB} -lpthread)
target_precompile_headers(${SERVER_LIB} REUSE_FROM ${LOGGER_LIB})
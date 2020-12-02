add_library(
        ${TCP_LIB}
        Src/TCPServer.cpp
)

target_include_directories(${TCP_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include /usr/local/include/)
target_link_libraries(${TCP_LIB} PRIVATE ${LOGGER_LIB})
target_precompile_headers(${TCP_LIB} REUSE_FROM ${LOGGER_LIB})
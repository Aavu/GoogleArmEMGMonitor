add_library(
        ${KEYLOGGER_LIB}
        Src/KeyLogger.cpp
)

target_include_directories(${KEYLOGGER_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(${KEYLOGGER_LIB} PRIVATE ${LOGGER_LIB} -lstdc++fs)
target_precompile_headers(${KEYLOGGER_LIB} REUSE_FROM ${LOGGER_LIB})
add_library(
        ${LOGGER_LIB}
        Src/Logger.cpp
)

target_include_directories(${LOGGER_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include ${CMAKE_SOURCE_DIR}/3rdparty/spdlog/Include)
target_precompile_headers(${LOGGER_LIB}
        PUBLIC
            ${CMAKE_SOURCE_DIR}/Include/pch.h
        PRIVATE
            ${CMAKE_SOURCE_DIR}/Logger/Include/spdlog_pch.h
        )
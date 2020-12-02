add_library(
        ${RINGBUFFER_LIB}
        Src/RingBuffer.cpp
        Src/RingBuffer-impl.cpp
)

target_include_directories(${RINGBUFFER_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(${RINGBUFFER_LIB} ${LOGGER_LIB})
target_precompile_headers(${RINGBUFFER_LIB} REUSE_FROM ${LOGGER_LIB})
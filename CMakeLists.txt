cmake_minimum_required(VERSION 3.16)
project(EMGDataServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror -Wno-psabi")

set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)

set(CAN_LIB libcan)
set(COAMP_LIB CoAmp)
set(RINGBUFFER_LIB RingBuffer)
set(LOGGER_LIB Logger)
set(TCP_LIB TCPServer)
set(SERVER_LIB Server)
set(KEYLOGGER_LIB KeyLogger)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})

include_directories(
        ${CMAKE_SOURCE_DIR}/Include
        ${CMAKE_SOURCE_DIR}/${LOGGER_LIB}/Include
        ${CMAKE_SOURCE_DIR}/${RINGBUFFER_LIB}/Include
)

add_subdirectory(${LOGGER_LIB})
add_subdirectory(3rdparty/${CAN_LIB})

add_subdirectory(${RINGBUFFER_LIB})
add_subdirectory(${KEYLOGGER_LIB})
add_subdirectory(${COAMP_LIB})
add_subdirectory(${TCP_LIB})
add_subdirectory(${SERVER_LIB})

add_executable(${PROJECT_NAME} main.cpp)
target_precompile_headers(${PROJECT_NAME} REUSE_FROM ${LOGGER_LIB})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LOGGER_LIB} ${SERVER_LIB} ${COAMP_LIB} ${KEYLOGGER_LIB})
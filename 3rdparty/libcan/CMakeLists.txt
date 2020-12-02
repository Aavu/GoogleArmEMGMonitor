add_library(
        ${CAN_LIB}
        Src/CANAdapter.cpp
        Src/CANFrameParser.cpp
        Src/SocketCAN.cpp
)

target_include_directories(${CAN_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include /usr/local/include/)
target_include_directories(${CAN_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/3rdparty/libcan/Include)
target_link_libraries(${CAN_LIB} PUBLIC ${LOGGER_LIB})
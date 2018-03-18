# Project definition
cmake_minimum_required(VERSION 3.1)
project(KontrollerSock VERSION 0.0.0 LANGUAGES CXX)
set(SERVER_TARGET "KontrollerServer")
set(CLIENT_TARGET "KontrollerClient")

# Directories
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SERVER_SRC_DIR "${SRC_DIR}/Server")
set(CLIENT_SRC_DIR "${SRC_DIR}/Client")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# Source files
set(SERVER_SOURCES)
list(APPEND SERVER_SOURCES
   "${INC_DIR}/KontrollerSock/Client.h"
   "${INC_DIR}/KontrollerSock/Handles.h"
   "${INC_DIR}/KontrollerSock/Packet.h"
   "${INC_DIR}/KontrollerSock/Sock.h"
   "${SERVER_SRC_DIR}/Server.cpp"
)
set(CLIENT_SOURCES)
list(APPEND CLIENT_SOURCES
   "${INC_DIR}/KontrollerSock/Client.h"
   "${INC_DIR}/KontrollerSock/Handles.h"
   "${INC_DIR}/KontrollerSock/Packet.h"
   "${INC_DIR}/KontrollerSock/Sock.h"
   "${CLIENT_SRC_DIR}/Client.cpp"
)

# Target definitions
add_library(${SERVER_TARGET} ${SERVER_SOURCES})
target_include_directories(${SERVER_TARGET}
   PUBLIC "${INC_DIR}"
   PRIVATE "${SRC_DIR}"
)
add_library(${CLIENT_TARGET} ${CLIENT_SOURCES})
target_include_directories(${CLIENT_TARGET}
   PUBLIC "${INC_DIR}"
   PRIVATE "${SRC_DIR}"
)
set_target_properties(${SERVER_TARGET} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED ON
)
set_target_properties(${CLIENT_TARGET} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED ON
)

# Libraries
set(KONTROLLER_DIR "${LIB_DIR}/Kontroller")
add_subdirectory("${KONTROLLER_DIR}")
target_include_directories(${SERVER_TARGET} PUBLIC "${KONTROLLER_DIR}/include")
target_include_directories(${CLIENT_TARGET} PUBLIC "${KONTROLLER_DIR}/include")
target_link_libraries(${SERVER_TARGET} "Kontroller")
target_link_libraries(${CLIENT_TARGET} "Kontroller")

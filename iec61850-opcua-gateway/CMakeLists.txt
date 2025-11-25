cmake_minimum_required(VERSION 3.20)

project(iec61850-opcua-gateway 
    VERSION 1.0.0
    DESCRIPTION "IEC61850 Client to OPC UA Server Gateway"
    LANGUAGES CXX C
)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Compiler Options
include(CompilerOptions)

# Add local dependency path
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../deps/install")

# Find Dependencies
find_package(Threads REQUIRED)

# libIEC61850 (Try to find installed, or use FetchContent/ExternalProject if needed later)
# For now, we assume it's installed or we'll add a Find module
find_package(LibIEC61850 REQUIRED)

# open62541
find_package(open62541 REQUIRED)

# spdlog
find_package(spdlog REQUIRED)

# yaml-cpp
find_package(yaml-cpp REQUIRED)

# nlohmann_json
# JSON (Single Header)
# find_package(nlohmann_json REQUIRED) - Using local header instead

# pugixml
add_library(pugixml STATIC "${CMAKE_SOURCE_DIR}/deps/pugixml/src/pugixml.cpp")
target_include_directories(pugixml PUBLIC "${CMAKE_SOURCE_DIR}/deps/pugixml/src")

# Asio (Header only or libr# asio (Header only)
# find_package(asio REQUIRED)
include_directories("${CMAKE_SOURCE_DIR}/../deps/asio/asio/include")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Source files
add_executable(iec61850-opcua-gateway
    src/main.cpp
    src/core/logger.cpp
    src/core/application.cpp
    src/core/config_parser.cpp
    src/core/service_manager.cpp
    src/iec61850/mms/mms_connection.cpp
    src/iec61850/scl/scl_generator.cpp
    src/iec61850/scl/scd_generator.cpp
    src/iec61850/scl/scl_parser.cpp
    src/iec61850/goose/goose_receiver.cpp
    src/iec61850/goose/goose_subscriber_manager.cpp
    src/opcua/opcua_server.cpp
    src/opcua/namespace/namespace_builder.cpp
    src/opcua/data_binder.cpp
    src/opcua/subscription/subscription_manager.cpp
    src/api/rest_api.cpp
    src/api/topology_parser.cpp
)

# Link libraries
target_link_libraries(iec61850-opcua-gateway PRIVATE
    Threads::Threads
    LibIEC61850::LibIEC61850
    open62541::open62541
    spdlog::spdlog
    yaml-cpp
    # nlohmann_json::nlohmann_json - Header only
    pugixml
    # asio::asio - Header only
)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY config/ DESTINATION config)

# Testing
enable_testing()# Tests
# add_subdirectory(tests)

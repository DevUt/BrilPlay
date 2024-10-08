cmake_minimum_required(VERSION 3.15)

project(BrilPlay)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

find_package(nlohmann_json REQUIRED)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
)
add_executable(${PROJECT_NAME} 
	src/main.cpp 
	src/jsonToOur.hpp
	src/jsonToOur.cpp
	src/program.cpp
	src/core.hpp)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

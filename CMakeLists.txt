cmake_minimum_required(VERSION 3.15)

project(BrilPlay)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
)
add_executable(${PROJECT_NAME} 
	src/main.cpp 
)

# add_executable(StrongVar, 
#         src/strongvar.cpp
# )


add_subdirectory(src/core)

# Link against the core library
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core
	nlohmann_json::nlohmann_json
)


# Create the core library
add_library(core
    function.cpp
    program.cpp
)

# Setup include directories for the library
target_include_directories(core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..  # This allows #include <core/core.hpp>
)

set_target_properties(core PROPERTIES
    PUBLIC_HEADER "core.hpp"
)

target_link_libraries(core
    PUBLIC  # or PRIVATE depending on whether core's headers expose json types
        nlohmann_json::nlohmann_json
        Boost::graph
)
install(TARGETS core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/core
)


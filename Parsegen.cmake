# Find necessary packages for parsegen
find_package(ZLIB REQUIRED)

# Build the parsegen utility
add_executable(parsegen parsegen.cpp)

# Set include directories for the parsegen target
target_include_directories(parsegen PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_directories(parsegen PRIVATE /usr/lib)

# Parsegen should not be compiled with -m32 or any of the other 32-bit specific flags
set_target_properties(parsegen PROPERTIES
    COMPILE_FLAGS ""
    LINK_FLAGS ""
)
target_link_libraries(parsegen PRIVATE ${ZLIB_LIBRARIES})
set(PARSEGEN_BINARY ${CMAKE_BINARY_DIR}/parsegen)

# Custom command to check the format of parsegen
add_custom_command(
    TARGET parsegen
    POST_BUILD
    COMMAND echo "=== Checking parsegen binary format ==="
    COMMAND file ${PARSEGEN_BINARY}
    COMMAND echo "=== Completed checking ==="
    COMMENT "Checking binary format of parsegen"
)

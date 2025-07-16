cmake_minimum_required(VERSION 3.16)
project(BrownianMotionSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML REQUIRED sfml-all>=2.5)

# Create executable
add_executable(brownian_simulation
    src/main.cpp
    src/simulation.cpp
    src/matrix_operations.cpp
    src/fps_counter.cpp
    src/obstacle_system.cpp
)

# Link SFML
target_link_libraries(brownian_simulation ${SFML_LIBRARIES})
target_include_directories(brownian_simulation PRIVATE ${SFML_INCLUDE_DIRS})
target_compile_options(brownian_simulation PRIVATE ${SFML_CFLAGS_OTHER})

# Set optimization flags for the demo (we want to see the difference)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(brownian_simulation PRIVATE -O0 -g)
else()
    target_compile_options(brownian_simulation PRIVATE -O2)
endif()

# Enable profiling symbols
target_compile_options(brownian_simulation PRIVATE -g)

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    find_library(SFML_SYSTEM sfml-system)
    find_library(SFML_WINDOW sfml-window)
    find_library(SFML_GRAPHICS sfml-graphics)
    
    if(SFML_SYSTEM AND SFML_WINDOW AND SFML_GRAPHICS)
        target_link_libraries(brownian_simulation ${SFML_SYSTEM} ${SFML_WINDOW} ${SFML_GRAPHICS})
    endif()
endif() 
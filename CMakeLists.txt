cmake_minimum_required(VERSION 3.10)
project(Simulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 3.0 COMPONENTS system window graphics REQUIRED)

# Find Box2D
find_package(box2d REQUIRED)

# Add executable
add_executable(simulation src/main.cpp)

# Link libraries
target_link_libraries(simulation 
    SFML::System 
    SFML::Window 
    SFML::Graphics
    box2d::box2d
)


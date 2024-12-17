cmake_minimum_required(VERSION 3.16)
project(SDL_Pong)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find and link SDL3 libraries
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

# Specify the source files
set(SOURCES
    game.cpp
    sdl_pong.cpp
)

# Specify the header files
set(HEADERS
    sdl_pong.hpp
)

# Add the executable
add_executable(sdl_pong ${SOURCES} ${HEADERS})

# Link the libraries
if(SDL3_FOUND AND SDL3_ttf_FOUND )
    target_link_libraries(sdl_pong
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
    )
else()
    message(FATAL_ERROR "SDL3 or its components not found")
endif()


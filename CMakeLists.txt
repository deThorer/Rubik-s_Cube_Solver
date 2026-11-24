cmake_minimum_required(VERSION 3.16)
project(rubiks_cube_solver CXX)

# C++17 for std::filesystem (used by main.cpp to create the database folder).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    # Search is CPU-bound; an unoptimised build is ~10x slower.
    set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(Model)

# The three cube models (Model/RubiksCube*Array.cpp, Model/RubiksCubeBitboard.cpp)
# define their classes inline and are #included where used, so they are not
# listed here as separate translation units.
add_executable(rubiks_cube_solver
        main.cpp
        Model/RubiksCube.cpp
        PatternDatabases/NibbleArray.cpp
        PatternDatabases/PatternDatabase.cpp
        PatternDatabases/CornerPatternDatabase.cpp
        PatternDatabases/CornerDBMaker.cpp
        PatternDatabases/math.cpp)

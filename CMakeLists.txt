cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    main.cpp
    piece.cpp
    piece_types.cpp
    board.cpp
    game.cpp
    engine.cpp
    ui.cpp
)

# Add header files
set(HEADERS
    main.h
    piece.h
    piece_types.h
    board.h
    game.h
    engine.h
    ui.h
)

# Create executable
add_executable(chess_engine ${SOURCES} ${HEADERS})

# Add any compiler flags if needed
if(MSVC)
    target_compile_options(chess_engine PRIVATE /W4)
else()
    target_compile_options(chess_engine PRIVATE -Wall -Wextra -pedantic)
endif()
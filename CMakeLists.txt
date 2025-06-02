cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    piece.cpp
    piece_types.cpp
    board.cpp
    game.cpp
    engine.cpp
    ui.cpp
    zobrist.cpp
    transposition.cpp
    perft.cpp
    tactical_tests.cpp
    uci.cpp
)

set(HEADERS
    main.h
    piece.h
    piece_types.h
    board.h
    game.h
    engine.h
    ui.h
    zobrist.h
    transposition.h
    board_state.h
    perft.h
    tactical_tests.h
    uci.h
)

# Create executable
add_executable(chess_engine ${SOURCES} ${HEADERS})

# Add any compiler flags if needed
if(MSVC)
    target_compile_options(chess_engine PRIVATE /W4)
else()
    target_compile_options(chess_engine PRIVATE -Wall -Wextra -pedantic)
endif()
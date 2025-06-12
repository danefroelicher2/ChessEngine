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
    common.h
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

# Create main chess engine executable
add_executable(chess_engine ${SOURCES} ${HEADERS})

# Create simple test server
add_executable(simple_server simple_server.cpp)
if(WIN32)
    target_link_libraries(simple_server ws2_32)
endif()

# Create engine bridge server (has crash issues)
add_executable(engine_bridge 
    engine_bridge.cpp
    piece.cpp
    piece_types.cpp
    board.cpp
    game.cpp
    engine.cpp
    zobrist.cpp
    transposition.cpp
)
if(WIN32)
    target_link_libraries(engine_bridge ws2_32)
endif()

# Create progressive engine (working smart moves)
add_executable(progressive_engine progressive_engine.cpp)
if(WIN32)
    target_link_libraries(progressive_engine ws2_32)
endif()

# Add any compiler flags if needed
if(MSVC)
    target_compile_options(chess_engine PRIVATE /W4)
    target_compile_options(simple_server PRIVATE /W4)
    target_compile_options(engine_bridge PRIVATE /W4)
    target_compile_options(progressive_engine PRIVATE /W4)
else()
    target_compile_options(chess_engine PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(simple_server PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(engine_bridge PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(progressive_engine PRIVATE -Wall -Wextra -pedantic)
endif()
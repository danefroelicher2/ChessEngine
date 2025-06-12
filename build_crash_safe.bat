#!/bin/bash
# build_crash_safe.bat - Windows batch file to build the crash-safe engine

echo "🛡️ Building Crash-Safe Chess Engine..."
echo "======================================"

# Clean old builds
echo "Cleaning old builds..."
if exist crash_safe_server.exe del crash_safe_server.exe
if exist chess_engine.exe del chess_engine.exe

# Build the main chess engine (for testing)
echo ""
echo "📦 Building main chess engine..."
g++ -std=c++17 -O2 ^
    main.cpp piece.cpp piece_types.cpp board.cpp game.cpp engine.cpp ^
    ui.cpp zobrist.cpp transposition.cpp perft.cpp tactical_tests.cpp uci.cpp ^
    -o chess_engine.exe -lws2_32

if exist chess_engine.exe (
    echo "✅ Main engine built successfully!"
) else (
    echo "❌ Main engine build failed!"
    pause
    exit /b 1
)

# Build the crash-safe server
echo ""
echo "🛡️ Building crash-safe server..."
g++ -std=c++17 -O2 ^
    crash_safe_server.cpp piece.cpp piece_types.cpp board.cpp game.cpp engine.cpp ^
    zobrist.cpp transposition.cpp ^
    -o crash_safe_server.exe -lws2_32

if exist crash_safe_server.exe (
    echo "✅ Crash-safe server built successfully!"
    echo ""
    echo "🎯 READY TO TEST!"
    echo "================="
    echo "1. Run: crash_safe_server.exe"
    echo "2. Open: chess_engine_tester.html in browser"
    echo "3. Click: 'Test Engine Connection'"
    echo "4. Try: 'Get Engine Move'"
    echo ""
    echo "The server should NOT crash and return legal moves!"
) else (
    echo "❌ Crash-safe server build failed!"
    echo ""
    echo "Common issues:"
    echo "- Missing source files"
    echo "- Compilation errors in engine.cpp"
    echo "- Missing Windows socket library"
    pause
    exit /b 1
)

echo ""
echo "🚀 Build complete! Files created:"
echo "- chess_engine.exe (main engine)"
echo "- crash_safe_server.exe (safe server)"
echo ""
echo "Next steps:"
echo "1. Test the crash-safe server first"
echo "2. Once stable, we can work on enabling more features"
echo "3. Gradually increase search depth"
echo ""
pause
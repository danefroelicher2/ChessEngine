#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "game.h"
#include "engine.h"

#pragma comment(lib, "ws2_32.lib")

class RealChessServer {
private:
    Game game;
    Engine engine;
    
public:
    RealChessServer() : engine(game, 3) {
        std::cout << "Real Chess Engine Server initialized!" << std::endl;
    }
    
    std::string httpResponse(const std::string& content) {
        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: application/json\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET\r\n";
        response << "Content-Length: " << content.length() << "\r\n";
        response << "\r\n";
        response << content;
        return response.str();
    }
    
    std::string urlDecode(const std::string& str) {
        std::string decoded = str;
        size_t pos = 0;
        while ((pos = decoded.find("%20", pos)) != std::string::npos) {
            decoded.replace(pos, 3, " ");
            pos += 1;
        }
        while ((pos = decoded.find("%2F", pos)) != std::string::npos) {
            decoded.replace(pos, 3, "/");
            pos += 1;
        }
        return decoded;
    }
    
    std::string handleRequest(const std::string& request) {
        if (request.find("GET /status") == 0) {
            return httpResponse("{\"status\":\"Real Chess Engine Ready\"}");
        }
        
        if (request.find("GET /move") == 0) {
            try {
                // Extract FEN and depth from request
                std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
                int depth = 3;
                
                size_t fenPos = request.find("fen=");
                if (fenPos != std::string::npos) {
                    size_t fenStart = fenPos + 4;
                    size_t fenEnd = request.find("&", fenStart);
                    if (fenEnd == std::string::npos) {
                        fenEnd = request.find(" ", fenStart);
                    }
                    if (fenEnd != std::string::npos) {
                        fen = urlDecode(request.substr(fenStart, fenEnd - fenStart));
                    }
                }
                
                size_t depthPos = request.find("depth=");
                if (depthPos != std::string::npos) {
                    try {
                        depth = std::stoi(request.substr(depthPos + 6, 1));
                        depth = std::max(1, std::min(depth, 6));
                    } catch (...) {
                        depth = 3;
                    }
                }
                
                std::cout << "Analyzing position (depth " << depth << ")..." << std::endl;
                
                // Set up the position
                game.newGameFromFEN(fen);
                engine.setDepth(depth);
                
                // Get the best move from YOUR engine
                Move bestMove = engine.getBestMove();
                int eval = engine.evaluatePosition(game.getBoard());
                
                // Convert move to string format
                std::string moveStr = bestMove.from.toString() + bestMove.to.toString();
                
                // Add promotion if needed
                if (bestMove.promotion != PieceType::NONE) {
                    switch (bestMove.promotion) {
                        case PieceType::QUEEN: moveStr += "q"; break;
                        case PieceType::ROOK: moveStr += "r"; break;
                        case PieceType::BISHOP: moveStr += "b"; break;
                        case PieceType::KNIGHT: moveStr += "n"; break;
                        default: break;
                    }
                }
                
                std::string evalStr = std::to_string(eval / 100.0);
                
                std::cout << "Engine move: " << moveStr << " (eval: " << evalStr << ")" << std::endl;
                
                std::string response = "{\"move\":\"" + moveStr + "\",\"eval\":\"" + evalStr + "\"}";
                return httpResponse(response);
                
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
                return httpResponse("{\"move\":\"e2e4\",\"eval\":\"0.0\",\"error\":\"Engine error\"}");
            }
        }
        
        return httpResponse("{\"error\":\"Unknown request\"}");
    }
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed" << std::endl;
        return 1;
    }
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Bind failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cout << "Listen failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "🏆 REAL CHESS ENGINE SERVER RUNNING! 🏆" << std::endl;
    std::cout << "Server: http://localhost:8080" << std::endl;
    std::cout << "Using YOUR actual chess engine with AI!" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    RealChessServer server;
    
    while (true) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) continue;
        
        char buffer[4096] = {0};
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);
        
        if (bytesReceived > 0) {
            std::string request(buffer);
            std::string response = server.handleRequest(request);
            send(clientSocket, response.c_str(), (int)response.length(), 0);
        }
        
        closesocket(clientSocket);
    }
    
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
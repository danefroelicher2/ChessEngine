// Simple HTTP server to bridge HTML GUI and Chess Engine
// Save as: engine_server.cpp

#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <map>

// For Windows socket programming
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

class ChessEngineServer {
private:
    int port = 8080;
    bool running = false;
    
    // Simple chess engine simulation (replace with your actual engine)
    std::string getCurrentPosition() {
        return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    
    std::string getBestMove(const std::string& fen, int depth) {
        // TODO: Replace this with actual engine call
        std::cout << "Engine request: FEN=" << fen << ", depth=" << depth << std::endl;
        
        // Simulate engine thinking
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Return a simple move (replace with actual engine logic)
        return "e2e4";
    }
    
    std::string evaluatePosition(const std::string& fen) {
        // TODO: Replace with actual evaluation
        return "0.5";
    }
    
    std::string httpResponse(const std::string& content, const std::string& contentType = "application/json") {
        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << content.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";  // Allow CORS
        response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "\r\n";
        response << content;
        return response.str();
    }
    
    std::string handleRequest(const std::string& request) {
        std::cout << "Received request: " << request.substr(0, 100) << "..." << std::endl;
        
        // Parse HTTP request
        if (request.find("GET /") == 0) {
            if (request.find("GET /status") != std::string::npos) {
                return httpResponse("{\"status\":\"ready\",\"engine\":\"Chess Engine v1.0\"}");
            }
            else if (request.find("GET /position") != std::string::npos) {
                std::string fen = getCurrentPosition();
                return httpResponse("{\"fen\":\"" + fen + "\"}");
            }
            else if (request.find("GET /move") != std::string::npos) {
                // Parse parameters from URL
                std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; // default
                int depth = 3; // default
                
                // Extract FEN and depth from query string if present
                size_t fenPos = request.find("fen=");
                if (fenPos != std::string::npos) {
                    size_t fenEnd = request.find("&", fenPos);
                    if (fenEnd == std::string::npos) fenEnd = request.find(" ", fenPos);
                    fen = request.substr(fenPos + 4, fenEnd - fenPos - 4);
                    // URL decode fen (basic)
                    std::replace(fen.begin(), fen.end(), '%', ' ');
                }
                
                size_t depthPos = request.find("depth=");
                if (depthPos != std::string::npos) {
                    depth = std::stoi(request.substr(depthPos + 6, 1));
                }
                
                std::string move = getBestMove(fen, depth);
                return httpResponse("{\"move\":\"" + move + "\",\"eval\":\"" + evaluatePosition(fen) + "\"}");
            }
        }
        else if (request.find("OPTIONS") == 0) {
            // Handle CORS preflight
            return httpResponse("", "text/plain");
        }
        
        // Default response
        return httpResponse("{\"error\":\"Unknown request\"}");
    }
    
public:
    void start() {
        #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return;
        }
        #endif
        
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        // Allow socket reuse
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed on port " << port << std::endl;
            return;
        }
        
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Listen failed" << std::endl;
            return;
        }
        
        running = true;
        std::cout << "Chess Engine Server running on http://localhost:" << port << std::endl;
        std::cout << "Open chess_engine_tester.html in your browser to connect" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        while (running) {
            sockaddr_in client_addr;
            int addrlen = sizeof(client_addr);
            int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
            
            if (client_socket < 0) {
                continue;
            }
            
            // Read request
            char buffer[4096] = {0};
            recv(client_socket, buffer, 4096, 0);
            
            // Handle request
            std::string response = handleRequest(std::string(buffer));
            send(client_socket, response.c_str(), response.length(), 0);
            
            #ifdef _WIN32
            closesocket(client_socket);
            #else
            close(client_socket);
            #endif
        }
        
        #ifdef _WIN32
        closesocket(server_fd);
        WSACleanup();
        #else
        close(server_fd);
        #endif
    }
    
    void stop() {
        running = false;
    }
};

int main() {
    std::cout << "=== Chess Engine HTTP Server ===" << std::endl;
    
    ChessEngineServer server;
    
    try {
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    
    return 0;
}
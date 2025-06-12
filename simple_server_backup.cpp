#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

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

std::string handleRequest(const std::string& request) {
    if (request.find("GET /status") == 0) {
        return httpResponse("{\"status\":\"Chess Engine Ready\"}");
    }
    
    if (request.find("GET /move") == 0) {
        // Return different moves to simulate engine thinking
        static int moveCount = 0;
        std::string moves[] = {"e2e4", "d2d4", "g1f3", "b1c3", "f1c4", "e2e3", "d2d3"};
        std::string move = moves[moveCount % 7];
        moveCount++;
        
        std::cout << "Engine move: " << move << std::endl;
        
        return httpResponse("{\"move\":\"" + move + "\",\"eval\":\"0.2\"}");
    }
    
    return httpResponse("{\"error\":\"Unknown request\"}");
}

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
    
    std::cout << "🎯 CHESS ENGINE SERVER RUNNING!" << std::endl;
    std::cout << "Server: http://localhost:8080" << std::endl;
    std::cout << "Ready to play chess!" << std::endl;
    
    while (true) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) continue;
        
        char buffer[4096] = {0};
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);
        
        if (bytesReceived > 0) {
            std::string request(buffer);
            std::string response = handleRequest(request);
            send(clientSocket, response.c_str(), (int)response.length(), 0);
        }
        
        closesocket(clientSocket);
    }
    
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
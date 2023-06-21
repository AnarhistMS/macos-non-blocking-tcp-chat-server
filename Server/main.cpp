//
//  main.cpp
//  Server
//
//  Created by Алексей Кокоулин on 10.05.2023.
//

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define port 1488

const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;

int main(int argc, const char * argv[]) {
    // Create a socket for the server
       int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
       if (serverSocket == -1) {
           std::cerr << "Failed to create socket." << std::endl;
           return 1;
       } else {
           std::cout<< "Socket created" << std::endl;
       }

       // Set the socket to non-blocking mode
       int flags = fcntl(serverSocket, F_GETFL, 0);
       fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

       // Bind the socket to a specific IP address and port
       sockaddr_in serverAddress{};
       serverAddress.sin_family = AF_INET;
       serverAddress.sin_addr.s_addr = INADDR_ANY;
       serverAddress.sin_port = htons(8080);

       if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
           std::cerr << "Failed to bind." << std::endl;
           close(serverSocket);
           return 1;
       } else {
           std::cout<< "Address binded" << INADDR_ANY << std::endl;
       }

       // Listen for incoming connections
       if (listen(serverSocket, MAX_CLIENTS) == -1) {
           std::cerr << "Failed to listen." << std::endl;
           close(serverSocket);
           return 1;
       } else {
           std::cout<< "Socket listened" << std::endl;
       }

       // Create a vector to store client sockets
       std::vector<int> clientSockets;

       while (true) {
           // Accept incoming connections
           sockaddr_in clientAddress{};
           socklen_t clientAddressLength = sizeof(clientAddress);
           int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress),
                                     &clientAddressLength);

           if (clientSocket != -1) {
               // Add the new client socket to the vector
               clientSockets.push_back(clientSocket);
               std::cout << "New client connected. Socket fd: " << clientSocket << std::endl;

               // Set the new client socket to non-blocking mode
               flags = fcntl(clientSocket, F_GETFL, 0);
               fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
           }

           // Handle messages from connected clients
           for (auto it = clientSockets.begin(); it != clientSockets.end();) {
               char buffer[BUFFER_SIZE];

               // Receive data from the client
               long bytesRead = recv(*it, buffer, BUFFER_SIZE - 1, 0);

               if (bytesRead > 0) {
                   buffer[bytesRead] = '\0';
                   std::cout << "Received message from client " << *it << ": " << buffer << std::endl;

                   // Broadcast the message to all other connected clients
                   for (const auto& client : clientSockets) {
                       if (client != *it) {
                           send(client, buffer, strlen(buffer), 0);
                       }
                   }
               } else if (bytesRead == 0) {
                   // Client disconnected
                   std::cout << "Client " << *it << " disconnected." << std::endl;
                   close(*it);
                   it = clientSockets.erase(it);
                   continue;
               }

               ++it;
           }
       }
    return 0;
}

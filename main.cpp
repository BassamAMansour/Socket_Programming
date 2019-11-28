#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "RequestProcessor.h"

#define PORT "2000"
#define BACKLOG 10
#define RECV_BUFFER_SIZE 1000000

using namespace std;

void setupAddressInfo(addrinfo **returnedAddressInfo);

int bindToSocket(addrinfo **addressInfo);

void listenToPort(int socketFd);

void serveClients(int mainSocketFd);

void *get_in_addr(struct sockaddr *socketAddress);

int sendAllInBuffer(int socketFd, const string &response);

void serveClient(int clientSocketFd, const string &request);

void receiveFromClient(int clientSocketFd, fd_set &masterFds, fd_set &readFds, int &connectedClients);

int main() {
    struct addrinfo *addressInfo;

    setupAddressInfo(&addressInfo);
    int socketFd = bindToSocket(&addressInfo);
    listenToPort(socketFd);

    serveClients(socketFd);

    return 0;
}

void serveClients(int mainSocketFd) {

    fd_set masterFds = fd_set(); // masterFds file descriptor list
    fd_set readFds = fd_set();
    int maxFd = mainSocketFd;
    int connectedClients = 0;

    FD_SET(mainSocketFd, &masterFds);
    FD_SET(mainSocketFd, &readFds);

    while (true) {

        struct timeval timeout{};
        timeout.tv_sec = (connectedClients + 1) * 50;

        if (select(maxFd + 1, &readFds, nullptr, nullptr, &timeout) == -1) {
            perror("select");
            exit(4);
        } else {
            // run through the existing connections looking for data to read
            for (int i = 0; i <= maxFd; i++) {
                if (FD_ISSET(i, &readFds)) {
                    if (i == mainSocketFd) {// New Connection

                        struct sockaddr_storage clientAddress{};

                        // handle new connections
                        socklen_t addressLength = sizeof clientAddress;
                        int clientSocketFd = accept(mainSocketFd, (struct sockaddr *) &clientAddress, &addressLength);

                        if (clientSocketFd == -1) {
                            perror("accept");
                        } else {
                            FD_SET(clientSocketFd, &masterFds); // add to masterFds set
                            FD_SET(clientSocketFd, &readFds); // add to readFds set

                            if (clientSocketFd > maxFd) {    // keep track of the max
                                maxFd = clientSocketFd;
                            }

                            char remoteIP[INET6_ADDRSTRLEN];

                            printf("selectserver: new connection from %s on socket %d\n",
                                   inet_ntop(clientAddress.ss_family, get_in_addr((struct sockaddr *) &clientAddress),
                                             remoteIP, INET6_ADDRSTRLEN), clientSocketFd);

                            connectedClients++;

                            struct timeval tv{};
                            tv.tv_sec = timeout.tv_sec / connectedClients;
                            setsockopt(clientSocketFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                            receiveFromClient(clientSocketFd, masterFds, readFds, connectedClients);
                        }
                    } else { // Receiving from existing connection
                        receiveFromClient(i, masterFds, readFds, connectedClients);
                    }
                }
            }
        }
    }
    }

void receiveFromClient(int clientSocketFd, fd_set &masterFds, fd_set &readFds, int &connectedClients) {
    int receivedBytes;
    char receiveBuffer[RECV_BUFFER_SIZE];

    if ((receivedBytes = recv(clientSocketFd, receiveBuffer, RECV_BUFFER_SIZE, 0)) <= 0) {
        // got error or connection closed by client
        if (receivedBytes == 0) {
            // connection closed
            printf("selectserver: socket %d hung up\n", clientSocketFd);
        } else {
            perror("recv");
        }

        connectedClients--;
        close(clientSocketFd); // bye!
        FD_CLR(clientSocketFd, &masterFds); // remove from masterFds set
        FD_CLR(clientSocketFd, &readFds);
    } else {
        serveClient(clientSocketFd, string(receiveBuffer));
    }
}

void serveClient(int clientSocketFd, const string &request) {
    cout << request << endl;
    string response = RequestProcessor::processRequest(request);
    sendAllInBuffer(clientSocketFd, response);
    cout << response << endl;
}

void listenToPort(int socketFd) {
    if (listen(socketFd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
}

int bindToSocket(addrinfo **addressInfo) {
    int socketFd = -1;
    struct addrinfo *addressIterator;

    // loop through all the results and bind to the first we can
    for (addressIterator = *addressInfo; addressIterator != nullptr; addressIterator = addressIterator->ai_next) {

        if ((socketFd = socket(addressIterator->ai_family, addressIterator->ai_socktype,
                               addressIterator->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // Free up socket if in use
        int yes = 1;
        if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(socketFd, addressIterator->ai_addr, addressIterator->ai_addrlen) == -1) {
            close(socketFd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (addressIterator == nullptr || socketFd == -1) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    freeaddrinfo(*addressInfo);

    return socketFd;
}

void setupAddressInfo(addrinfo **returnedAddressInfo) {
    struct addrinfo hints{};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(nullptr, PORT, &hints, returnedAddressInfo);

    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
}

int sendAllInBuffer(int socketFd, const string &response) {
    int bufferLength = response.size();
    char buffer[bufferLength + 1];
    response.copy(buffer, bufferLength + 1);

    int total = 0; // how many bytes we've sent
    int bytesLeft = bufferLength; // how many we have left to send
    int sentBytes = -1;

    while (total < bufferLength) {
        sentBytes = send(socketFd, buffer + total, bytesLeft, 0);
        if (sentBytes == -1) { break; }
        total += sentBytes;
        bytesLeft -= sentBytes;
    }

    bufferLength = total; // return number actually sent here

    if (sentBytes == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", bufferLength);
    }

    return sentBytes == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *socketAddress) {
    if (socketAddress->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) socketAddress)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) socketAddress)->sin6_addr);
}
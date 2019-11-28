//
// Created by bassam on 31/10/2019.
//

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#ifndef SOCKET_PROGRAMMING_REQUESTPROCESSOR_H
#define SOCKET_PROGRAMMING_REQUESTPROCESSOR_H

using namespace std;

class RequestProcessor {
public:
    struct ClientRequest {
        int requestType;
        string request;
        string path;
        static const int REQ_TYPE_GET = 0, REQ_TYPE_POST = 1;
    };

public:
    static string processRequest(string requestString);

    static ClientRequest parseRequest(string requestString);

    static int getRequestType(const vector<string> &splitRequest);

    static string postRequest(const ClientRequest &request);

    static string getRequest(const ClientRequest &request);
};


#endif //SOCKET_PROGRAMMING_REQUESTPROCESSOR_H

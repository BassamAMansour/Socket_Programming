//
// Created by bassam on 31/10/2019.
//

#include "RequestProcessor.h"
#include "StringUtils.h"

string RequestProcessor::processRequest(string requestString) {
    struct ClientRequest request = parseRequest(requestString);

    switch (request.requestType) {
        case ClientRequest::REQ_TYPE_GET:
            return getRequest(request);
        case ClientRequest::REQ_TYPE_POST:
            return postRequest(request);
        default:
            return "Error identifying request type.";
    }
}

RequestProcessor::ClientRequest RequestProcessor::parseRequest(string requestString) {
    vector<string> headerAndBody = StringUtils::split(requestString, "\r\n");
    vector<string> header = StringUtils::split(headerAndBody[0], " ");

    ClientRequest clientRequest{};

    clientRequest.requestType = getRequestType(header);
    clientRequest.path = header[1];
    clientRequest.request = headerAndBody[1];

    return clientRequest;
}

int RequestProcessor::getRequestType(const vector<string> &splitRequest) {
    if (splitRequest[0] == "GET") {
        return ClientRequest::REQ_TYPE_GET;
    } else if (splitRequest[0] == "POST") {
        return ClientRequest::REQ_TYPE_POST;
    } else {
        cout << "Error parsing request type. " << splitRequest[0] << endl;
        return -1;
    }
}

string RequestProcessor::postRequest(const RequestProcessor::ClientRequest &request) {
    ofstream outFile(request.path, ios::binary);

    outFile << request.request << endl;

    outFile.close();

    return "HTTP/1.1 200 OK\r\n\r\n";
}

string RequestProcessor::getRequest(const RequestProcessor::ClientRequest &request) {
    string response = "HTTP/1.1 200 OK\r\n\r\n";
    string path(".");
    path.append(request.path);
    ifstream inFile(path, ios::binary);

    if (inFile.fail()) return "HTTP/1.1 404 NOT FOUND\r\n\r\n";

    string line;

    while (inFile) {
        getline(inFile, line);
        response.append(line);
    }

    inFile.close();

    return response;
}

//
// Created by bassam on 02/11/2019.
//

#ifndef SOCKET_PROGRAMMING_STRINGUTILS_H
#define SOCKET_PROGRAMMING_STRINGUTILS_H


#include <bits/types/stack_t.h>
#include <vector>
#include <string>

using namespace std;

class StringUtils {
public:
    static vector <string> split(const string &str, const string &sep);
};


#endif //SOCKET_PROGRAMMING_STRINGUTILS_H

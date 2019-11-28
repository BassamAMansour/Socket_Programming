//
// Created by bassam on 02/11/2019.
//

#include <cstring>
#include "StringUtils.h"

vector<string> StringUtils::split(const string &str, const string &sep) {
    char *cstr = const_cast<char *>(str.c_str());
    char *current;
    vector<string> arr;
    current = strtok(cstr, sep.c_str());
    while (current != nullptr) {
        arr.emplace_back(current);
        current = strtok(nullptr, sep.c_str());
    }
    return arr;
}

#pragma once
#include <iostream>
#include <string>
#include <cctype>
using namespace std;

inline string toLower(string str){
    for(char &c : str){ c = tolower(c); }
    return str;
}
#pragma once
#include <iostream>
#include <string>
#include <cctype>
using namespace std;

inline string toLower(string str){
    for(char &c : str){ c = tolower(c); }
    return str;
}
inline double enterPrice(double &price){
    do{
        cout<<"Enter Price: ";
        cin>>price;
    } while(price < 0);
    return price;
}
inline int enterShares(int &shares){
    do{
        cout<<"Enter Price: ";
        cin>>shares;
    } while(shares < 0);
    return shares;
}
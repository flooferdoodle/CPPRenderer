#ifndef CustomException_DEFINED
#define CustomException_DEFINED

#include <exception>
#include <iostream>
#include <string>

using namespace std;

class CustomException : public exception {
private:
    string message;
public:
    CustomException(const char* msg) : message(msg) {}
    const char* what() const throw() { return message.c_str(); }
};
#endif
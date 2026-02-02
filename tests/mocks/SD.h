#ifndef SD_H
#define SD_H

#include "Arduino.h"

class File {
public:
    operator bool() const { return true; }
    void close() {}
    void println(const char* s) { mock_sink += s[0]; }
    void println(const String& s) { mock_sink += s.c_str()[0]; }
    void println(int n) { mock_sink += n; }
    void print(const char* s) { mock_sink += s[0]; }
    void print(const String& s) { mock_sink += s.c_str()[0]; }
    void print(int n) { mock_sink += n; }
    void print(int n, int f) { mock_sink += n; }
    void print(char c) { mock_sink += c; }
    int available() { return 0; }
    int read(uint8_t* buf, size_t size) { return 0; }
};

class SDClass {
public:
    bool begin(int pin) { return true; }
    bool exists(const char* filepath) { return true; }
    File open(const char* filepath, int mode = 0) { return File(); }
};

extern SDClass SD;

#endif

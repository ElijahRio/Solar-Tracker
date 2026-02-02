#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstring>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>

extern volatile int mock_sink;

// Mock String class to simulate allocation overhead
class String {
public:
    char* buffer;

    String(const char* str) {
        if (str) {
            size_t len = strlen(str);
            buffer = new char[len + 1];
            strcpy(buffer, str);
        } else {
            buffer = new char[1];
            buffer[0] = '\0';
        }
    }

    String(const String& other) {
        size_t len = strlen(other.buffer);
        buffer = new char[len + 1];
        strcpy(buffer, other.buffer);
    }

    ~String() {
        delete[] buffer;
    }

    const char* c_str() const { return buffer; }
};

// Standard constants and types
#define OUTPUT 0x1
#define INPUT 0x0
#define INPUT_PULLUP 0x2
#define HIGH 0x1
#define LOW 0x0
#define FILE_WRITE 1
#define DEC 10
#define A0 54
#define A1 55

// Mock Serial
class SerialClass {
public:
    void begin(unsigned long baud) {}
    void println(const char* s) { mock_sink += s[0]; }
    void println(const String& s) { mock_sink += s.c_str()[0]; }
    void println(int n) { mock_sink += n; }
    void print(const char* s) { mock_sink += s[0]; }
    void print(const String& s) { mock_sink += s.c_str()[0]; }
    void print(int n) { mock_sink += n; }
    void print(int n, int f) { mock_sink += n; }
    void print(char c) { mock_sink += c; }
    void write(const uint8_t* buf, size_t size) { mock_sink += size; }
    int available() { return 0; }
    int read() { return -1; }
};

extern SerialClass Serial;

// Mock functions
inline unsigned long millis() { return 0; }
inline void delay(unsigned long ms) {}
inline void pinMode(int pin, int mode) {}
inline void digitalWrite(int pin, int val) {}
inline int digitalRead(int pin) { return LOW; }
inline int analogRead(int pin) { return 0; }
inline int abs(int x) { return x > 0 ? x : -x; }
inline const char* F(const char* s) { return s; } // F() macro

#endif

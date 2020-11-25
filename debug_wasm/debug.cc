#include <iostream>
#include <emscripten.h>

using namespace std;

extern "C" char EMSCRIPTEN_KEEPALIVE add(char x, char y) {
    cout << "This comes from C++." << endl;
    return x + y;
}


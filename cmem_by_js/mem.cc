//export1.cc
#ifndef EM_PORT_API
#	if defined(__EMSCRIPTEN__)
#		include <emscripten.h>
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#		else
#			define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#		endif
#	else
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype
#		else
#			define EM_PORT_API(rettype) rettype
#		endif
#	endif
#endif
#include <iostream>
#include <stdio.h>

int g_int = 42;
double g_double = 3.1415926;

EM_PORT_API(int*) get_int_ptr() {
    std::cout << "&g_int value: " << &g_int << std::endl;
    return &g_int;
}

EM_PORT_API(double*) get_double_ptr() {
    return &g_double;
}

EM_PORT_API(void) print_data() {
    std::cout << "CPP{g_int: " << g_int << "}" << std::endl;
    printf("C{g_double:%lf}\n", g_double);
}
## JavaScript调用C函数

C:
int show_me_the_answer() {
    return 42;
}
float add(float a, float b) {
    return a + b;
}

js:
Module = {};
Module.onRuntimeInitialized = function() {
    console.log(Module._show_me_the_answer());
    console.log(Module._add(12, 1.0));
}

## JavaScript函数 注入 C
C:
int js_add(int a, int b);
void js_console_log_int(int param);
void print_the_answer() {
    int i = js_add(21,21);
    js_console_log_int(i);
}
js:
mergeInto(LibraryManager.library, {
    js_add: function (a, b) {
        console.log("js_add");
        return a + b;
    },

    js_console_log_int: function (param) {
        console.log("js_console_log_int:" + param);
    }
})


无论编译目标是asm.js还是wasm, C/C++代码眼中的内存空间实际上对应的都是Emscripten提供的ArrayBuffer对象：Module.buffer
C/C++代码能直接通过地址访问的数据全部在内存中(包括运行时堆，运行时栈),而内存对应Module.buffer 对象，C/C++代码能直接访问
的数据事实上被限制在Module.buffer内部，JavaScript环境中的其他对象无法被C/C++直接访问——因此我们称其为单向透明的内存模型。

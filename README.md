# wasm_learning


### ccall/cwrap
JavaScipt调用C/C++时只能使用Number作为参数，因此如果参数是字符串，数组等非Number等类型，需要拆分为一下步骤：
 1. 使用Module._malloc()在Module堆中分配内存，获取地址ptr:
 2. 将字符串/数组等数据拷入内存的ptr处
 3. 使用Module._free()释放ptr
为了简化调用过程：Emscipten提供了ccall/cwrap封装函数
#### ccall
eg: var result = Module.ccall(ident, returnType, argTypes, args);
args: ident: C导出函数的函数名(不含"_"下划线前缀);
      returnType: C导出函数的返回值类型，可以为'boolean', 'number', 'string', 'null', 分别代表返回值为：布尔值，数值，字符串，无返回值:
      argTypes: C导出函数的参数类型的数组。参数类型可以为'number', 'string', 'array', 分别代表数值，字符串，数组;
      args: 参数数组。
eg:
```C
EM_PORT_API(double) add(double a, int b) {
    return a + (double)b;
}

编译：
emcc ccal_wrap.cc -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap'] -o ccall_wrap.js
在JavaScript中可以使用以下方法调用：
var result = Module.ccall('add', 'number', ['number', 'number'], [13.0, 42]);
```
ccall的优势在于可以直接使用字符串/Uint8Array/Int8Array作为参数

#### cwrap
ccall虽然封装了字符串等数据类型，但调用时仍然需要填入参数类型数组，参数列表等，为此cwrap进行了进一步封装:
var func = Module.cwrap(ident, returType, argTypes);
参数：
    1. ident: C导出函数的函数名(不含"_"下划线前缀);
    2. returnType: C导出函数的返回值类型，可以为'boolean', 'number', 'string', 'null',分别表示函数返回值为布尔值，
       数值，字符串，无返回值
    3. argTypes: C导出函数的参数类型的数组。参数类型可以为'number', 'string', 'array',分别代表数值，字符串，数组；
eg：
```C
var c_add = Module.cwrap('add', 'number', ['number', 'number']);

C导出的函数add()被封装为c_add(),这些封装方法与普通的JavaScript方法一样可以被直接调用
```

### call/cwrap潜在的风险
虽然简化了字符串参数的交换，但是代价就是，输入参数为'string'/'array'时，ccall/cwrap在C环境的栈上分配了相应的空间，并将数据拷入了其中
然后调用相应的导出函数。
所以频繁使用ccall/cwrap生成的胶水代码，可能会爆栈

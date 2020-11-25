# 多线程实验性
编译命令如下:
emcc -O2 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=2 -o pthreadTest.html pthread_demo.cc
echo "Beginning Build: "
rm -r dist
mkdir -p dist
cd ./FFmpeg
echo "emconfigure"
emconfigure ./configure --cc="emcc" --enable-cross-compile --target-os=none --arch=x86_32 --cpu=generic --prefix=$(pwd)/../dist \
    --disable-ffplay --disable-ffprobe --disable-asm --disable-doc --disable-devices --disable-pthreads \
    --disable-w32threads --disable-network --disable-hwaccels --disable-parsers --disable-bsfs \
    --disable-debug --disable-protocols --disable-indevs --disable-outdevs --enable-protocol=file
if [ -f "Makefile" ]; then
 echo "make clean"
 make clean
fi 
echo "make"
make
echo "make install"
make install
cd ../

rm -rf libffmpeg.wasm libffmpeg.js
export TOTAL_MEMORY=67108864

echo "Running Emscripten..."
emcc web.c process.c dist/lib/libavformat.a dist/lib/libavcodec.a dist/lib/libavutil.a dist/lib/libswscale.a dist/lib/libswresample.a \
 -O3 \
 -I "dist/include" \
 -s WASM=1 \
 -s TOTAL_MEMORY=${TOTAL_MEMORY} \
 -s ALLOW_MEMORY_GROWTH=1 \
 -o libffmpeg.js
echo "finished Build"

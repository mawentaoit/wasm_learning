rm -rf libffmpeg.wasm libffmpeg.js
export TOTAL_MEMORY=67108864

echo "Running Emscripten..."
emcc 
<!DOCType html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>ffmpeg wasm截取视频帧功能</title>
    <style>
        input,label,p {font-size:16px}
        body {margin: 20px}
        @media (max-width: 700px) {input{display:block;margin-bottom:15px}}
    </style>
</head>
<body>
<form style="margin-bottom: 20px;">
    <p>请选择一个视频（本地操作不会上传）</p>
    <input type="file" required name="file">
    <label>时间(秒)</label><input type="number" step="1" value="0" required name="time">
    <input type="submit" value="获取图像" style="font-size:16px;">
</form>
<p id="text-tip">WASM下载解析中，请稍候。。。<br>（解析需约5s，手机上需要的时间较长，可先选择一个视频文件）</p>
<div id="error-tip"></div>
<canvas width="600" height="400" id="canvas"></canvas>
<script src="index.js"></script>
<script>
!function() {
    let setFile = null;
    let $text = document.querySelector('#text-tip'),
        $errorTip = document.querySelector('#error-tip');
    if (!window.WebAssembly) {
        let tip = '你的浏览器不支持WASM!';
        if (/iPhone|iPad/.test(window.navigator.userAgent)) {
            tip += ' ios需要11以上版本'
        }
        $text.textContent = tip;
        return;
    }
    let form = document.querySelector('form');
    Module.onRuntimeInitialized = function () {
        setFile = Module.cwrap('setFile', 'number', ['number', 'number', 'number']);
        $text.textContent = 'WASM初始化完毕，请选择视频文件';
        setTimeout(() => $text.remove(), 3000);
        console.log('WASM initialized done!');
        if (form.file.value) {
            process();
        }
        // console.log(setFile(5));
    };
    form.onsubmit = form.file.onchange = process;
    function process (event) {
        if (!form.file.value) {
            return;
        }
        event && event.preventDefault();
        let fileReader = new FileReader();
        fileReader.onload = function () {
            if (!setFile) {
                // alert('WASM未加载解析完毕，请稍候');
                // form.file.value = '';
                return;
            }
            // console.log(buffer.length);
            // console.log(buffer);
            // let ptr = setFile(buffer, buffer.length);
            // console.log(offset);
            let ptr = 0;
            let offset = 0;
            try {
                let buffer = new Uint8Array(this.result);
                offset = Module._malloc(buffer.length);
                console.log(offset);
                Module.HEAP8.set(buffer, offset);
                ptr = setFile(offset, buffer.length, +form.time.value * 1000);
            } catch (e) {
                $errorTip.innerHtml = 'oh, no! 不小心挂了，请刷新页面重试';
                throw e;
            }
            let width = Module.HEAPU32[ptr / 4]
                height = Module.HEAPU32[ptr / 4 + 1],
                imgBufferPtr = Module.HEAPU32[ptr / 4 + 2],
                imageBuffer = Module.HEAPU8.subarray(imgBufferPtr, imgBufferPtr + width * height * 3);
            if (!width || !height) {
                $errorTip.textContent = '获取图片帧失败，图片宽高为0，时间可能超限';
                return;
            }
            drawImage(width, height, imageBuffer);
            Module._free(offset);
            Module._free(ptr);
            Module._free(imgBufferPtr);
        };
        // console.log(form.file.files[0]);
        let file = form.file.files[0];
        if (file.type && file.type.indexOf('video') !== 0) {
            $errorTip.textContent = '您选择的类型是：' + file.type + '不是一个视频文件';
            return;
        }
        $errorTip.textContent = '';
        fileReader.readAsArrayBuffer(file);
    };
    // 内存画布
    let memCanvas = document.createElement('canvas'),
        memContext = memCanvas.getContext('2d');
    let canvas = document.querySelector('#canvas'),
        ctx = canvas.getContext('2d');
    canvas.width = Math.max(600, window.innerWidth - 40);

    function drawImage(width, height, buffer) {
        let imageData = ctx.createImageData(width, height);
        let k = 0;
        for (let i = 0; i < buffer.length; i++) {
            if (i && i % 3 === 0) {
                imageData.data[k++] = 255;
            }
            imageData.data[k++] = buffer[i];
        }
        imageData.data[k] = 255;
        memCanvas.width = width;
        memCanvas.height = height;
        canvas.height = canvas.width * height / width;
        memContext.putImageData(imageData, 0, 0, 0, 0, width, height);
        ctx.drawImage(memCanvas, 0, 0, width, height, 0, 0, canvas.width, canvas.height);
    }
}();
</script>
</body>
</html>
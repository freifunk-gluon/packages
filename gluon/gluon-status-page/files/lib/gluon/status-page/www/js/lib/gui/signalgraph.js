"use strict";
define(function () {
  return function (canvas, min, max, holdtime, autowidth) {
    var i = 0;
    var v = null;

    var ctx = canvas.getContext("2d");

    var graphWidth;

    var last = 0;

    var fadei = 0;
    var peakHold = null;
    var peakTime = null;

    resize();

    window.addEventListener("resize", resize, false);
    window.requestAnimationFrame(step);

    function peak() {
      if (v != null)
        if (peakHold == null || v > peakHold) {
          peakHold = v;
          peakTime = new Date().getTime();
        }

      ctx.clearRect(0, 0, 18, canvas.height);
      ctx.fillStyle = "rgba(255, 180, 0, 0.3)";
      ctx.fillRect(0, 0, 18, canvas.height);

      if (v) {
        var x = Math.round(scale(v, min, max, canvas.height));
        ctx.fillStyle = "#ffb400";
        ctx.fillRect(0, x, 18, canvas.height);
      }

      ctx.fillStyle = "#dc0067";
      ctx.fillRect(0, Math.round(scale(peakHold, min, max, canvas.height)) - 3, 18, 3);
    }

    function step(timestamp) {
      var delta = timestamp - last;

      if (delta > 20) {
        draw();
        fadei++;
        if (fadei > 2) {
          fade();
          fadei = 0;
        }

        var peakAge = (new Date().getTime()) - peakTime

        if (peakAge > holdtime*1000)
          peakHold = null;

        peak();

        last = timestamp;
      }

      window.requestAnimationFrame(step);
    }

    function draw() {
      ctx.save();
      ctx.rect(20, 0, canvas.width, canvas.height);
      ctx.clip();
      var x = Math.round(scale(v, min, max, canvas.height));
      if (x)
        drawPixel(i + 20, x);

      ctx.restore();

      i = (i + 1) % graphWidth;
    }

    function drawPixel (x, y) {
      var radius = 1.5;
      ctx.save();
      ctx.fillStyle = "#ffb400";
      ctx.shadowBlur = 15;
      ctx.shadowColor = "#ffb400";
      ctx.shadowOffsetX = 3;
      ctx.shadowOffsetY = 3;
      ctx.globalCompositeOperation = "lighter";
      ctx.beginPath();

      ctx.arc(x, y, radius, 0, Math.PI * 2, false);
      ctx.closePath();
      ctx.fill();
      ctx.restore();
    }

    function fade() {
      var lastImage = ctx.getImageData(19, 0, graphWidth + 1, canvas.height);
      var pixelData = lastImage.data;

      for (var i = 0; i < pixelData.length; i += 4) {
        pixelData[i+3] -= 2;
      }

      ctx.putImageData(lastImage, 19, 0);
    }

    function scale(n, min, max, height) {
      return (1 - (n-min)/(max-min)) * height;
    }

    function resize() {
      var newWidth = canvas.parentNode.clientWidth;

      if (newWidth == 0)
        return;

      var lastImage = ctx.getImageData(0, 0, newWidth, canvas.height);
      canvas.width = newWidth;
      graphWidth = canvas.width - 20;
      ctx.putImageData(lastImage, 0, 0);
    }

    return function (n) {
      v = n;
    }
  }
})

// handtracking.js

console.log("handtracking.js running");


const videoElement = document.getElementById("input-video");
const canvasElement = document.getElementById("output-canvas");

console.log("videoElement:", videoElement);
console.log("canvasElement:", canvasElement);

if (!videoElement) {
  console.error("❌  #input-video not found—check that <video id='input-video'> is BEFORE handtracking.js");
}
if (!canvasElement) {
  console.error("❌  #output-canvas not found—check that <canvas id='output-canvas'> is BEFORE handtracking.js");
}

const canvasCtx = canvasElement.getContext("2d");

console.log("Hands global:", typeof Hands);
console.log("Camera global:", typeof Camera);
console.log("drawConnectors global:", typeof drawConnectors);
console.log("drawLandmarks global:", typeof drawLandmarks);
if (typeof Hands !== "function") {
  console.error("❌  Hands is not defined—check that hands.js loaded before handtracking.js");
}
if (typeof Camera !== "function") {
  console.error("❌  Camera is not defined—check that camera_utils.js loaded before handtracking.js");
}
if (typeof drawConnectors !== "function") {
  console.error("❌  drawConnectors is not defined—check that drawing_utils.js loaded before handtracking.js");
}
if (typeof drawLandmarks !== "function") {
  console.error("❌  drawLandmarks is not defined—check that drawing_utils.js loaded before handtracking.js");
}




// 2) Create a global variable so render.js can see the latest landmarks
window.latestLandmarks = null;

// 3) Instantiate MediaPipe Hands (using the global "Hands" that your HTML <script> brought in)
const hands = new Hands({
    locateFile: (file) => {
        return `https://cdn.jsdelivr.net/npm/@mediapipe/hands@0.4/${file}`;
    }
});
hands.setOptions({
    maxNumHands: 2, // maximum number of hands to detect
    modelComplexity: 0, // 0: light, 1: full, 2: heavy
    useCpuInference: false, // true: use CPU, false: use GPU (default)
    minDetectionConfidence: 0.7,
    minTrackingConfidence: 0.3
});

// 4) Whenever MediaPipe finishes detecting landmarks, store them in window.latestLandmarks
hands.onResults((results) => {
    if (results.multiHandLandmarks && results.multiHandLandmarks.length > 0) {
        window.latestLandmarks = results.multiHandLandmarks[0];

        //draw an overlay on the hidden canvas for debugging
        canvasElement.width = videoElement.videoWidth;
        canvasElement.height = videoElement.videoHeight;
        canvasCtx.save();
        canvasCtx.clearRect(0, 0, canvasElement.width, canvasElement.height);
        canvasCtx.drawImage(results.image, 0, 0, canvasElement.width, canvasElement.height);
        drawConnectors(canvasCtx, window.latestLandmarks, Hands.HAND_CONNECTIONS, {
            color: "#00FF00",
            lineWidth: 2
        });
        drawLandmarks(canvasCtx, window.latestLandmarks, {
            color: "#FF0000",
            lineWidth: 1
        });
        canvasCtx.restore();
    } else {
        window.latestLandmarks = null;
        // clear the hidden canvas here if you want:
        // canvasCtx.clearRect(0,0,canvasElement.width,canvasElement.height);
    }
});

// 5) Start the user’s webcam via MediaPipe Camera utility, but call it "mpCamera"
const mpCamera = new Camera(videoElement, {
    onFrame: async () => {
        await hands.send({ image: videoElement });
    },
    width: 200,   // adjust as needed
    height: 200,
});
// 6) Start the camera
//mpCamera.start();

mpCamera.start().then(() => {
  console.log("mpCamera.start() resolved—webcam streaming should be on");
}).catch((err) => {
  console.error("mpCamera.start() failed:", err);
});
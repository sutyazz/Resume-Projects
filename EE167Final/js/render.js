/**
 * @file EE267 Virtual Reality
 * Homework 4
 * Build your HMD, Implement Stereo Rendering and Lens Distortion Correction
 *
 * In our homework, we heavily rely on THREE.js library for rendering.
 * THREE.js is a wonderful library to render a complicated scene without
 * cumbersome raw WebGL/OpenGL programming related to GPU use. Furthermore,
 * it also hides most of the math of computer graphics to allow designers to
 * focus on the scene creation. However, this homework does not use such
 * capabilities. We will compute them manually to understand the mathematics
 * behind the rendering pipeline!
 *
 * Instructor: Gordon Wetzstein <gordon.wetzstein@stanford.edu>
 *
 * The previous C++/OpenGL version was developed by Robert Konrad in 2016, and
 * the JavaScript/WebGL version was developed by Hayato Ikoma in 2017.
 *
 * @copyright The Board of Trustees of the Leland Stanford Junior University
 * @version 2022/05/05
 * This version uses Three.js (r127), stats.js (r17) and jQuery (3.2.1).
 */

// ───────────────────────────────────────────────────────────────────────────
// SIMPLE DIAGNOSTICS OBJECT
// ───────────────────────────────────────────────────────────────────────────
window.diag = {
	frameTimes: [],            // raw timestamps (ms)
	hitsPerTeapot: [],         // will be resized once teapots[] exists
	prevHitStates: null,       // to detect “new” hits
	collisionCount: 0,         // total number of hits
	prevHandPos: null,         // THREE.Vector3 at last frame
	velocities: [],
	maxHandVel: 0              // maximum velocity (units/s) seen so far
};

// After your teapots array is created (e.g. right after you push/populate it),
// initialize hitsPerTeapot to zeros:
if (window.teapots && window.teapots.length) {
	window.diag.hitsPerTeapot = new Array(window.teapots.length).fill(0);
	window.diag.prevHitStates = new Array(window.teapots.length).fill(false);
}

// Call this from the console anytime to see a short summary:
window.dumpDiagnostics = function () {
	// 1. Compute per‐frame deltas → FPS stats
	const ft = window.diag.frameTimes;
	let deltas = [];
	for (let i = 1; i < ft.length; i++) {
		deltas.push(ft[i] - ft[i - 1]);
	}
	const fpsList = deltas.map(d => 1000 / d);
	const meanFPS = fpsList.reduce((a, b) => a + b, 0) / Math.max(fpsList.length, 1);
	const fpsStd = Math.sqrt(
		fpsList.reduce((sum, x) => sum + (x - meanFPS) ** 2, 0) / Math.max(fpsList.length, 1)
	);

	console.group("=== DIAGNOSTICS SUMMARY ===");
	console.log(`Frames recorded: ${ft.length}`);
	console.log(`Average FPS: ${meanFPS.toFixed(1)}  (σ = ${fpsStd.toFixed(1)})`);
	console.log(`Max blade (hand) velocity: ${window.diag.maxHandVel.toFixed(2)} units/sec`);
	console.log("Hits per teapot:");
	window.diag.hitsPerTeapot.forEach((c, i) => {
		console.log(`  Teapot ${i}: ${c}`);
	});
	console.log(`Total collisions: ${window.diag.collisionCount}`);
	console.groupEnd();
};
//----------------------------------------------


// Global variables to control the rendering mode
const STEREO_MODE = 0;
const STEREO_UNWARP_MODE = 1;

var renderingMode = STEREO_MODE;//change to start in stereo mode


// Set up display parameters.
// The display parameters are hard-coded in the class since all teams have
// the same HMD.
var dispParams = new DisplayParameters();


// Create an instance for Javascript performance monitor.
// In our class, we would like to visualize the number of frames rendered
// in every second. For this purpose, stats.js is a handy tool to achieve it.
// https://github.com/mrdoob/stats.js/
var stats = new Stats();

// Add a DOM element of the performance monitor to HTML.
$(".renderCanvas").prepend(stats.dom);


// Create a THREE's WebGLRenderer instance.
// Since we are not going to use stencil and depth buffers in this
// homework, those buffers are turned off. These two buffers are commonly
// used for more advanced rendering.
var webglRenderer = new THREE.WebGLRenderer({
	antialias: false,
	stencil: false,
	depth: true,
});

// keep track of elapsed time between frames // addition sutyazz
const clock = new THREE.Clock();
// end addition sutyazz


// Add a DOM element of the renderer to HTML.
$(".renderCanvas").prepend(webglRenderer.domElement);


// Set the size of the renderer based on the current window size.
webglRenderer.setSize(dispParams.canvasWidth, dispParams.canvasHeight);


//SUTYAZZ ADDITION
// --------------------------------------------
var teapots = [];

// --------------------------------------------
// FLYING teapots
// parameters
const NUM_FLYERS = 1;
const START_Z = -1000;    // spawn this far “in front” of viewer
const RESET_Z = 2000;     // once they pass closer than this, reset
const SPEED = 600;    // mm per second toward viewer

for (let i = 0; i < NUM_FLYERS; i++) {
	// random x/y spread
	const x = (Math.random() - 0.5) * 1000;
	const y = (Math.random() - 0.5) * 1000;
	const z = START_Z; //+ Math.random() * 1000; //sutyazz addition commented out the random z start
	const flyer = new Teapot(
		new THREE.Vector3(x, y, z),
		$("#vShaderMultiPhong").text(),
		$("#fShaderMultiPhong").text()
	);
	// give it a per-second velocity straight toward viewer
	flyer.velocity = new THREE.Vector3(0, 0, SPEED);
	teapots.push(flyer);
}


// ───────────────────────────────────────────────────────────
// 1) blade geometry + material
const bladeLength = 800;     // height
const bladeWidth = 5;      // thickness
const bladeDepth = 10;       // slim edge
const bladeGeo = new THREE.BoxGeometry(
	bladeWidth, bladeLength, bladeDepth
);
const bladeMat = new THREE.MeshStandardMaterial({
	color: 0x888888,
	metalness: 0.8,
	roughness: 0.2
});

// 2) create mesh & add to scene
const blade = new THREE.Mesh(bladeGeo, bladeMat);
blade.geometry.computeBoundingBox();
const bbox = blade.geometry.boundingBox;
const bladeHeight = bbox.max.y - bbox.min.y; // full height along Y
const halfHeight = bladeHeight / 2;
blade.geometry.translate(0, halfHeight, 0); // center it at origin

// set once, before animation loop
const BLADE_Z = 800;
blade.position.set(0, 0, BLADE_Z);

// tilt it a bit so it looks “edgier”
blade.rotation.z = Math.PI * 0.1;  // ~6° roll






// Create an instance of our StateCoontroller class.
// By using this class, we store the mouse movement to change the scene to be
// rendered.
var sc = new StateController(dispParams);


// Set the teapots to the renderer
var standardRenderer =
	new StandardRenderer(webglRenderer, teapots, dispParams);

var stereoUnwarpRenderer =
	new StereoUnwarpRenderer(webglRenderer, dispParams);


// Instantiate our MVPmat class
var mat = new MVPmat(dispParams);

//Add the Blade and corridor to the scene / sutyazz addition
standardRenderer.teapotScene.add(blade);
let previousBladePos = blade.position.clone();
const SMOOTHING = 0.2; // between 0 (no movement) and 1 (no smoothing)

// end sutyazz addition


// Start rendering!
animate();



// animate
// This function is the main function to render the scene repeatedly.
//
// Note:
// This function uses some global variables.
//
// Advanced note:
// requestAnimationFrame() is a WebAPI which is often used to perform animation.
// Importantly, requestAnimationFrame() is asynchronous, which makes this
// rendering loop not recursive. requestAnimationFrame immediately returns, and
// the following codes are executed continuously. After a certain amount of
// time, which is determined by a refresh rate of a monitor, the passed callback
// function is executed. In addition, when the window is not displayed (i.e.
// another tab of your browser is displayed), requestAnimationFrame
// significantly reduces its refresh rate to save computational resource and
// battery.
//
// If you are interested, please check out the documentation of
// requestAnimationFrame().
// https://developer.mozilla.org/en-US/docs/Web/API/window/requestAnimationFrame
function animate() {

	requestAnimationFrame(animate);

	//log timestamp for diagnostics
	window.diag.frameTimes.push(performance.now());

	// Start performance monitoring
	stats.begin();

	// compute how much time passed
	const delta = clock.getDelta();  // seconds since last frame
	const t = clock.getElapsedTime(); //sutyazz addition

	//teapot spin rate: π radians/sec (i.e. 180° per second)
	const SPIN_SPEED = Math.PI / 8; // sutyazz addition

	// advance any teapots that have a velocity field
	teapots.forEach(tp => {
		if (tp.velocity) {
			// spin the geometry around its Y‐axis
			tp.geometry.rotateX(SPIN_SPEED * delta);
			// advance position
			tp.position.addScaledVector(tp.velocity, delta);
			// if it’s past the viewer, send it back out front
			if (tp.position.z > RESET_Z) {
				tp.position.z = START_Z - Math.random() * 1000; //sutyazz addition commented out the random z start
				tp.position.x = (Math.random() - 0.5) * 1000;
				tp.position.y = (Math.random() - 0.5) * 1000;
				tp.isHit = false;	 // reset hit flag
				tp.mesh.material.uniforms.material.value.diffuse.set(0, 1, 0);
			}
		}
	});

	//sutyazz addition -----------------------------------------------
	// ── HAND-TRACKED BLADE POSITION/ORIENTATION (with roll, pitch & yaw) ─────────────────────────
	if (window.latestLandmarks) {
		// MediaPipe Hands indices:
		//   0  = wrist
		//   2  = thumb MCP
		//   4  = thumb tip
		//   5  = index-finger MCP (base joint)
		//   8  = index-finger tip
		//   9  = middle-finger MCP
		//  13  = ring finger MCP
		//  17  = pinky-finger MCP

		const lmWrist = window.latestLandmarks[0];
		const lmThumbMCP = window.latestLandmarks[2];//these were changed to knuckle landmarks,	 needs to be changed back
		const lmThumbTip = window.latestLandmarks[4];//these were changed to knuckle landmarks, needs to be changed back
		const lmIndexMCP = window.latestLandmarks[5];
		const lmIndexTip = window.latestLandmarks[8];
		const lmPinkyMCP = window.latestLandmarks[17];

		// Helper: converts a normalized MediaPipe [0..1] → NDC [–1..+1], z=0.5, then unprojects to world-space
		function landmarkToWorld(lm) {
			const ndc = new THREE.Vector3(
				-((lm.x * 8) - 1) * 50,
				-((lm.y * 8) - 1) * 50,
				((lm.z * 8) - 1) * 50 - 400        // constant depth in NDC
			);
			ndc.unproject(window.threeCamera);
			return ndc;
		}

		// 1) Unproject the four key points:
		const posWrist = landmarkToWorld(lmWrist);
		//thumb tracking version
		const posThumbMCP = landmarkToWorld(lmThumbMCP);
		const posThumbTip = landmarkToWorld(lmThumbTip);
		//index tracking version
		const posIndexMCP = landmarkToWorld(lmIndexMCP);
		const posIndexTip = landmarkToWorld(lmIndexTip);
		const posPinkyMCP = landmarkToWorld(lmPinkyMCP);

		// 2) Put the blade’s origin at the index-tip:
		//blade.position.copy(posIndexTip);
		// 2) Build a smoothed position for the blade’s origin:
		//    newPos = lerp(previousBladePos, posIndexTip, SMOOTHING)
		const targetPos = posIndexTip;
		const newPos = previousBladePos.clone().lerp(targetPos, SMOOTHING);
		blade.position.copy(newPos);
		previousBladePos.copy(newPos);


		// 3) Compute “forward” (wrist → indexTip) and “palm normal” for index finger version:
		///*
		const forwardVec = new THREE.Vector3().subVectors(posIndexTip, posWrist).normalize();
		const palmVec1 = new THREE.Vector3().subVectors(posIndexMCP, posWrist);
		const palmVec2 = new THREE.Vector3().subVectors(posPinkyMCP, posWrist);
		const palmNormal = new THREE.Vector3().crossVectors(palmVec1, palmVec2).normalize();
		//*/

		// 3) Compute “forward” (wrist → thumbTip) and palm normal for thumb tracking version:
		/*
		const forwardVec = new THREE.Vector3().subVectors(posThumbTip, posWrist).normalize();
		const palmVec1 = new THREE.Vector3().subVectors(posThumbMCP, posWrist);
		const palmVec2 = new THREE.Vector3().subVectors(posPinkyMCP, posWrist);
		const palmNormal = new THREE.Vector3().crossVectors(palmVec1, palmVec2).normalize();
		*/

		// 4) Build an orthonormal basis (axisX, axisY, axisZ):
		const axisY = forwardVec.clone();
		const axisZ = palmNormal.clone();
		const axisX = new THREE.Vector3().crossVectors(axisY, axisZ).normalize();
		// Re-orthogonalize axisZ = axisX × axisY in case of tiny drift
		axisZ.copy(new THREE.Vector3().crossVectors(axisX, axisY).normalize());

		// 5) Convert that basis → single quaternion (pitch, roll, yaw all encoded):
		const basisMat = new THREE.Matrix4().makeBasis(axisX, axisY, axisZ);
		const targetQuat = new THREE.Quaternion().setFromRotationMatrix(basisMat);

		// 6)Apply quaternion:
		//blade.quaternion.copy(targetQuat);
		blade.quaternion.slerp(targetQuat, SMOOTHING); // slerp for smoothness
	}
	// ────────────────────────────────────────────────────────────────────────────────────────────────────────────────────

	// Compute blade velocity, but ignore any jump above a threshold
	const deltaSec = clock.getDelta();
	const handPos = blade.position.clone();

	if (window.diag.prevHandPos && deltaSec > 0) {
		const dist = handPos.distanceTo(window.diag.prevHandPos);
		const vel = dist / deltaSec;  // raw velocity

		// Suppose you’ve determined that anything above 10 units/sec is impossible by hand:
		const MAX_PLAUSIBLE_VEL = 10000000000.0;

		if (vel <= MAX_PLAUSIBLE_VEL) {
			if (vel > window.diag.maxHandVel) {
				window.diag.maxHandVel = vel/100000;//convert to meters/sec
			}
		}
	}
	window.diag.prevHandPos = handPos;
	// ──────────────────────────────────────────────────────────────

	///*blade contact section -----collision detection pre-diagnostic implementation--------------	
	// build a bounding box for the blade once per frame
	/*
	const bladeBB = new THREE.Box3().setFromObject(blade);
	
	// for each teapot, test intersection and mark a .isHit flag
	teapots.forEach(tp => {
		const teapotBB = new THREE.Box3().setFromCenterAndSize(
			tp.position,
			new THREE.Vector3(200, 200, 200)  // adjust to your teapot’s roughly size
		);
		tp.isHit = bladeBB.intersectsBox(teapotBB);
	});
	*/
	///*
	const bladeBB = new THREE.Box3().setFromObject(blade);
	if (!window.diag.prevHitStates) {
		window.diag.prevHitStates = window.teapots.map(() => false);
	}
	window.teapots.forEach((tp, i) => {
		const teapotBB = new THREE.Box3().setFromCenterAndSize(
			tp.position,
			new THREE.Vector3(100, 100, 100)
		);
		const isNowHit = bladeBB.intersectsBox(teapotBB);
		tp.isHit = isNowHit;

		if (isNowHit && !window.diag.prevHitStates[i]) {
			window.diag.hitsPerTeapot[i]++;
			window.diag.collisionCount++;
		}
		window.diag.prevHitStates[i] = isNowHit;
	});
	// ──────────────────────────────────────────────────────────────
	//*/

	//end sutyazz addition -----------------------------------------------

	// update model/view/projection matrices
	mat.update(sc.state);

	if (renderingMode === STEREO_MODE) {

		if (webglRenderer.autoClear) webglRenderer.clear();

		webglRenderer.setScissorTest(true);

		// Render for the left eye on the left viewport
		webglRenderer.setScissor(
			0, 0, dispParams.canvasWidth / 2, dispParams.canvasHeight);

		webglRenderer.setViewport(
			0, 0, dispParams.canvasWidth / 2, dispParams.canvasHeight);

		standardRenderer.render(
			sc.state, mat.modelMat, mat.stereoViewMat.L, mat.stereoProjectionMat.L);

		// Render for the right eye on the right viewport
		webglRenderer.setScissor(
			dispParams.canvasWidth / 2, 0,
			dispParams.canvasWidth / 2, dispParams.canvasHeight);

		webglRenderer.setViewport(
			dispParams.canvasWidth / 2, 0,
			dispParams.canvasWidth / 2, dispParams.canvasHeight);

		standardRenderer.render(
			sc.state, mat.modelMat, mat.stereoViewMat.R, mat.stereoProjectionMat.R);

		webglRenderer.setScissorTest(false);

	} else if (renderingMode === STEREO_UNWARP_MODE) {

		// Render for the left eye on frame buffer object
		standardRenderer.renderOnTarget(stereoUnwarpRenderer.renderTargetL,
			sc.state, mat.modelMat, mat.stereoViewMat.L, mat.stereoProjectionMat.L);

		// Render for the right eye on frame buffer object
		standardRenderer.renderOnTarget(stereoUnwarpRenderer.renderTargetR,
			sc.state, mat.modelMat, mat.stereoViewMat.R, mat.stereoProjectionMat.R);

		stereoUnwarpRenderer.render(sc.state);



	}

	// End performance monitoring
	stats.end();

	// Display parameters used for rendering.
	sc.display();

}

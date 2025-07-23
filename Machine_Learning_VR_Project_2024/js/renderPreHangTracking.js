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


// add teapots with different shaders
/*var teapots = [];

var teapot1 =
	new Teapot( new THREE.Vector3( - 500, 0, 0 ),
		$( "#vShaderMultiPhong" ).text(),
		$( "#fShaderMultiPhong" ).text() );

teapots.push( teapot1 );

var teapot2 =
	new Teapot( new THREE.Vector3( 0, - 350, 100 ),
		$( "#vShaderMultiPhong" ).text(),
		$( "#fShaderMultiPhong" ).text() );

teapots.push( teapot2 );


var teapot3 =
	new Teapot( new THREE.Vector3( 500, - 200, - 130 ),
		$( "#vShaderMultiPhong" ).text(),
		$( "#fShaderMultiPhong" ).text() );

teapots.push( teapot3 );

var teapot4 =
	new Teapot( new THREE.Vector3( 0, 300, - 200 ),
		$( "#vShaderMultiPhong" ).text(),
		$( "#fShaderMultiPhong" ).text() );

teapots.push( teapot4 );*/

//SUTYAZZ ADDITION
// --------------------------------------------
// STATIC teapots (optional)
var teapots = [];
/*var staticPositions = [
  new THREE.Vector3(-500, 0, 0),
  new THREE.Vector3(0, -350, 100),
  new THREE.Vector3(500, -200, -130),
  new THREE.Vector3(0, 300, -200)
];
staticPositions.forEach(pos => {
  const tp = new Teapot(pos,
	$("#vShaderMultiPhong").text(),
	$("#fShaderMultiPhong").text() );
  teapots.push(tp);
});*/

// --------------------------------------------
// FLYING teapots
// parameters
const NUM_FLYERS = 3;
const START_Z = -1000;    // spawn this far “in front” of viewer
const RESET_Z = 2000;     // once they pass closer than this, reset
const SPEED = 200;    // mm per second toward viewer

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
const bladeWidth = 10;      // thickness
const bladeDepth = 1;       // slim edge
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

// set once, before animation loop
const BLADE_Z = 1400;
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
//standardRenderer.teapotScene.add( corridor );

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
				tp.position.z = START_Z - Math.random() * 3000; //sutyazz addition commented out the random z start
			}
		}
	});

	//sutyazz addition -----------------------------------------------

	// ── hover & rock blade ──
	//blade.position.y = Math.sin(t * 2) * 50;     // up/down
	//blade.rotation.z = Math.sin(t * 0.5) * 0.2;  // roll

	// how far in front of the eyes
	const BLADE_DISTANCE = 400;

	// pull in your raw head pose
	const headPos = sc.state.viewerPosition;    // world-space eye pos :contentReference[oaicite:0]{index=0}
	const headQuat = sc.state.imuQuaternion;     // IMU quaternion :contentReference[oaicite:1]{index=1}

	// correct the IMU→world flip exactly as your view-matrix does
	const flipY = new THREE.Quaternion(0, 1, 0, 0);
	const flipYinv = flipY.clone().invert();
	const worldQuat = headQuat.clone()
		.premultiply(flipYinv)
		.multiply(flipY);

	// compute forward in world coords from that corrected quat
	const forward = new THREE.Vector3(0, 0, -1)
		.applyQuaternion(worldQuat)
		.normalize();

	// 1) position the blade in front of the head
	blade.position.copy(headPos)
		.addScaledVector(forward, BLADE_DISTANCE);

	// 2) base orientation = your head’s world-space orientation
	blade.quaternion.copy(worldQuat);

	// 3) now spin around the blade’s own forward axis
	//    e.g. use your baseStationRoll from the Teensy:
	const rollAngle = sc.state.baseStationRoll;   // in radians :contentReference[oaicite:2]{index=2}
	const rollQuat = new THREE.Quaternion().setFromAxisAngle(forward, rollAngle);

	// 4) apply that roll on top
	blade.quaternion.multiply(rollQuat);




	///*blade contact section -----------------------------------------	
	// build a bounding box for the blade once per frame
	const bladeBB = new THREE.Box3().setFromObject(blade);

	// for each teapot, test intersection and mark a .isHit flag
	teapots.forEach(tp => {
		const teapotBB = new THREE.Box3().setFromCenterAndSize(
			tp.position,
			new THREE.Vector3(100, 100, 100)  // adjust to your teapot’s roughly size
		);
		tp.isHit = bladeBB.intersectsBox(teapotBB);
	});

	
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

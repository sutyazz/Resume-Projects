/**
 * @file Serial to WebSocket communication
 * This program achieve the serial communication between Arduino and browsers
 * through WebSocket connections. It allows multiple/non WebSocket connections.
 *
 * Please change the port name and also the port number for your environment.
 *
 * This code is based on the article written by Prof. Tom Igoe at
 * NYU Tische School of the Arts. The code example is copied and modified to
 * achieve the communication between Arduino and browsers with the permission
 * of Prof. Igoe.
 *
 * Reference:
 * https://itp.nyu.edu/physcomp/labs/labs-serial-communication/lab-serial-communication-with-node-js/
 *
 * @copyright The Board of Trustees of the Leland Stanford Junior University
 * @version 2022/04/28
 *
 */


// ─────────── 1) EXPRESS STATIC SERVER ───────────

// Then open in your browser:
// http://localhost:3000/render.html
const path = require("path");
const express = require("express");
const { SerialPort, ReadlineParser } = require("serialport");
const WebSocketServer = require("ws").Server;

// ─────────── 1) EXPRESS STATIC SERVER ───────────

// Create an Express app
const app = express();

// Because server.js lives in "server/", __dirname is ".../your-project/server".
// We want to serve files from the parent directory (project root), where render.html actually is.
const PUBLIC_DIR = path.join(__dirname, "..");

// Tell Express to serve everything under PUBLIC_DIR
app.use(express.static(PUBLIC_DIR));

const HTTP_PORT = 3000;
app.listen(HTTP_PORT, () => {
  console.log(`✅ HTTP server running. Open http://localhost:${HTTP_PORT}/render.html`);
});


// ─────────── 2) SERIAL‐to‐WEBSOCKET BRIDGE ───────────


// Import serialport library
//const { SerialPort, ReadlineParser } = require('serialport')

// Import WebSocketServer
//const WebSocketServer = require( "ws" ).Server;

// Instantiate the WebSocket server with port 8081
const wss = new WebSocketServer( { port: 8081 } );


// List of WebSocket connections
// It is useful to use it as a contanair even if we assume 1 WebSocket
// connection at most because we don't want to close the serial port even when
// we don't have any WebSocket connections.
var wssConnections = [];


// Toggle switch for printing out the data
var printData = true;


// Event listner of the WebSocketServer.
wss.on( "connection", function ( client ) {

	console.log( "The browser is connected to the serial port." );

	wssConnections.push( client );

	client.on( "close", function () {

		console.log( "The connection to the browser is closed." );

		var idx = wssConnections.indexOf( client );

		wssConnections.splice( idx, 1 );

	} );

} );



// Keyboard input to Arduino through stdin.
// By setting it to be raw mode, the data is sent without hitting enter.
var stdin = process.openStdin();

stdin.setRawMode( true );

stdin.setEncoding( "utf8" );

stdin.on( "data", function ( key ) {

	if ( key === '\u0003' ) {

		process.exit();

	}

} );

findSerialPort();



function findSerialPort() {

	console.log( "Looking for Teensy..." );

	var connected = false;

	SerialPort.list().then(
		ports => ports.forEach(
			function ( port ) {

				if (( port.manufacturer == "Teensyduino" ||
					port.manufacturer == "Microsoft" ) && !port.friendlyName.includes("Bluetooth") )  {

					console.log( "Teensy found!" );

					setupSerialPort( port.path );

					connected = true;

				}

			}
		),
		err => console.error( err )
	).then(
		function () {

			if ( ! connected ) {

				console.log( "Teensy not found..." );

				//try to reconnect in 1 s
				setTimeout( findSerialPort, 1000 );

			}

		}
	);

}

function setupSerialPort( portName ) {

	// Instantiate SerialPort
	const parser = new ReadlineParser();

	const serialPort = new SerialPort( {
        
        path: portName,
        baudRate: 115200

    } );

	serialPort.pipe( parser );

	// Set up event listeners on the serial port
	serialPort.on( "open", function () {

		console.log( "The serial port to Teensy is opened." );

	} );

	serialPort.on( "close", function () {

		console.log( "The serial port to Teensy is closed." );

		//try to reconnect in 1 s
		setTimeout( findSerialPort, 1000 );

	} );

	serialPort.on( "error", function ( err ) {

		console.log( "Serial port error: " + err );

	} );

	// transmit serial port data though the web socket server
	parser.on( "data", function ( data ) {

		if ( printData ) {

			console.log( data );

		}

		wssConnections.forEach( function ( socket ) {

			socket.send( data, function ( err ) { } );

		} );

	} );


	stdin.on( "data", function ( key ) {

		if ( key === '\u0003' ) {

			process.exit();

		}

		if ( key === 'd' ) {

			printData = ! printData;

			if ( printData ) {

				console.log( "Start printing data..." );

			} else {

				console.log( "Stop printing data... (Still streaming data to WebSocket)" );

			}

		}

		serialPort.write( key );

	} );

}

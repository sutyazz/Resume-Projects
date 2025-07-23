#include <Arduino.h>
#include <SPI.h>
#include <Arduino_LSM6DSOX.h>

#if defined(ARDUINO_ARCH_AVR)
  #define PinStatus int
#endif
#include <WiFiNINA.h>

// WiFi AP credentials
const char* ap_ssid = "RectangleTracker";     
const char* ap_pass = "12345678";             

WiFiServer server(80);

// Position tracking variables 
float x_pos = 300, y_pos = 150;
float x_vel = 0, y_vel = 0;
const float damping = 0.95;
const float sensitivity = 50.0;

// Time variables
unsigned long lastUpdate = 0;
const float dt = 0.1;

// Debug variables
float current_ax = 0, current_ay = 0, current_gz = 0;

// Store webpage in program memory
const char html_header[] PROGMEM = R"=====(
HTTP/1.1 200 OK
Content-Type: text/html
Connection: close

<!DOCTYPE html><html><head><meta name='viewport'content='width=device-width,initial-scale=1'>
<style>
body{text-align:center;font-family:Arial}
svg{width:600px;height:300px}
#s{fill:red}#d{fill:green}
.o{fill:rgba(255,0,0,0.5);stroke:black}
.g{fill:green}
</style></head><body>
)=====";

const char html_body[] PROGMEM = R"=====(
<svg viewBox='0 0 600 300'>
<rect x='30'y='140'width='20'height='20'class='g'/>
<circle id='d'cx='300'cy='50'r='10'/>
<rect x='50'y='50'width='500'height='200'fill='none'stroke='black'stroke-width='5'/>
<rect x='150'y='210'width='300'height='40'class='o'/>
<rect id='s'x='50'y='250'width='20'height='20'/>
</svg>
<p id='i'></p>
<p id='debug'></p>
)=====";

const char html_script[] PROGMEM = R"=====(
<script>
async function u(){
    try {
        const response = await fetch('/coords');
        if (!response.ok) throw new Error('Network response not ok');
        const text = await response.text();
        try {
            const d = JSON.parse(text);
            const s = document.getElementById('s');
            const x = Math.max(50,Math.min(550,d.x));
            const y = Math.max(50,Math.min(250,d.y));
            s.setAttribute('transform',`translate(${x} ${y}) rotate(${d.h} ${x+10} ${y+10})`);
            document.getElementById('d').setAttribute('cx',d.dx);
            document.getElementById('i').innerText = `Position: (${x|0},${y|0}) | Heading: ${d.h|0}°`;
            document.getElementById('debug').innerText = `Accel: X:${d.ax.toFixed(2)} Y:${d.ay.toFixed(2)} | Gyro Z:${d.gz.toFixed(2)}`;
        } catch (parseError) {
            console.error('Parse error:', text);
        }
    } catch(e) {
        console.error('Fetch error:', e);
    }
}
setInterval(u, 100);
</script></body></html>
)=====";

void updatePosition(float ax, float ay, float gz) {
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastUpdate) / 1000.0;
    
    if (deltaTime >= 0.05) {
        current_ax = ax;
        current_ay = ay;
        current_gz = gz;

        ax *= sensitivity;
        ay *= sensitivity;

        x_vel += ax;
        y_vel += ay;
        x_vel *= damping;
        y_vel *= damping;

        x_pos += x_vel * deltaTime;
        y_pos += y_vel * deltaTime;
        
        x_pos = constrain(x_pos, 50, 550);
        y_pos = constrain(y_pos, 50, 250);

        lastUpdate = currentTime;
    }
}

void handleClient(WiFiClient& client) {
    String currentLine = "";
    String requestLine = "";
    unsigned long timeout = millis();
    
    while (client.connected() && millis() - timeout < 1000) {
        if (client.available()) {
            char c = client.read();
            if (c == '\n') {
                // If this is the first line, save it as the request
                if (requestLine.length() == 0) {
                    requestLine = currentLine;
                }
                
                // If we get a blank line, we're done with headers
                if (currentLine.length() == 0) {
                    if (requestLine.indexOf("GET /coords") >= 0) {
                        // Send coords JSON response
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-Type: application/json"));
                        client.println(F("Access-Control-Allow-Origin: *"));
                        client.println(F("Connection: close"));
                        client.println();
                        
                        // Create JSON without any HTML
                        client.print("{\"x\":");
                        client.print(x_pos);
                        client.print(",\"y\":");
                        client.print(y_pos);
                        client.print(",\"h\":0");
                        client.print(",\"dx\":300");
                        client.print(",\"ax\":");
                        client.print(current_ax);
                        client.print(",\"ay\":");
                        client.print(current_ay);
                        client.print(",\"gz\":");
                        client.print(current_gz);
                        client.print("}");
                    } else if (requestLine.indexOf("GET /") >= 0) {
                        // Send HTML webpage
                        sendProgmem(client, html_header);
                        sendProgmem(client, html_body);
                        sendProgmem(client, html_script);
                    }
                    break;
                }
                currentLine = "";
            } else if (c != '\r') {
                currentLine += c;
            }
        }
    }
    delay(1);
    client.stop();
}

void sendProgmem(WiFiClient& client, const char* data) {
    char chunk[64];
    int len = strlen_P(data);
    for(int i = 0; i < len; i += sizeof(chunk)-1) {
        int chunkLen = min(sizeof(chunk)-1, len - i);
        memcpy_P(chunk, data + i, chunkLen);
        chunk[chunkLen] = 0;
        client.print(chunk);
    }
}

void setup() {
    Serial.begin(115200);
    
    SPI.begin();
    if (!IMU.begin()) {
        Serial.println("IMU failed!");
        while (1);
    }

    WiFi.beginAP(ap_ssid, ap_pass);
    server.begin();
    Serial.print("AP IP: ");
    Serial.println(WiFi.localIP());
    
    lastUpdate = millis();
}

void loop() {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
        float ax, ay, az, gx, gy, gz;
        IMU.readAcceleration(ax, ay, az);
        IMU.readGyroscope(gx, gy, gz);
        updatePosition(ax, ay, gz);
    }

    WiFiClient client = server.available();
    if (client) {
        handleClient(client);
    }
}
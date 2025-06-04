#include "MPU6050_VisualLib.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;  
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long gyroDelay = 10;
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 200;

// Create a sensor object
Adafruit_MPU6050 mpu;

sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;


//Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;

// Embedded HTML with CSS and JS inside string literals
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" 
    integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/107/three.min.js"></script>
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
    }
    p {
      font-size: 1.2rem;
    }
    body {
      margin: 0;
    }
    .topnav {
      overflow: hidden;
      background-color: #003366;
      color: #FFD43B;
      font-size: 1rem;
    }
    .content {
      padding: 20px;
    }
    .card {
      background-color: white;
      box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
      margin-bottom: 20px;
    }
    .card-title {
      color:#003366;
      font-weight: bold;
      margin-top: 10px;
      margin-bottom: 10px;
    }
    .cards {
      max-width: 800px;
      margin: 0 auto;
      display: grid; 
      grid-gap: 2rem;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    }
    .reading {
      font-size: 1.2rem;
    }
    .cube-content{
      width: 100%;
      background-color: white;
      height: 300px; 
      margin: auto;
      padding-top:2%;
    }
    #reset {
      border: none;
      color: #FEFCFB;
      background-color: #003366;
      padding: 10px;
      text-align: center;
      display: inline-block;
      font-size: 14px; 
      width: 150px;
      border-radius: 4px;
      cursor: pointer;
    }
    #resetX, #resetY, #resetZ {
      border: none;
      color: #FEFCFB;
      background-color: #003366;
      padding-top: 10px;
      padding-bottom: 10px;
      text-align: center;
      display: inline-block;
      font-size: 14px;
      width: 20px;
      border-radius: 4px;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <div class="topnav">
    <h1><i class="far fa-compass"></i> MPU6050 <i class="far fa-compass"></i></h1>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <p class="card-title">GYROSCOPE</p>
        <p><span class="reading">X: <span id="gyroX"></span> rad</span></p>
        <p><span class="reading">Y: <span id="gyroY"></span> rad</span></p>
        <p><span class="reading">Z: <span id="gyroZ"></span> rad</span></p>
      </div>
      <div class="card">
        <p class="card-title">ACCELEROMETER</p>
        <p><span class="reading">X: <span id="accX"></span> ms<sup>2</sup></span></p>
        <p><span class="reading">Y: <span id="accY"></span> ms<sup>2</sup></span></p>
        <p><span class="reading">Z: <span id="accZ"></span> ms<sup>2</sup></span></p>
      </div>
      <div class="card">
        <p class="card-title">TEMPERATURE</p>
        <p><span class="reading"><span id="temp"></span> &deg;C</span></p>
        <p class="card-title">3D ANIMATION</p>
        <button id="reset" onclick="resetPosition(this)">RESET POSITION</button>
        <button id="resetX" onclick="resetPosition(this)">X</button>
        <button id="resetY" onclick="resetPosition(this)">Y</button>
        <button id="resetZ" onclick="resetPosition(this)">Z</button>
      </div>
    </div>
    <div class="cube-content">
      <div id="3Dcube"></div>
    </div>
  </div>

  <script>
    let scene, camera, renderer, cube;

    function parentWidth(elem) {
      return elem.parentElement.clientWidth;
    }

    function parentHeight(elem) {
      return elem.parentElement.clientHeight;
    }

    function init3D(){
      scene = new THREE.Scene();
      scene.background = new THREE.Color(0xffffff);

      camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 0.1, 1000);

      renderer = new THREE.WebGLRenderer({ antialias: true });
      renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

      document.getElementById('3Dcube').appendChild(renderer.domElement);

      // Create a geometry
      const geometry = new THREE.BoxGeometry(5, 1, 4);

      // Materials of each face
      var cubeMaterials = [
        new THREE.MeshBasicMaterial({color:0x03045e}),
        new THREE.MeshBasicMaterial({color:0x023e8a}),
        new THREE.MeshBasicMaterial({color:0x0077b6}),
        new THREE.MeshBasicMaterial({color:0x03045e}),
        new THREE.MeshBasicMaterial({color:0x023e8a}),
        new THREE.MeshBasicMaterial({color:0x0077b6}),
      ];

      const material = new THREE.MeshFaceMaterial(cubeMaterials);

      cube = new THREE.Mesh(geometry, material);
      scene.add(cube);
      camera.position.z = 5;
      renderer.render(scene, camera);
    }

    // Resize the 3D object when the browser window changes size
    function onWindowResize(){
      camera.aspect = parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube"));
      camera.updateProjectionMatrix();
      renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));
    }

    window.addEventListener('resize', onWindowResize, false);

    // Create the 3D representation
    init3D();

    // Setup EventSource
    if (!!window.EventSource) {
      var source = new EventSource('/events');

      source.addEventListener('open', function(e) {
        console.log("Events Connected");
      }, false);

      source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Events Disconnected");
        }
      }, false);

      source.addEventListener('gyro_readings', function(e) {
        var obj = JSON.parse(e.data);
        document.getElementById("gyroX").innerHTML = obj.gyroX;
        document.getElementById("gyroY").innerHTML = obj.gyroY;
        document.getElementById("gyroZ").innerHTML = obj.gyroZ;

        // Update cube rotation
        cube.rotation.x = obj.gyroY;
        cube.rotation.z = obj.gyroX;
        cube.rotation.y = obj.gyroZ;
        renderer.render(scene, camera);
      }, false);

      source.addEventListener('temperature_reading', function(e) {
        document.getElementById("temp").innerHTML = e.data;
      }, false);

      source.addEventListener('accelerometer_readings', function(e) {
        var obj = JSON.parse(e.data);
        document.getElementById("accX").innerHTML = obj.accX;
        document.getElementById("accY").innerHTML = obj.accY;
        document.getElementById("accZ").innerHTML = obj.accZ;
      }, false);
    }

    function resetPosition(element){
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/"+element.id, true);
      console.log(element.id);
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

// Function to send headers for SSE and start the connection
void MPU6050_VisualLib::initMPU() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}

// Call this periodically to send new data if client connected
void MPU6050_VisualLib::initWiFi(char* ssid , char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());


  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroX=0;
    gyroY=0;
    gyroZ=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroX=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroY=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroZ=0;
    request->send(200, "text/plain", "OK");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  server.begin();

}

String MPU6050_VisualLib::getAccReadings() {
  mpu.getEvent(&a, &g, &temp);
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
  readings["accX"] = String(accX);
  readings["accY"] = String(accY);
  readings["accZ"] = String(accZ);
  String accString = JSON.stringify (readings);
  return accString;
}

String MPU6050_VisualLib::getGyroReadings() {
  mpu.getEvent(&a, &g, &temp);

  float gyroX_temp = g.gyro.x;
  if(abs(gyroX_temp) > gyroXerror)  {
    gyroX += gyroX_temp/50.00;
  }
  
  float gyroY_temp = g.gyro.y;
  if(abs(gyroY_temp) > gyroYerror) {
    gyroY += gyroY_temp/70.00;
  }

  float gyroZ_temp = g.gyro.z;
  if(abs(gyroZ_temp) > gyroZerror) {
    gyroZ += gyroZ_temp/90.00;
  }

  readings["gyroX"] = String(gyroX);
  readings["gyroY"] = String(gyroY);
  readings["gyroZ"] = String(gyroZ);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}


String MPU6050_VisualLib::getTemperature() {
  mpu.getEvent(&a, &g, &temp);
  temperature = temp.temperature;
  return String(temperature);
}

void MPU6050_VisualLib::getVisualization() {
  if ((millis() - lastTime) > gyroDelay) {
    events.send(getGyroReadings().c_str(),"gyro_readings",millis());
    lastTime = millis();
  }
  if ((millis() - lastTimeAcc) > accelerometerDelay) {
    events.send(getAccReadings().c_str(),"accelerometer_readings",millis());
    lastTimeAcc = millis();
  }
  if ((millis() - lastTimeTemperature) > temperatureDelay) {
    events.send(getTemperature().c_str(),"temperature_reading",millis());
    lastTimeTemperature = millis();
  }
}
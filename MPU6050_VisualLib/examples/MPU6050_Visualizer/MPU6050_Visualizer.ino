#include <MPU6050_VisualLib.h>

// Create object of the library
MPU6050_VisualLib mpuVisual;

// Your Wi-Fi credentials
char ssid[] = "Vivo Y30";
char password[] = "123456789";

void setup() {
  Serial.begin(115200);
  
  // Initialize MPU6050
  mpuVisual.initMPU();
  
  // Connect to Wi-Fi and setup web server + SSE
  mpuVisual.initWiFi(ssid, password);
}

void loop() {
  // Continuously send data over SSE
  mpuVisual.getVisualization();

  // Optional: Delay for smoother loop
  delay(10);
}

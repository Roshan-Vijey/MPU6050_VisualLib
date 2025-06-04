# MPU6050_VisualLib

MPU6050_VisualLib is an Arduino library for ESP32 that visualizes data from the MPU6050 sensor via SSE (Server-Sent Events) in a web browser using Three.js for 3D rendering.

## Features
- Gyroscope data visualization
- Accelerometer readings
- Temperature monitoring
- Real-time web interface using Three.js

## Dependencies
- ESPAsyncWebServer
- AsyncTCP
- Adafruit_MPU6050
- Arduino_JSON

## Installation
1. Clone/download this repo.
2. Install required dependencies through Arduino Library Manager or manually.
3. Place this folder into your `libraries/` folder.

## Example
An example sketch is provided in the `examples/MPU6050_Visualization_Example/` directory.

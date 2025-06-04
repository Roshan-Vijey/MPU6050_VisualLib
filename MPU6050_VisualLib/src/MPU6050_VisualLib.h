#ifndef MPU6050_VISUAL_LIB_H
#define MPU6050_VISUAL_LIB_H

#include <Arduino.h>

class MPU6050_VisualLib {
public:
  // Initialize MPU6050 and server
  void initMPU();

  // Read acceleration values into x, y, z
  String getAccReadings();

  // Read rotation values into x, y, z
  String getGyroReadings();

  // Read and return temperature in Celsius
  String getTemperature();

  // Handle server and stream sensor values via SSE
  void getVisualization();

  void initWiFi(char* ssid , char* password);
};

#endif

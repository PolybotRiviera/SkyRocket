#include <HardwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include "USB.h"

#define LIDAR_BAUD_RATE 230400  // Baud rate for the LiDAR
#define BUFFER_SIZE 47          // Total bytes to read (including header)
#define PIN_LED 21
#define PIN_LIDAR 18

Adafruit_NeoPixel pixels(1, PIN_LED, NEO_GRB + NEO_KHZ800);
USBCDC USBSerial;
HardwareSerial SerialLidar(1);

int minimalDistanceAuthorized = 150;
bool securityStop = false;

int minDistanceIndex = 0;
int minIntensityIndex = 0;
uint16_t minDistance = UINT16_MAX;  // Initialize with the maximum possible value
byte minIntensity = 255;            // Initialize with the maximum possible value

unsigned long previousMillis = 0;
const long interval = 2000;  // 2 seconds
int angleBeginning = 0;
int angleEnding = 0;
int angleSelector = 1;

static const uint8_t CrcTable[256] = {
  0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25,
  0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07,
  0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8, 0xf5, 0x1f, 0x52, 0x85, 0xc8,
  0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77, 0x3a, 0x94, 0xd9, 0x0e, 0x43,
  0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55, 0x18, 0x44, 0x09, 0xde, 0x93,
  0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4, 0xe9, 0x47, 0x0a, 0xdd, 0x90,
  0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f, 0x62, 0x97, 0xda, 0x0d, 0x40,
  0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff, 0xb2, 0x1c, 0x51, 0x86, 0xcb,
  0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2, 0x8f, 0xd3, 0x9e, 0x49, 0x04,
  0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12, 0x5f, 0xf1, 0xbc, 0x6b, 0x26,
  0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99, 0xd4, 0x7c, 0x31, 0xe6, 0xab,
  0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14, 0x59, 0xf7, 0xba, 0x6d, 0x20,
  0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36, 0x7b, 0x27, 0x6a, 0xbd, 0xf0,
  0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9, 0xb4, 0x1a, 0x57, 0x80, 0xcd,
  0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72, 0x3f, 0xca, 0x87, 0x50, 0x1d,
  0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2, 0xef, 0x41, 0x0c, 0xdb, 0x96,
  0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1, 0xec, 0xb0, 0xfd, 0x2a, 0x67,
  0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71, 0x3c, 0x92, 0xdf, 0x08, 0x45,
  0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa, 0xb7, 0x5d, 0x10, 0xc7, 0x8a,
  0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35, 0x78, 0xd6, 0x9b, 0x4c, 0x01,
  0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1,
  0x7f, 0x32, 0xe5, 0xa8
};


uint8_t CalCRC8(uint8_t *p, uint8_t len) {
  uint8_t crc = 0;
  uint16_t i;
  for (i = 0; i < len; i++) {
    crc = CrcTable[(crc ^ *p++) & 0xff];
  }
  return crc;
}

void setup() {
  Serial.begin(115200);                                    // Debugging output to your computer
  SerialLidar.begin(LIDAR_BAUD_RATE, SERIAL_8N1, PIN_LIDAR, -1);  // GPIO11 as RX, TX not used

  USBSerial.begin();
  USB.begin();
  USBSerial.println("ESP32-WROOM-32 LiDAR raw data processing started..");

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(10);
  colorPixel(0, 0, 255);

  delay(2000);

  colorPixel(0, 255, 0);
}

void loop() {
  securityStop = false;
  if (SerialLidar.available() < BUFFER_SIZE * 2) {
    return;  // Wait for more data
  }

  byte buffer[BUFFER_SIZE];
  // Synchronization: Look for the header (0x54, 0x2C)
  while (SerialLidar.available() >= BUFFER_SIZE) {
    // Read one byte at a time to search for the header
    if (SerialLidar.read() == 0x54) {    // Check for the first byte of the header
      if (SerialLidar.peek() == 0x2C) {  // Check for the second byte of the header
        SerialLidar.read();              // Consume the second byte of the header
        // Add the header back into the buffer manually
        buffer[0] = 0x54;
        buffer[1] = 0x2C;
        break;  // Exit the synchronization loop
      }
    }
  }

  // Read the remaining 43 bytes of the packet
  size_t bytesRead = SerialLidar.readBytes(buffer + 2, BUFFER_SIZE - 2);

  if (bytesRead != BUFFER_SIZE - 2) {
    USBSerial.println("Error: Incomplete packet received.");
    return;
  }
  // Print the entire packet in hexadecimal format
  USBSerial.println("New Packet");

  uint8_t calculatedCRC = CalCRC8(buffer, BUFFER_SIZE - 1);
  if (calculatedCRC == buffer[BUFFER_SIZE - 1]) {
    byte verLen = buffer[1];
    if (verLen != 0x2C) {
      USBSerial.println("Error: Invalid VerLen field. Expected 0x2C.");
      return;
    }

    // Speed (2 bytes)
    uint16_t speed = (buffer[3] << 8) | buffer[2];  // Speed is 2 bytes

    // Start Angle (2 bytes)
    uint16_t startAngle = (buffer[5] << 8) | buffer[4];
    USBSerial.print("Start Angle: ");
    USBSerial.print(startAngle * 0.01);  // Convert to degrees
    USBSerial.println(" degrees");

    // Data points (12 points, each point 3 bytes)
    minDistanceIndex = 0;
    minIntensityIndex = 0;
    minDistance = UINT16_MAX;  // Initialize with the maximum possible value
    minIntensity = 255;        // Initialize with the maximum possible value

    for (int i = 0; i < 12; i++) {
      int offset = 6 + i * 3;
      uint16_t distance = (buffer[offset + 1] << 8) | buffer[offset];  // 2 bytes for distance
      byte intensity = buffer[offset + 2];                             // 1 byte for intensity

      // Check for the minimum distance
      if (intensity >= 5 && distance < minDistance) {
        minDistance = distance;
        minDistanceIndex = i;
      }
    }

    // Print the point with the lowest distance
    USBSerial.print("Point ");
    USBSerial.print(minDistanceIndex + 1);
    USBSerial.print(" lowest Distance: ");
    USBSerial.print(minDistance);
    USBSerial.println(" mm");

    // End Angle (2 bytes)
    uint16_t endAngle = (buffer[43] << 8) | buffer[42];
    USBSerial.print("End Angle: ");
    USBSerial.print(endAngle * 0.01);  // Convert to degrees
    USBSerial.println(" degrees");

    // Timestamp (2 bytes)
    uint16_t timestamp = (buffer[45] << 8) | buffer[44];

    // CRC Check (1 byte)
    byte crc = buffer[46];

    USBSerial.println();
    USBSerial.println();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      angleSelector++;
      if (angleSelector > 4) {
        angleSelector = 1;
      }
    }

    USBSerial.println(angleSelector);

    switch (angleSelector) {
      case 1:
        colorPixel(255, 255, 0);
        angleBeginning = 0;
        angleEnding = angleBeginning + 90;
        break;
      case 2:
        colorPixel(255, 0, 255);
        angleBeginning = 90;
        angleEnding = angleBeginning + 90;
        break;
      case 3:
        colorPixel(0, 255, 255);
        angleBeginning = 180;
        angleEnding = angleBeginning + 90;
        break;
      case 4:
        colorPixel(255, 255, 255);
        angleBeginning = 270;
        angleEnding = angleBeginning + 90;
        break;
    }

    if (startAngle * 0.01 >= angleBeginning && endAngle * 0.01 <= angleEnding) {
      if (minDistance < minimalDistanceAuthorized) {
        colorPixel(255, 0, 0);
        securityStop = true;
        delay(10000);
      }
    }
  }

  else {
    USBSerial.println("CRC invalid. Discarding packet.");
  }
}

void colorPixel(int redColor, int greenColor, int blueColor) {
  pixels.setPixelColor(0, pixels.Color(redColor, greenColor, blueColor));
  pixels.show();
}
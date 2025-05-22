import processing.serial.*;
import java.util.ArrayDeque;  // For maintaining FIFO queue

Serial myPort;
String receivedData = "";
ArrayDeque<Point> points;  // FIFO queue for storing last 1000 points
int maxPoints = 1000;      // Maximum number of points to store
float zoomFactor = 1.0;    // Zoom scaling factor, starting at 1.0 (no zoom)
float zoomSpeed = 0.1;     // The sensitivity of zoom (change the step size if needed)
float panX = 0;            // Horizontal pan offset
float panY = 0;            // Vertical pan offset
float prevMouseX = 0;      // Previous mouse X position (for panning)
float prevMouseY = 0;      // Previous mouse Y position (for panning)
boolean isPanning = false; // Flag to check if the user is holding the left mouse button

void setup() {
  size(800, 800);
  background(0);
  noStroke();  // Ensure no outline around the points

  points = new ArrayDeque<Point>(maxPoints);  // Initialize the FIFO queue

  // Adjust the port name to match your Arduino port
  myPort = new Serial(this, Serial.list()[0], 115200);
  myPort.bufferUntil('\n');
}

void draw() {
  background(0);  // Clear the background each frame
  translate(width / 2 + panX, height / 2 + panY);  // Center the drawing and apply panning
  stroke(0, 255, 0);
  synchronized (points) {  // Ensure safe iteration
    for (Point p : points) {
      // Set color to green for points (no stroke, just green fill)
      fill(map(sqrt(pow(p.x, 2)+pow(p.y, 2)), 0, 120, 0, 255), 255, 0);  // Green color for points
      strokeWeight(0);
      ellipse(p.x * zoomFactor, p.y * zoomFactor, 5 * zoomFactor, 5 * zoomFactor);  // Apply zoom to point size and position
    }
  }

  // Display the red cross at the Lidar location (origin)
  drawLidarCross();
}

// Capture mouse wheel input to zoom in and out
void mouseWheel(MouseEvent event) {
  float delta = event.getCount();  // Get the direction of the wheel scroll
  zoomFactor += delta * zoomSpeed;  // Adjust the zoom factor
  zoomFactor = constrain(zoomFactor, 0.01, 50.0);  // Limit the zoom factor between 0.1 and 5.0
}

// Start panning when the left mouse button is pressed
void mousePressed() {
  if (mouseButton == LEFT) {
    isPanning = true;  // Enable panning
    prevMouseX = mouseX;  // Store initial mouse X position
    prevMouseY = mouseY;  // Store initial mouse Y position
  }
}

// Stop panning when the left mouse button is released
void mouseReleased() {
  if (mouseButton == LEFT) {
    isPanning = false;  // Disable panning
  }
}

// Update pan offsets while dragging the left mouse button
void mouseDragged() {
  if (isPanning) {
    // Calculate the change in mouse position
    float dx = mouseX - prevMouseX;
    float dy = mouseY - prevMouseY;

    // Update the pan offsets
    panX += dx / zoomFactor;  // Adjust for zoom factor
    panY += dy / zoomFactor;  // Adjust for zoom factor

    // Store the new mouse position for the next drag event
    prevMouseX = mouseX;
    prevMouseY = mouseY;
  }
}

void serialEvent(Serial myPort) {
  receivedData = myPort.readString().trim();

  // Parse and process the received data
  if (receivedData != "") {
    String[] newPoints = split(receivedData, ';');

    synchronized (points) {  // Ensure safe modification
      for (String point : newPoints) {
        if (point.length() > 0) {
          String[] parts = split(point, ':');
          if (parts.length == 3) {
            float angle = radians(float(parts[0]));
            float distance = float(parts[1]) * 0.1;  // Scale distance for visualization
            int intensity = int(parts[2]);

            float x = cos(angle) * distance;
            float y = sin(angle) * distance;

            // If the queue is full, remove the oldest point
            if (points.size() == maxPoints) {
              Point oldest = points.poll();
              // "Erase" the oldest point by redrawing it with the background color
              fill(0);
              ellipse(oldest.x*zoomFactor, oldest.y*zoomFactor, 5*zoomFactor, 5*zoomFactor);
            }

            // Add the new point to the queue
            points.add(new Point(x, y, intensity));
          }
        }
      }
    }
  }
  myPort.clear();
}

// Helper class to store point data
class Point {
  float x, y;
  int intensity;

  Point(float x, float y, int intensity) {
    this.x = x;
    this.y = y;
    this.intensity = intensity;
  }
}

// Function to draw the red cross at the Lidar location
void drawLidarCross() {
  float crossSize = 50 * zoomFactor;  // Adjust the size of the cross with the zoom factor

  // Set red color for the cross
  stroke(255, 0, 0);
  strokeWeight(2);  // Set the thickness of the lines

  // Draw the vertical line (Y axis)
  line(0, -crossSize, 0, crossSize);

  // Draw the horizontal line (X axis)
  line(-crossSize, 0, crossSize, 0);
  stroke(0, 0, 0);
}

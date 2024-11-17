#include <Arduino.h>
#include "base/graphics.hpp"
#include "base/n64controller.hpp"

#include <cmath>
#include <algorithm> // for std::swap
#include <vector>

struct Point3D {
    float x, y, z;
};

struct Point2D {
    int x, y;
};

struct Face {
    int v0, v1, v2; // Indices of vertices that form the face
    int color;          // Color index (0 to 15)
    int depth;
};

// Cube vertices (8 points for a unit cube centered at the origin)
Point3D vertices[] = {
    { -1.0f, -1.0f, -1.0f }, // 0
    {  1.0f, -1.0f, -1.0f }, // 1
    {  1.0f,  1.0f, -1.0f }, // 2
    { -1.0f,  1.0f, -1.0f }, // 3
    { -1.0f, -1.0f,  1.0f }, // 4
    {  1.0f, -1.0f,  1.0f }, // 5
    {  1.0f,  1.0f,  1.0f }, // 6
    { -1.0f,  1.0f,  1.0f }  // 7
};

Point3D oldVertices[] = {
    { -1.0f, -1.0f, -1.0f }, // 0
    {  1.0f, -1.0f, -1.0f }, // 1
    {  1.0f,  1.0f, -1.0f }, // 2
    { -1.0f,  1.0f, -1.0f }, // 3
    { -1.0f, -1.0f,  1.0f }, // 4
    {  1.0f, -1.0f,  1.0f }, // 5
    {  1.0f,  1.0f,  1.0f }, // 6
    { -1.0f,  1.0f,  1.0f }  // 7
};

// Cube faces (each face is represented by two triangles)
Face faces[] = {
    // Front face (z = -1), Color 0
    { 0, 1, 2, 6 },  // Triangle 1
    { 0, 2, 3, 6 },  // Triangle 2

    // Back face (z = 1), Color 1
    { 4, 5, 6, 1 },  // Triangle 1
    { 4, 6, 7, 1 },  // Triangle 2

    // Left face (x = -1), Color 2
    { 0, 3, 7, 2 },  // Triangle 1
    { 0, 7, 4, 2 },  // Triangle 2

    // Right face (x = 1), Color 3
    { 1, 5, 6, 3 },  // Triangle 1
    { 1, 6, 2, 3 },  // Triangle 2

    // Top face (y = 1), Color 4
    { 2, 3, 7, 4 },  // Triangle 1
    { 2, 7, 6, 4 },  // Triangle 2

    // Bottom face (y = -1), Color 5
    { 0, 1, 5, 5 },  // Triangle 1
    { 0, 5, 4, 5 }   // Triangle 2
};


const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;
const float FOV = 256;  // Field of view

// Function to project 3D point to 2D
// Point2D ProjectTo2D(Point3D point3D) {
//     Point2D point2D;
    
//     // Apply perspective projection
//     float xProjected = (point3D.x * FOV) / (point3D.z + 4);
//     float yProjected = (point3D.y * FOV) / (point3D.z + 4);

//     // Normalize to screen coordinates
//     point2D.x = xProjected + 320 / 2;
//     point2D.y = -yProjected + 240 / 2;

//     return point2D;
// }

float computeFaceDepth(Face &face) {
    // Calculate average depth for sorting
    float z0 = vertices[face.v0].z;
    float z1 = vertices[face.v1].z;
    float z2 = vertices[face.v2].z;
    return (z0 + z1 + z2) / 3;
}

void sortFacesByDepth() {
    for (Face &face : faces) {
        face.depth = computeFaceDepth(face);
    }
    std::sort(faces, faces + sizeof(faces) / sizeof(Face), [](const Face &a, const Face &b) {
        return a.depth > b.depth;
    });
}

Point2D ProjectTo2D(Point3D p) {
    int x = SCREEN_WIDTH / 2 + (p.x * FOV) / (p.z + 4);
    int y = SCREEN_HEIGHT / 2 - (p.y * FOV) / (p.z + 4);
    return {x, y};
}

std::vector<double> Interpolate(int i0, double d0, int i1, double d1) {
    std::vector<double> values;

    if (i0 == i1) {
        values.push_back(d0);
        return values;
    }

    int di = (i1 - i0);
    if(!di) di = 1;
    double a = (d1 - d0) / di;
    double d = d0;

    for (int i = i0; i <= i1; i++) {
        values.push_back(d);
        d += a;
    }

    return values;
}

void DrawFilledTriangle(Point2D p0, Point2D p1, Point2D p2, int color) {
  // Sort the points from bottom to top.
  if (p1.y < p0.y) { std::swap(p0, p1); }
  if (p2.y < p0.y) { std::swap(p0, p2); }
  if (p2.y < p1.y) { std::swap(p2, p1); }

  // Compute X coordinates of the edges.
  std::vector<double> x01 = Interpolate(p0.y, p0.x, p1.y, p1.x);
  std::vector<double> x12 = Interpolate(p1.y, p1.x, p2.y, p2.x);
  std::vector<double> x02 = Interpolate(p0.y, p0.x, p2.y, p2.x);

  // Merge the two short sides.
  x01.pop_back();
  std::vector<double> x012;
  x012.reserve(x01.size() + x12.size());
  x012.insert(x012.end(), x01.begin(), x01.end());  // Insert elements of vec1
  x012.insert(x012.end(), x12.begin(), x12.end());  // Insert elements of vec2

  // Determine which is left and which is right.
  std::vector<double> x_left, x_right;
  int m = (x02.size()/2) | 0;
  if (x02[m] < x012[m]) {
    x_left = x02;
    x_right = x012;
  } else {
    x_left = x012;
    x_right = x02;
  }

    // Draw horizontal segments.
    for (int y = p0.y; y <= p2.y; y++) {
        for (int x = x_left[y - p0.y]; x <= x_right[y - p0.y]; x++) {
            GFX::setPixel(x, y, color);
        }
    }
}

void fillFace(Face face) {
    Point2D p0 = ProjectTo2D(vertices[face.v0]);
    Point2D p1 = ProjectTo2D(vertices[face.v1]);
    Point2D p2 = ProjectTo2D(vertices[face.v2]);

    // Fill two triangles for the quadrilateral face
    DrawFilledTriangle(p0, p1, p2, face.color);
}

void rotateAndFillCube(float angleX, float angleY, float angleZ) {
    // Rotation matrices
    float cosX = cos(angleX), sinX = sin(angleX);
    float cosY = cos(angleY), sinY = sin(angleY);
    float cosZ = cos(angleZ), sinZ = sin(angleZ);

    // Render each face of the cube
    for (Face &face : faces) {
        memcpy(vertices, oldVertices, sizeof(oldVertices));
        int vertexIndices[] = {face.v0, face.v1, face.v2};
        for (int idx : vertexIndices) {
            Point3D &v = vertices[idx];

            // Rotate around X
            float yRot = cosX * v.y - sinX * v.z;
            float zRot = sinX * v.y + cosX * v.z;
            v.y = yRot;
            v.z = zRot;

            // Rotate around Y
            float xRot = cosY * v.x + sinY * v.z;
            zRot = -sinY * v.x + cosY * v.z;
            v.x = xRot;
            v.z = zRot;

            // Rotate around Z
            float finalX = cosZ * v.x - sinZ * v.y;
            float finalY = sinZ * v.x + cosZ * v.y;
            v.x = finalX;
            v.y = finalY;
        }

        // Fill the face
        fillFace(face);
    }
}


void renderCube() {
    GFX::clearFrameBuffer();
    static float angleX = 0, angleY = 0, angleZ = 0;
    angleX += 0.02;  // Rotation speed around X
    angleY += 0.02;  // Rotation speed around Y
    angleZ += 0.02;  // Rotation speed around Z

    rotateAndFillCube(angleX, angleY, angleZ);
}

void setup() {
    Serial.begin(921600);
    GFX::init();
    N64C::init();
    N64C::checkForFactoryStart();
    GFX::clearFrameBuffer();
}

void loop() {
    renderCube();
    GFX::executeRoutines();
    GFX::updateScreen();
    N64C::updateInputBuffer();
    N64C::update();
}

# C++ Ray Tracer

A C++ ray tracer built to practice core computer graphics concepts.. The project implements core ray tracing concepts including camera rays, object intersections, lighting, shadows, Blinn-Phong shading, and recursive mirror reflections.

---

## Overview

This project was built to understand how ray tracing works from the ground up. The renderer parses a scene input file, creates camera rays for each pixel, checks intersections with scene objects, computes lighting and shading, and writes the final rendered image.

The project includes support for spheres and triangles, material properties, point lights, ambient lighting, shadows, specular highlights, and recursive mirror reflections.

---

## Features

- Ray generation from camera parameters
- Sphere intersection
- Triangle intersection using the Möller-Trumbore algorithm
- Ambient lighting
- Diffuse shading using Lambertian reflection
- Specular highlights using the Blinn-Phong model
- Shadow rays
- Recursive mirror reflections
- Material handling using material IDs
- Scene parsing from an input file
- PPM-style image output
- Tested with Visual Studio and WSL

---

## Tech Stack

**Language:** C++  
**IDE:** Visual Studio  
**Concepts:** Ray Tracing, Computer Graphics, OOP, Geometry, Vectors, Lighting Models, Recursion  
**Testing / Tools:** WSL, IrfanView for viewing PPM outputs  

---

## Implementation Details

The program includes classes and structures for:

- 3D vectors
- Rays
- Materials
- Lights
- Hit records
- Scene objects
- Spheres
- Triangles
- Camera configuration

The ray tracer checks intersections between rays and objects, stores the closest hit, and shades the result based on material and lighting information.

---

## Shading Model

The shading pipeline includes:

### Ambient Shading

Ambient light is applied as a base contribution to visible objects.

### Diffuse Shading

Diffuse shading is calculated using Lambert’s cosine law, allowing surfaces to react to light direction.

### Specular Highlights

Specular highlights are implemented using the Blinn-Phong model.

### Shadows

Shadow rays are cast from hit points toward light sources to determine whether a point is blocked from direct light.

### Recursive Mirror Reflections

Mirror-like materials can reflect rays recursively up to a maximum recursion depth.

---

## Debugging and Improvements

During development, I encountered a material assignment issue where objects were rendered with incorrect colors. I fixed this by storing materials in an `unordered_map` using material IDs, allowing each object to correctly reference its own material.

---

## How to Run

### Option 1: Visual Studio

1. Open the Visual Studio solution file.
2. Build the project.
3. Run the project with `input.txt` as the scene input file.

### Option 2: Command Line

Compile with a C++ compiler:

```bash
g++ main.cpp -o raytracer
./raytracer input.txt

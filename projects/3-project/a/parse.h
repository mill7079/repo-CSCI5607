#ifndef PARSE_H
#define PARSE_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

#include <vector>
#include "image_lib.h" //Defines an image class and a color class



// Set default values for camera/scene parameters
// From Homework 3

//Image Parameters
int img_width = 640, img_height = 480;
std::string imgName = "raytraced.bmp";

//Camera Parameters
vec3 pos = vec3(0,0,0);
//vec3 fwd = vec3(0,0,-1).normalized();
vec3 fwd = vec3(0,0,1).normalized();
vec3 up = vec3(0,1,0).normalized();
vec3 right = vec3(-1,0,0).normalized();
float halfAngleVFOV = 45;

// if I include this at the top, things don't work
// I do not know how to C++
#include "structs.h"

//Scene (Sphere) Parameters
vec3 spherePos = vec3(0,0,2);
float sphereRadius = 1;
std::vector<sphere> spheres;

// Material parameters
Color background = Color(0,0,0);
//float material [14] = {0, 0, 0, 1, 1, 1, 0, 0, 0, 5, 0, 0, 0, 1};
material cur = material(Color(0,0,0), Color(1,1,1), Color(0,0,0), Color(0,0,0), 5, 1);

// Light parameters
Color ambient = Color(0,0,0);
//std::vector<light> lights;
std::vector<light*> lights;

// Misc parameters
int maxDepth = 5;
float displace = 0.0001;  // move shadow ray out from sphere to avoid speckling


void parseSceneFile(std::string fileName){
   std::ifstream input(fileName.c_str());
   if (input.fail()) {
      std::cout << "Can't open scene file " << fileName << std::endl;
      return;
   }
   
   std::string line, word;
   int count = 0;
   while (input >> word) {
      
      // skip comments
      if (word.length() >= 1 && word[0] == '#') {
         getline(input, line);
         continue;
      }
      
      // really wish c++ switch statements worked on strings...
      if (word == "sphere:") {
         input >> spherePos.x >> spherePos.y >> spherePos.z >> sphereRadius;
         spheres.push_back(sphere(spherePos, sphereRadius, cur));
      } else if (word == "camera_pos:") {
         input >> pos.x >> pos.y >> pos.z;
      } else if (word == "camera_fwd:") {
         input >> fwd.x >> fwd.y >> fwd.z;
         fwd = fwd.normalized();
      } else if (word == "camera_up:") {
         input >> up.x >> up.y >> up.z;
         up = up.normalized();
      } else if (word == "camera_fov_ha:") {
         input >> halfAngleVFOV;
      } else if (word == "image_resolution:" || word == "film_resolution:") {
         input >> img_width >> img_height;
      } else if (word == "output_image:") {
         input >> imgName;
      } else if (word == "background:") {
         input >> background.r >> background.g >> background.b;
      } else if (word == "material:") {
         float r, g, b, ns, ior;
         Color a, d, s, t;
         
         input >> r >> g >> b;
         a = Color(r, g, b);
         
         input >> r >> g >> b;
         d = Color(r, g, b);
         
         input >> r >> g >> b >> ns;
         s = Color(r, g, b);
         
         input >> r >> g >> b >> ior;
         t = Color(r, g, b);
         
         cur = material (a, d, s, t, ns, ior);
      } else if (word == "directional_light:") {
         Color c;
         vec3 d;
         input >> c.r >> c.g >> c.b >> d.x >> d.y >> d.z;
//         lights.push_back(directionalLight(c,d));
         lights.push_back(new directionalLight(c,d));
      } else if (word == "point_light:") {
         Color c;
         vec3 p;
         input >> c.r >> c.g >> c.b >> p.x >> p.y >> p.z;
//         lights.push_back(pointLight(c, p));
         lights.push_back(new pointLight(c, p));
      } else if (word == "spot_light:") {
         Color c;
         vec3 p, d;
         float a1, a2;
         input >> c.r >> c.g >> c.b >> p.x >> p.y >> p.z >>
         d.x >> d.y >> d.z >> a1 >> a2;
//         lights.push_back(spotLight(c, p, d, a1, a2));
         lights.push_back(new spotLight(c, p, d, a1, a2));
      } else if (word == "ambient_light:") {
         input >> ambient.r >> ambient.g >> ambient.b;
      } else if (word == "max_depth:") {
         input >> maxDepth;
      } else {
         std::cout << "Command not recognized: " << word << std::endl;
      }
      
      count++;
   
   }


  //TODO: Create an orthogonal camera basis, based on the provided up and right vectors
   
   float dotProd = dot(fwd, up);
   if (dotProd == 0) {  // already orthogonal :)
//      std::cout << "orthogonal" << std::endl;
   } else if (dotProd == 1) {  // parallel? i think?
      std::cout << "ERROR: Forward and Up vectors cannot be parallel" << std::endl;
      fwd = vec3(0,0,-1).normalized();
      up = vec3(0,1,0).normalized();
   } else {  // orthogonalize
      vec3 u1 = fwd.normalized();
      up = up.normalized();
      up = up - (dot(u1, up) / dot(u1, u1)) * u1;
      
      up = up.normalized();
      fwd = fwd.normalized();
   }
   
   // find right
   // i thought this should be forward x up?? but that reverses the image from the given example
   // so i'm not really sure what to think
   right = cross(up, fwd).normalized();
   
   std::cout << "file: " << imgName << std::endl;
//   printf("Orthogonal Camera Basis:\n");
//   printf("forward: %f,%f,%f\n",fwd.x,fwd.y,fwd.z);
//   printf("right: %f,%f,%f\n",right.x,right.y,right.z);
//   printf("up: %f,%f,%f\n",up.x,up.y,up.z);
}

#endif

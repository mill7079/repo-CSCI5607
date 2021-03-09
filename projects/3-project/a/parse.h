#ifndef PARSE_H
#define PARSE_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

#include <vector>

struct sphere {
   vec3 pos;
   float r;
   sphere(vec3 position, float radius) {
      pos = position;
      r = radius;
   }
};

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

//Scene (Sphere) Parameters
vec3 spherePos = vec3(0,0,2);
float sphereRadius = 1;
std::vector<sphere> spheres;

// Material parameters
Color background = Color(0,0,0);
float material [14] = {0, 0, 0, 1, 1, 1, 0, 0, 0, 5, 0, 0, 0, 1};

// Light parameters
Color ambient = Color(0,0,0);
// TODO: figure out how to handle other light sources

// Misc parameters
int depth = 5;


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
         spheres.push_back(sphere(spherePos, sphereRadius));
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
      } else if (word == "image_resolution:") {
         input >> img_width >> img_height;
      } else if (word == "output_image:") {
         input >> imgName;
      } else if (word == "background:") {
         input >> background.r >> background.g >> background.b;
      } else if (word == "material:") {
         for (int i = 0; i < 14; i++) {
            input >> material[i];
         }
      } else if (word == "directional_light:") {
         
      } else if (word == "point_light:") {
         
      } else if (word == "spot_light:") {
         
      } else if (word == "ambient_light:") {
         input >> ambient.r >> ambient.g >> ambient.b;
      } else if (word == "max_depth:") {
         input >> depth;
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

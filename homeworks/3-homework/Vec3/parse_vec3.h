
//Set the global scene parameter variables
//TODO: Set the scene parameters based on the values in the scene file

#ifndef PARSE_VEC3_H
#define PARSE_VEC3_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

//Camera & Scene Parameters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parameters
int img_width = 800, img_height = 600;
std::string imgName = "raytraced.png";

//Camera Parameters
vec3 eye = vec3(0,0,0); 
vec3 forward = vec3(0,0,-1).normalized();
vec3 up = vec3(0,1,0).normalized();
vec3 right = vec3(-1,0,0).normalized();
float halfAngleVFOV = 35; 

//Scene (Sphere) Parameters
vec3 spherePos = vec3(0,0,2);
float sphereRadius = 1; 

void parseSceneFile(std::string fileName){
  //TODO: Override the default values with new data from the file "fileName"
   std::ifstream input(fileName.c_str());
   if (input.fail()) {
      std::cout << "Can't open file " << fileName << std::endl;
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
      } else if (word == "camera_pos:") {
         input >> eye.x >> eye.y >> eye.z;
      } else if (word == "camera_fwd:") {
         input >> forward.x >> forward.y >> forward.z;
         forward = forward.normalized();
      } else if (word == "camera_up:") {
         input >> up.x >> up.y >> up.z;
         up = up.normalized();
      } else if (word == "camera_fov_ha:") {
         input >> halfAngleVFOV;
      } else if (word == "image_resolution:") {
         input >> img_width >> img_height;
      } else if (word == "output_image:") {
         input >> imgName;
      } else {
         std::cout << "Command not recognized: " << word << std::endl;
      }
      
      count++;
   
   }


  //TODO: Create an orthogonal camera basis, based on the provided up and right vectors
   
   float dotProd = dot(forward, up);
   if (dotProd == 0) {  // already orthogonal :)
      std::cout << "orthogonal" << std::endl;
   } else if (dotProd == 1) {  // parallel? i think?
      std::cout << "ERROR: Forward and Up vectors cannot be parallel" << std::endl;
      forward = vec3(0,0,-1).normalized();
      up = vec3(0,1,0).normalized();
   } else {  // orthogonalize
      vec3 u1 = forward.normalized();
      up = up.normalized();
      up = up - (dot(u1, up) / dot(u1, u1)) * u1;
      
      up = up.normalized();
      forward = forward.normalized();
   }
   
   // find right
   // i thought this should be forward x up?? but that reverses the image from the given example
   // so i'm not really sure what to think
   right = cross(up, forward).normalized();
   
   printf("Orthogonal Camera Basis:\n");
   printf("forward: %f,%f,%f\n",forward.x,forward.y,forward.z);
   printf("right: %f,%f,%f\n",right.x,right.y,right.z);
   printf("up: %f,%f,%f\n",up.x,up.y,up.z);
}

#endif

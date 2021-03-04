
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

//Scene (Sphere) Parmaters
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
      std::cout << "line " << count << ": " << word << std::endl;
      if (word.length() >= 1 && word[0] == '#') {  // skip comments
         std::cout << "comment" << std::endl;
         getline(input, line);
         count++;
         continue;
      }
      
      if (word == "sphere:") {
         std::cout << "sphere" << std::endl;
         input >> spherePos.x >> spherePos.y >> spherePos.z >> sphereRadius;
         std::cout << "new sphere pos: " << spherePos.x << " " << spherePos.y <<
         " " << spherePos.z << std::endl;
         std::cout << "new sphere radius: " << sphereRadius << std::endl;
      } else if (word == "camera_pos:") {
         std::cout << "camera pos" << std::endl;
         input >> eye.x >> eye.y >> eye.z;
         std::cout << "new eye: " << eye.x << " " << eye.y << " " << eye.z << std::endl;
      } else if (word == "camera_fwd:") {
         std::cout << "camera fwd" << std::endl;
         input >> forward.x >> forward.y >> forward.z;
         std::cout << "new forward: " << forward.x << " " << forward.y << " " << forward.z << std::endl;
         forward = forward.normalized();
      } else if (word == "camera_up:") {
         std::cout << "camera up" << std::endl;
         input >> up.x >> up.y >> up.z;
         std::cout << "new up: " << up.x << " " << up.y << " " << up.z << std::endl;
         up = up.normalized();
      } else if (word == "camera_fov_ha:") {
         std::cout << "fov" << std::endl;
         input >> halfAngleVFOV;
         std::cout << "new fov: " << halfAngleVFOV << std::endl;
      } else if (word == "image_resolution:") {
         std::cout << "resolution" << std::endl;
         input >> img_width >> img_height;
         std::cout << "new width: " << img_width << " new height: " << img_height << std::endl;
      } else if (word == "output_image:") {
         std::cout << "output image" << std::endl;
         input >> imgName;
         std::cout << "new image name: " << imgName << std::endl;
      } else {
         std::cout << "Command not recognized: " << word << std::endl;
      }
      
      count++;
   
   }


  //TODO: Create an orthagonal camera basis, based on the provided up and right vectors
  printf("Orthagonal Camera Basis:\n");
  printf("forward: %f,%f,%f\n",forward.x,forward.y,forward.z);
  printf("right: %f,%f,%f\n",right.x,right.y,right.z);
  printf("up: %f,%f,%f\n",up.x,up.y,up.z);
}

#endif

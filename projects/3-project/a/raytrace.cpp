// from Homework 3:
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp file
#include "image_lib.h" //Defines an image class and a color class

//#Vec3 Library
#include "vec3.h"

// Scene parser with scene variables
#include "parse.h"

Color white = Color(1,1,1);

bool raySphereIntersection(vec3 pos, vec3 dir) {
//   bool hit = false;
   for (sphere s : spheres) {
      vec3 toStart = (pos - s.pos);
      float a = dot(dir, dir);
      float b = 2 * dot(dir,toStart);
      float c = dot(toStart, toStart) - pow(s.r, 2);
      float det = pow(b,2) - (4*a*c);
      
      if (det < 0) continue;
      else {
         float t0 = (-b + sqrt(det)) / (2*a);
         float t1 = (-b - sqrt(det)) / (2*a);
         if (t0 > 0 || t1 > 0) {
//            std::cout << "hit" << std::endl;
            return true;
         }
      }
   }
//   return hit;
   return false;
}

int main(int argc, char** argv) {
   // get scene file name from args and parse scene
   if (argc != 2){
      std::cout << "Usage: ./a.out scenefile\n";
      return(0);
   }
   std::string file = argv[1];
   parseSceneFile(file);
   
   // create image
   Image img = Image(img_width, img_height);
   
   float width = img_width, height = img_height;  // turns out this is actually necessary. whoops
   float half_w = width / 2, half_h = height / 2;
   float d = half_h / tanf(halfAngleVFOV * (M_PI / 180.0f));
   
   // raytrace :)
   int i, j;
   for (int i = 0; i < img_width; i++) {
      for (int j = 0; j < img_height; j++) {
         float u = (half_w - (width) * (i/width));
         float v = (half_h - (height) * (j/height));
         vec3 p = pos - d*fwd + u*right + v*up;
         vec3 dir = (p - pos).normalized();
         bool hit = raySphereIntersection(pos, dir);
         if (hit) img.setPixel(i, j, white);
         else img.setPixel(i, j, background);
      }
   }
   
//   img.write("output/" + imgName.c_str());
   img.write(imgName.c_str());
   return 0;
}

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

Color getColor(vec3 point, sphere s) {
   
   Color color = Color(0,0,0);
   vec3 n = point - s.pos;
   n = n.normalized();
   
//   vec3 v = (point - pos).normalized();
//   vec3 v = point - pos;
//   vec3 v = pos - point;
   vec3 v = (pos - point).normalized();
   
   color.r = ambient.r * s.mat.ambient.r;
   color.g = ambient.g * s.mat.ambient.g;
   color.b = ambient.b * s.mat.ambient.b;
   
   // kdI*max(0, n dot l) + ksI*max(0, n dot h)^p
   for (light l : lights) {
//      vec3 lDir = (point - l.pos).normalized();
      vec3 lDir = (l.pos - point).normalized();
//      vec3 lDir = l.pos - point;
      vec3 h = (v+lDir).normalized();
      
      // attenuate with distance from light
      float dSquare = pow((l.pos - point).length(), 2);
      
      color.r += (s.mat.diffuse.r * l.i.r * fmax(0, dot(n, lDir)))/dSquare + (s.mat.specular.r * l.i.r * pow(fmax(0, dot(n, h)), s.mat.ns))/dSquare;
      color.g += (s.mat.diffuse.g * l.i.g * fmax(0, dot(n, lDir)))/dSquare + (s.mat.specular.g * l.i.g * pow(fmax(0, dot(n, h)), s.mat.ns))/dSquare;
      color.b += (s.mat.diffuse.b * l.i.b * fmax(0, dot(n, lDir)))/dSquare + (s.mat.specular.b * l.i.b * pow(fmax(0, dot(n, h)), s.mat.ns))/dSquare;
   }
   
   return color;
}

//bool raySphereIntersection(vec3 pos, vec3 dir) {
Color raySphereIntersection(vec3 pos, vec3 dir) {
   float minMag = -1;
   vec3 point = vec3(0,0,0);
   sphere hitSphere = sphere(point, 1, cur);
   bool hit = false;
   Color color = background;
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
            hit = true;
//            std::cout << "hit" << std::endl;
//            return true;
            if (toStart.length() < minMag || minMag == -1) {
               minMag = toStart.length();
//               point = pos + fmax(t0,t1)*dir;  // maybe fmin? idk how t0 and t1 relate
               point = pos + fmin(t0,t1)*dir;  // definitely fmin
               hitSphere = s;
//               color = s.mat.diffuse;
//               color.r = s.mat.ambient.r + s.mat.diffuse.r + s.mat.specular.r;
//               color.g = s.mat.ambient.g + s.mat.diffuse.g + s.mat.specular.g;
//               color.b = s.mat.ambient.b + s.mat.diffuse.b + s.mat.specular.b;
            }
//            return s.mat.diffuse;
         }
      }
   }
   
//   return hit;
//   return false;
//   return background;
//   return color;
//   return point;
   if (!hit) return background;
   return getColor(point, hitSphere);
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
         
//         bool hit = raySphereIntersection(pos, dir);
//         if (hit) {
//            img.setPixel(i, j, getColor());
//         }
//         else img.setPixel(i, j, background);
//         img.setPixel(i, j, raySphereIntersection(pos,dir));
         img.setPixel(i, j, raySphereIntersection(pos, dir));
      }
   }
   
//   img.write("output/" + imgName.c_str());
   img.write(imgName.c_str());
   return 0;
}

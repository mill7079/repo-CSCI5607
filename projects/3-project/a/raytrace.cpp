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
#include "structs.h"

#include <limits>

//Color white = Color(1,1,1);

//bool raySphereIntersection(vec3 pos, vec3 dir) {
//Color raySphereIntersection(vec3 pos, vec3 dir) {
intersection raySphereIntersection(vec3 pos, vec3 dir) {
//   float minMag = -1;
   float minMag = INFINITY;
   vec3 point = vec3(0,0,0);
   sphere hitSphere = sphere(point, 1, cur);
   bool hit = false;
   Color color = background;
   for (sphere s : spheres) {
      vec3 toStart = (pos - s.pos);
      if (toStart.length() >= minMag) continue;  // don't need to check intersection if there's a closer intersecting sphere
      
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
//            if (toStart.length() < minMag) {//} || minMag == -1) {
            minMag = toStart.length();
//               point = pos + fmax(t0,t1)*dir;  // maybe fmin? idk how t0 and t1 relate
            point = pos + fmin(t0,t1)*dir;  // definitely fmin
            hitSphere = s;
//            }
         }
      }
   }
   
//   if (!hit) return background;
//   return getColor(point, hitSphere);
   return intersection(hit, point, hitSphere);
}

//Color getColor(vec3 point, sphere s) {
Color getColor(intersection i) {
   if (!i.hit) return background; // eye ray did not hit sphere
   
   vec3 point = i.point;
   sphere s = i.s;
   
   Color color = Color(0,0,0);
   
   // normal
   vec3 n = point - s.pos;
   n = n.normalized();
   
   // view direction
   vec3 v = (pos - point).normalized();
   
   // avoid hitting current sphere with shadow ray
   vec3 pS = point + (displace * n);
   
   // start with ambient light
   color.r = ambient.r * s.mat.ambient.r;
   color.g = ambient.g * s.mat.ambient.g;
   color.b = ambient.b * s.mat.ambient.b;
   
   // kdI*max(0, n dot l) + ksI*max(0, n dot h)^p
   // calculate contributions for each light source
   for (light* l : lights) {
      // vector to light source
//      vec3 toLight = l->pos - point;
//
//      // don't add light if point is in shadow
//      if (raySphereIntersection(pS, toLight.normalized()).hit) continue;
//
//      // light direction and halfway vector for blinn-phong
//      vec3 lDir = toLight.normalized();
//      vec3 h = (v+lDir).normalized();
//
//      // attenuate with distance from light (1/d^2)
//      float dSquare = pow(toLight.length(), 2);
//
//      // calculate diffuse and specular contributions
//      Color dif = l->diffuse(s.mat, lDir, n);
//      Color spec = l->specular(s.mat, n, h);
//
//      color.r += (dif.r + spec.r) / dSquare;
//      color.g += (dif.g + spec.g) / dSquare;
//      color.b += (dif.b + spec.b) / dSquare;
      
      Color c = l->findLight(s, pS);
      
      color.r += c.r;
      color.g += c.g;
      color.b += c.b;
   }
   
   return color;
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
         
         img.setPixel(i, j, getColor(raySphereIntersection(pos, dir)));
      }
   }
   
//   img.write("output/" + imgName.c_str());
   img.write(imgName.c_str());
   
   for (light* l : lights) {
      free(l);
   }
   return 0;
}

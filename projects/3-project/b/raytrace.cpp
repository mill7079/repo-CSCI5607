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
            minMag = toStart.length();
//            point = pos + fmin(t0,t1)*dir;
            point = pos + fmin(fmax(0,t0),fmax(0,t1))*dir;
            point = pos + fmin(fmax(displace,t0),fmax(displace,t1))*dir;
            hitSphere = s;
         }
      }
   }
   
//   if (!hit) return background;
//   return getColor(point, hitSphere);
   return intersection(hit, point, hitSphere);
}

//Color getColor(vec3 point, sphere s) {
//Color getColor(intersection i) {
Color getColor(intersection i, int depth, vec3 initPos) {
//   if (!i.hit) return background; // eye ray did not hit sphere
   if (!i.hit && depth == 1) return background;
   if (!i.hit) return Color();
   
   vec3 point = i.point;
   sphere s = i.s;
   
   Color color = Color(0,0,0);
   
   // normal
   vec3 n = point - s.pos;
   n = n.normalized();
   
   // avoid hitting current sphere with shadow ray
   vec3 pS = point + (displace * n);
   
   // view direction
//   vec3 v = (pos - point).normalized();
//   vec3 v = (initPos - point).normalized();
   vec3 v = (initPos - pS).normalized();
   
   // reflection vector
   // r = d - 2*dot(d,n) * n
   // 𝑟=𝑑−2(𝑑⋅𝑛)𝑛
//   vec3 d = point - initPos;
   vec3 d = pS - initPos;
   vec3 r = d - 2*dot(d,n) * n;
//   vec3 r = v - 2*dot(v,n) * n;
   
   // start with ambient light
//   color.r = ambient.r * s.mat.ambient.r;
//   color.g = ambient.g * s.mat.ambient.g;
//   color.b = ambient.b * s.mat.ambient.b;
   color = ambient * s.mat.ambient;
   
   // kdI*max(0, n dot l) + ksI*max(0, n dot h)^p
   // calculate contributions for each light source
   for (light* l : lights) {
      
//      Color c = l->findLight(s, pS);
      Color c = l->findLight(s, pS, v, r);
      
      color = color + c;
   }
   
   // reflection?
   if (depth < maxDepth) {
//      color = color + getColor(raySphereIntersection(point, r), depth+1, point);
//      color = color + getColor(raySphereIntersection(point, r), depth+1, pS);
      color = color + i.s.mat.specular * getColor(raySphereIntersection(pS, r.normalized()), depth+1, pS);
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
         
//         vec3 p = pos - d*fwd + u*right + v*up;
         vec3 p = camPos - d*fwd + u*right + v*up;
//         vec3 dir = (p - pos).normalized();
         vec3 dir = (p - camPos).normalized();
         
//         img.setPixel(i, j, getColor(raySphereIntersection(pos, dir), 1, pos));
         img.setPixel(i, j, getColor(raySphereIntersection(camPos, dir), 1, camPos));
      }
   }
   
//   img.write("output/" + imgName.c_str());
   img.write(imgName.c_str());
   
   for (light* l : lights) {
      free(l);
   }
   return 0;
}











































//// from Homework 3:
//#ifdef _MSC_VER
//#define _CRT_SECURE_NO_WARNINGS
//#endif
//
////Images Lib includes:
//#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
//#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp file
//#include "image_lib.h" //Defines an image class and a color class
//
////#Vec3 Library
//#include "vec3.h"
//
//// Scene parser with scene variables
//#include "parse.h"
//#include "structs.h"
//
//// infinity
//#include <limits>
//
//intersection rayShapeIntersection(vec3 pos, vec3 dir) {
//   float minMag = INFINITY;
//   intersection ret = intersection(false, vec3(0,0,0), shapes[0]);
//
//   Color color = background;
////   for (shape s : shapes) {
//   for (shape* s : shapes) {
////      intersection i = s.intersect(pos, dir);
//      intersection i = s->intersect(pos, dir);
//
//      if (i.hit && (pos-i.point).length() < minMag) {
//         minMag = (pos-i.point).length();
//         ret = i;
//      }
//   }
//
//   return ret;
//}
//
//Color getColor(intersection i, int depth, vec3 initPos) {
//   if (!i.hit && depth == 1) return background;
//   if (!i.hit) return Color();
//
//   vec3 point = i.point;
////   sphere s = i.s;
//   shape* s = i.s;
//
//   Color color = Color(0,0,0);
//
//   // normal
////   vec3 n = point - s.pos;
////   n = n.normalized();
//   vec3 n = s->findNormal(point);
//
//   // avoid hitting current shape with shadow ray
//   vec3 pS = point + (displace * n);
//
//   // view direction
////   vec3 v = (initPos - point).normalized();
//   vec3 v = (initPos - pS).normalized();
//
//   // reflection vector
//   // 𝑟=𝑑−2(𝑑⋅𝑛)𝑛
//   vec3 d = pS - initPos;
//   vec3 r = d - 2*dot(d,n) * n;
//
//   // start with ambient light
////   color = ambient * s.mat.ambient;
//   color = ambient * s->mat.ambient;
//
//   // kdI*max(0, n dot l) + ksI*max(0, n dot h)^p
//   // calculate contributions for each light source
//   for (light* l : lights) {
//      Color c = l->findLight(s, pS, v, r);
//      color = color + c;
//   }
//
//   // reflection?
//   if (depth < maxDepth) {
////      color = color + i.s.mat.specular * getColor(raySphereIntersection(pS, r.normalized()), depth+1, pS);
//      color = color + i.s->mat.specular * getColor(rayShapeIntersection(pS, r.normalized()), depth+1, pS);
//   }
//
//   return color;
//}
//
//int main(int argc, char** argv) {
//   // get scene file name from args and parse scene
//   if (argc != 2){
//      std::cout << "Usage: ./a.out scenefile\n";
//      return(0);
//   }
//   std::string file = argv[1];
//   parseSceneFile(file);
//
//   if (shapes.size() <= 0) {
//      std::cout << "Error: No shapes found." << std::endl;
//      return(1);
//   }
//
//   // create image
//   Image img = Image(img_width, img_height);
//
//   float width = img_width, height = img_height;  // turns out this is actually necessary. whoops
//   float half_w = width / 2, half_h = height / 2;
//   float d = half_h / tanf(halfAngleVFOV * (M_PI / 180.0f));
//
//   // raytrace :)
//   // int i, j;
////   #pragma omp parallel for
//   for (int i = 0; i < img_width; i++) {
//      for (int j = 0; j < img_height; j++) {
//         float u = (half_w - (width) * (i/width));
//         float v = (half_h - (height) * (j/height));
//
//         vec3 p = camPos - d*fwd + u*right + v*up;
//         vec3 dir = (p - camPos).normalized();
//
//         img.setPixel(i, j, getColor(rayShapeIntersection(camPos, dir), 1, camPos));
//      }
//   }
//
//   img.write(imgName.c_str());
//
//   for (light* l : lights) {
//      free(l);
//   }
//   for (shape* s : shapes) {
//      free(s);
//   }
//   return 0;
//}

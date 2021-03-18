// from Homework 3:
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION // only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION // only place once in one .cpp file
#include "image_lib.h" // Defines an image class and a color class

// High resolution timer
#include <chrono>

// Vec3 Library
#include "vec3.h"

// Scene parser with scene variables
#include "parse.h"
#include "structs.h"

// infinity
#include <limits>

// test if ray defined by pos and dir intersects with any shapes in scene
//intersection raySphereIntersection(vec3 pos, vec3 dir) {
intersection rayShapeIntersection(vec3 pos, vec3 dir) {
   float minMag = INFINITY;
   vec3 point = vec3(0,0,0);
   sphere* hitSphere = new sphere(point, 1, cur);
   bool hit = false;
   Color color = background;
   
   intersection ret = intersection(false, vec3(0,0,0), shapes[0]);
   for (shape* s : shapes) {
      intersection i = s->intersect(pos, dir);

      if (i.hit && (pos-i.point).length() < minMag) {
         minMag = (pos-i.point).length();
         ret = i;
      }
   }
   
   return ret;
}

// apply lighting model and reflection/refraction to get color of a pixel
Color getColor(intersection i, int depth, vec3 initPos) {
   if (!i.hit && depth == 1) return background;
   if (!i.hit) return Color();
   
   vec3 point = i.point;  // intersection point
   Color color = Color(0,0,0);
   
   // normal
//   vec3 n = point - s->pos;
//   n = n.normalized();
   vec3 n = i.s->findNormal(point);
//   if (dot((initPos - point).normalized(), n) > 0) n = -1 * n;
   
   // avoid hitting current sphere with shadow ray
   vec3 pS = point + (displace * n);
   
   // view direction
   vec3 v = (initPos - pS).normalized();
   
   // reflection vector
   // r = d - 2*dot(d,n) * n
   // 𝑟=𝑑−2(𝑑⋅𝑛)𝑛
//   vec3 d = point - initPos;
   vec3 d = pS - initPos;
   vec3 r = d - 2*dot(d,n) * n;
//   vec3 r = v - 2*dot(v,n) * n;
   
   // start with ambient light
   color = ambient * i.s->mat.ambient;
   
   // kdI*max(0, n dot l) + ksI*max(0, n dot h)^p
   // calculate contributions for each light source
   for (light* l : lights) {
      Color c = l->findLight(i.s, pS, v, r);
      color = color + c;
   }
   
   // reflection/refraction
   if (depth < maxDepth) {
      color = color + i.s->mat.specular * getColor(rayShapeIntersection(pS, r.normalized()), depth+1, pS);
      
//      for (light* l : lights) {
//         color = color + l->refract(i.s, pS, n);
//      }
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
   
   if (shapes.size() <= 0) {
      std::cout << "Error: No shapes found." << std::endl;
      return(1);
   }
   
   // create image
   Image img = Image(img_width, img_height);
   
   float width = img_width, height = img_height;  // turns out this is actually necessary. whoops
   float half_w = width / 2, half_h = height / 2;
   float d = half_h / tanf(halfAngleVFOV * (M_PI / 180.0f));
   
   // raytrace :)
   auto t_start = std::chrono::high_resolution_clock::now();
   #pragma omp parallel for
   for (int i = 0; i < img_width; i++) {
      for (int j = 0; j < img_height; j++) {
         float u = (half_w - (width) * (i/width));
         float v = (half_h - (height) * (j/height));
         
         vec3 p = camPos - d*fwd + u*right + v*up;
         vec3 dir = (p - camPos).normalized();
         
         img.setPixel(i, j, getColor(rayShapeIntersection(camPos, dir), 1, camPos));
      }
   }
   auto t_end = std::chrono::high_resolution_clock::now();
   printf("Rendering took %.2f ms\n",std::chrono::duration<double, std::milli>(t_end-t_start).count());
   
   img.write(imgName.c_str());
   
   for (light* l : lights) {
      free(l);
   }
   for (shape* s : shapes) {
      free(s);
   }
   return 0;
}

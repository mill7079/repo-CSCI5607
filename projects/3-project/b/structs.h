#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

#include <vector>
#include "image_lib.h" // Defines an image class and a color class

#include "parse.h"
//#include "raytrace.cpp"

// forward declaration for the forward declaration. whyyyyy
struct intersection;
intersection raySphereIntersection(vec3 pos, vec3 dir);

// material for objects
struct material {
   Color ambient, diffuse, specular, transmissive;
   float ns, ior;
   material(Color a, Color d, Color s, Color t, float n, float i) {
      ambient = a;
      diffuse = d;
      specular = s;
      transmissive = t;
      ns = n;
      ior = i;
   }
   
   material(){}  // it complains if I don't have this
};

// general shape class for spheres and triangles
class shape {
public:
   material mat;
   shape(){};

   virtual intersection intersect(vec3 pos, vec3 dir) = 0;
   virtual vec3 findNormal(vec3 hitPoint) = 0;
};

// contains intersection information
struct intersection {
   bool hit;
   vec3 point;
//   vec3 ray;
   shape* s;
   intersection(bool h, vec3 p, shape* sh) {
      hit = h;
      point = p;
      s = sh;
   }
};

class sphere : public shape{  // position of center, radius, material
public:
   vec3 pos;
   float r;
   sphere(vec3 position, float radius, material m) {
      pos = position;
      r = radius;
      mat = m;
   }
   
   intersection intersect(vec3 rayPos, vec3 dir) {
      intersection ret = intersection(false, rayPos, this);
      vec3 toStart = (rayPos - pos);

      float a = dot(dir, dir);
      float b = 2 * dot(dir,toStart);
      float c = dot(toStart, toStart) - pow(r, 2);
      float det = pow(b,2) - (4*a*c);

      if (det < 0) return ret;
      else {
         float t0 = (-b + sqrt(det)) / (2*a);
         float t1 = (-b - sqrt(det)) / (2*a);
         if (t0 > 0 || t1 > 0) {
            ret.hit = true;
//            ret.point = pos + fmin(fmax(displace,t0),fmax(displace,t1))*dir;  // RIP
            ret.point = rayPos + fmin(fmax(displace,t0),fmax(displace,t1))*dir;

         }
      }

      return ret;
   }

   vec3 findNormal(vec3 hitPoint) {
      return (hitPoint - this->pos).normalized();
   }

   sphere() {}
};

 
// lights - general class for point, directional, and spot lights
class light {
public:
   Color i;
   vec3 p, dir;
   float a1, a2;
   
   // refactor actual lighting calculations out to light classes to avoid issues
   virtual Color findLight(shape* s, vec3 point, vec3 v, vec3 r) = 0;
};

class pointLight : public light {
public:
   pointLight(Color intensity, vec3 position) {
      i = intensity;
      p = position;
   }
   
   Color findLight(shape* s, vec3 point, vec3 v, vec3 r) override {
      vec3 toLight = p - point;
      vec3 lDir = toLight.normalized();
      Color c = Color(0,0,0);
      
      // don't add light if point is in shadow
      if (raySphereIntersection(point, lDir).hit) return c;
      
      // normal
      vec3 n = s->findNormal(point);
      
      // halfway vector
      vec3 h = ((camPos - point).normalized() + lDir).normalized();  // TODO: check camPos? shouldn't it be from reflected point?
      
//      vec3 v2 = (pos-point).normalized();
      
      // attenuate with distance from light (1/d^2)
      float dSquare = pow(toLight.length(), 2);
      
      // diffuse factor
      float dMult = fmax(0, dot(n, lDir));
      
      // specular factor -- v dot r
      float sMult = pow(fmax(0, dot(n, h)), s->mat.ns);
//      float sMult = pow(fmax(0, dot(v,r.normalized())), s.mat.ns);
//      float sMult = pow(fmax(0, dot(v2,r.normalized())), s.mat.ns);

      // calculate light/material contributions
      c.r += i.r * ((s->mat.diffuse.r * dMult) / dSquare + (s->mat.specular.r * sMult)/dSquare);
      c.g += i.g * ((s->mat.diffuse.g * dMult) / dSquare + (s->mat.specular.g * sMult)/dSquare);
      c.b += i.b * ((s->mat.diffuse.b * dMult) / dSquare + (s->mat.specular.b * sMult)/dSquare);
      
      return c;
   }
};

class directionalLight : public light {
public:
   directionalLight(Color intensity, vec3 direction) {
      i = intensity;
      dir = direction;
   }
   
   Color findLight(shape* s, vec3 point, vec3 v, vec3 r) override {
      Color c = Color(0,0,0);
      vec3 toLight = -1 * dir;
      vec3 lDir = toLight.normalized();
      
      // shadow
      if (raySphereIntersection(point, lDir).hit) return c;
      
      // normal
      vec3 n = s->findNormal(point);
      
      // halfway vector
      vec3 h = ((camPos - point).normalized() + lDir).normalized();  // TODO: check camPos? shouldn't it be from reflected point?
      
      // diffuse factor
      float dMult = fmax(0, dot(n, lDir));
      
      // specular factor
//      float sMult = pow(fmax(0, dot(n, h)), s.mat.ns);
      float sMult = pow(fmax(0, dot(n, h)), s->mat.ns);
//      float sMult = pow(fmax(0, dot(v,r.normalized())), s.mat.ns);
      
      c.r += i.r * (s->mat.diffuse.r * dMult + s->mat.specular.r * sMult);
      c.g += i.g * (s->mat.diffuse.g * dMult + s->mat.specular.g * sMult);
      c.b += i.b * (s->mat.diffuse.b * dMult + s->mat.specular.b * sMult);
      
      return c;
   }
};

class spotLight : public light {
public:
   spotLight(Color intensity, vec3 position, vec3 direction, float angle1, float angle2) {
      i = intensity;
      p = position;
      dir = direction;
      a1 = angle1;
      a2 = angle2;
   }
   
   // not implemented
   Color findLight(shape* s, vec3 point, vec3 v, vec3 r) override
   {return Color(0,0,0);}

};
#endif





































//#ifndef STRUCTS_H
//#define STRUCTS_H
//
//#include <cstdio>
//#include <iostream>
//#include <fstream>
//#include <cstring>
//
//#include <vector>
//#include "image_lib.h" //Defines an image class and a color class
//
//#include "parse.h"
//
//// forward declaration for the forward declaration. whyyyyy
//struct intersection;
//intersection rayShapeIntersection(vec3 pos, vec3 dir);
//
//// material for objects
//struct material {
//   Color ambient, diffuse, specular, transmissive;
//   float ns, ior;
//   material(Color a, Color d, Color s, Color t, float n, float i) {
//      ambient = a;
//      diffuse = d;
//      specular = s;
//      transmissive = t;
//      ns = n;
//      ior = i;
//   }
//
//   material(){}  // it complains if I don't have this
//};
//
//struct intersection;
//
//// overall shape class
//class shape {
//public:
//   material mat;
//   shape(){};
//
//   virtual intersection intersect(vec3 pos, vec3 dir) = 0;
//   virtual vec3 findNormal(vec3 hitPoint) = 0;
//};
//
//// point of intersection, sphere intersected with
//struct intersection {
//   bool hit;
//   vec3 point;
////   vec3 ray;
////   sphere s;
//   shape* s;
////   intersection(bool h, vec3 p, sphere sph) {
//   intersection(bool h, vec3 p, shape* sh) {
//      hit = h;
//      point = p;
//      s = sh;
//   }
//};
//
//// sphere class
//class sphere: public shape {
//public:
//   vec3 pos;
//   float r;
//   bool print = false;
//
//   sphere(vec3 position, float radius, material m) {
//      pos = position;
//      r = radius;
//      mat = m;
//   }
//
//   intersection intersect(vec3 rayPos, vec3 dir) {
//      intersection ret = intersection(false, pos, this);
//      vec3 toStart = (rayPos - pos);
//
//      float a = dot(dir, dir);
//      float b = 2 * dot(dir,toStart);
//      float c = dot(toStart, toStart) - pow(r, 2);
//      float det = pow(b,2) - (4*a*c);
//
////      if (det >= 0) {
//      if (det < 0) return ret;
//      else {
//         float t0 = (-b + sqrt(det)) / (2*a);
//         float t1 = (-b - sqrt(det)) / (2*a);
//         if (!print) {
//            std::cout << "raypos " << rayPos.x << ", " << rayPos.y << ", " << rayPos.z << std::endl;
//            std::cout << "for sphere " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
//            std::cout << "t0: " << t0 << " t1: " << t1 << std::endl << std::endl;
//            print = true;
//         }
//         if (t0 > 0 || t1 > 0) {
//            ret.hit = true;
////            point = pos + fmin(t0,t1)*dir;
//            ret.point = pos + fmin(fmax(displace,t0),fmax(displace,t1))*dir;
//         }
//      }
//
//      return ret;
//   }
//
//   vec3 findNormal(vec3 hitPoint) {
////      return (pos-hitPoint).normalized();
////      std::cout << "for point " << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << " and position ";
////      std::cout << this->pos.x << ", " << this->pos.y << ", " << this->pos.z << ": " << std::endl;
//      return (hitPoint - this->pos).normalized();
//   }
//};
//
////class flatTriangle: public shape {
////public:
////
////   flatTriangle(vec3 v1, vec3 v2, vec3 v3) {
////
////   }
////}
////
////class triangle: public shape {
////public:
////   triangle (vec3 v1, vec3 v2, vec3 v3, vec3 n1, vec3 n2, vec3 n3) {
////
////   }
////}
//
//
//// lights - general class for point, directional, and spot lights
//class light {
//public:
//   Color i;
////   vec3 pos, dir;
//   vec3 p, dir;
//   float a1, a2;
//
//   // refactor actual calculations out to structs to avoid issues
////   virtual Color findLight(sphere s, vec3 point) = 0;
//   virtual Color findLight(shape* s, vec3 point, vec3 v, vec3 r) = 0;
//};
//
//class pointLight : public light {
//public:
//   pointLight(Color intensity, vec3 position) {
//      i = intensity;
////      pos = position;
//      p = position;
//   }
//
////   Color findLight(sphere s, vec3 point) override {
////   Color findLight(sphere s, vec3 point, vec3 v, vec3 r) override {
//   Color findLight(shape* s, vec3 point, vec3 v, vec3 r) override {
////      std::cout << "find light" << std::endl;
//      vec3 toLight = p - point;
//      vec3 lDir = toLight.normalized();
//      Color c = Color(0,0,0);
//      material m = s->mat;
//
//      // don't add light if point is in shadow
////      if (raySphereIntersection(point, lDir).hit) return c;
////      if (s->intersect(point, lDir).hit) return c;
//      if (rayShapeIntersection(point, lDir).hit) return c;
//
//      // normal
////      vec3 n = (point - s.pos).normalized();
//      vec3 n = s->findNormal(point);
//
//      //TODO: print
////      sphere* sph = (sphere*) s;
////      if (sph->pos.x == -3) {
////         std::cout << "for point " << point.x << ", " << point.y << ", " << point.z << " and position ";
////         std::cout << sph->pos.x << ", " << sph->pos.y << ", " << sph->pos.z << ": " << std::endl;
////         std::cout << "normal: " << n.x << " " << n.y << " " << n.z << std::endl << std::endl;;
////      }
//
//      // halfway vector
//      vec3 h = ((camPos - point).normalized() + lDir).normalized();
//
//      // attenuate with distance from light (1/d^2)
//      float dSquare = pow(toLight.length(), 2);
//
//      // diffuse factor
//      float dMult = fmax(0, dot(n, lDir));
//
//      // specular factor -- v dot r
//      float sMult = pow(fmax(0, dot(n, h)), m.ns);
////      float sMult = pow(fmax(0, dot(v,r.normalized())), s.mat.ns);
//
//
//      // calculate light/material contributions
////      c.r += i.r * ((s.mat.diffuse.r * dMult) / dSquare + (s.mat.specular.r * sMult)/dSquare);
////      c.g += i.g * ((s.mat.diffuse.g * dMult) / dSquare + (s.mat.specular.g * sMult)/dSquare);
////      c.b += i.b * ((s.mat.diffuse.b * dMult) / dSquare + (s.mat.specular.b * sMult)/dSquare);
////
//      sphere* sph = (sphere*) s;
//      if (sph->pos.x == -3) {
//         std::cout << dMult << " " << sMult << std::endl;
//      }
//      c.r += i.r * ((m.diffuse.r * dMult) / dSquare + (m.specular.r * sMult)/dSquare);
//      c.g += i.g * ((m.diffuse.g * dMult) / dSquare + (m.specular.g * sMult)/dSquare);
//      c.b += i.b * ((m.diffuse.b * dMult) / dSquare + (m.specular.b * sMult)/dSquare);
//
////      std::cout << c.r << c.g << c.b << std::endl;
//
//      return c;
//   }
//};
//
//class directionalLight : public light {
//public:
//   directionalLight(Color intensity, vec3 direction) {
//      i = intensity;
//      dir = direction;
//   }
//
////   Color findLight(sphere s, vec3 point) override {
////   Color findLight(sphere s, vec3 point, vec3 v, vec3 r) override {
//   Color findLight(shape* s, vec3 point, vec3 v, vec3 r) override {
//      Color c = Color(0,0,0);
//      vec3 toLight = -1 * dir;
//      vec3 lDir = toLight.normalized();
//      material m = s->mat;
//
////      if (raySphereIntersection(point, lDir).hit) return c;
////      if (s->intersect(point, lDir).hit) return c;
//      if (rayShapeIntersection(point, lDir).hit) return c;
//
//      // normal
////      vec3 n = (point - s.pos).normalized();
//      vec3 n = s->findNormal(point);
//
//      // halfway vector
//      vec3 h = ((camPos - point).normalized() + lDir).normalized();
//
//      // diffuse factor
//      float dMult = fmax(0, dot(n, lDir));
//
//      // specular factor
//      float sMult = pow(fmax(0, dot(n, h)), m.ns);
////      float sMult = pow(fmax(0, dot(v,r.normalized())), s.mat.ns);
//
//      c.r += i.r * (m.diffuse.r * dMult + m.specular.r * sMult);
//      c.g += i.g * (m.diffuse.g * dMult + m.specular.g * sMult);
//      c.b += i.b * (m.diffuse.b * dMult + m.specular.b * sMult);
//
//
//      return c;
//   }
//};
//
//class spotLight : public light {
//public:
//   spotLight(Color intensity, vec3 position, vec3 direction, float angle1, float angle2) {
//      i = intensity;
////      pos = position;
//      p = position;
//      dir = direction;
//      a1 = angle1;
//      a2 = angle2;
//   }
//
//   Color findLight(shape* s, vec3 point, vec3 v, vec3 r) override {
//      return Color(0,0,0);
//   }
//
//};
//#endif

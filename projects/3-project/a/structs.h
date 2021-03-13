#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

#include <vector>
#include "image_lib.h" //Defines an image class and a color class

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

// sphere object
struct sphere {  // position of center, radius, material
   vec3 pos;
   float r;
   material mat;
   sphere(vec3 position, float radius, material m) {
      pos = position;
      r = radius;
      mat = m;
   }
   
   sphere() {}
};

// point of intersection, sphere intersected with
struct intersection {
   bool hit;
   vec3 point;
   vec3 ray;
   sphere s;
   intersection(bool h, vec3 p, sphere sph) {
      hit = h;
      point = p;
      s = sph;
   }
};

 
// lights - general class for point, directional, and spot lights
class light {
public:
   Color i;
//   vec3 pos, dir;
   vec3 p, dir;
   float a1, a2;
   
   // refactor actual calculations out to structs to avoid issues
//   virtual Color diffuse(material mat, vec3 lDir, vec3 n) = 0;
//   virtual Color specular(material mat, vec3 n, vec3 h) = 0;
//   virtual Color findLight(sphere s, vec3 point) = 0;
   virtual Color findLight(sphere s, vec3 point, vec3 v, vec3 r) = 0;
};

class pointLight : public light {
public:
   pointLight(Color intensity, vec3 position) {
      i = intensity;
//      pos = position;
      p = position;
   }
   
//   Color findLight(sphere s, vec3 point) override {
   Color findLight(sphere s, vec3 point, vec3 v, vec3 r) override {
//      std::cout << "find light" << std::endl;
      vec3 toLight = p - point;
      vec3 lDir = toLight.normalized();
      Color c = Color(0,0,0);
      
      // don't add light if point is in shadow
      if (raySphereIntersection(point, lDir).hit) return c;
      
      // normal
      vec3 n = (point - s.pos).normalized();
      
      // halfway vector
      vec3 h = ((pos - point).normalized() + lDir).normalized();
      
//      vec3 v2 = (pos-point).normalized();
      
      // attenuate with distance from light (1/d^2)
      float dSquare = pow(toLight.length(), 2);
      
      // diffuse factor
      float dMult = fmax(0, dot(n, lDir));
      
      // specular factor -- v dot r
      float sMult = pow(fmax(0, dot(n, h)), s.mat.ns);
//      float sMult = pow(fmax(0, dot(v,r.normalized())), s.mat.ns);
//      float sMult = pow(fmax(0, dot(v2,r.normalized())), s.mat.ns);

      // calculate light/material contributions
      c.r += i.r * ((s.mat.diffuse.r * dMult) / dSquare + (s.mat.specular.r * sMult)/dSquare);
      c.g += i.g * ((s.mat.diffuse.g * dMult) / dSquare + (s.mat.specular.g * sMult)/dSquare);
      c.b += i.b * ((s.mat.diffuse.b * dMult) / dSquare + (s.mat.specular.b * sMult)/dSquare);
      
      return c;
   }
};

class directionalLight : public light {
public:
   directionalLight(Color intensity, vec3 direction) {
      i = intensity;
      dir = direction;
   }
   
//   Color findLight(sphere s, vec3 point) override {
   Color findLight(sphere s, vec3 point, vec3 v, vec3 r) override {
      Color c = Color(0,0,0);
      vec3 toLight = -1 * dir;
      vec3 lDir = toLight.normalized();
      
      if (raySphereIntersection(point, lDir).hit) return c;
      
      // normal
      vec3 n = (point - s.pos).normalized();
      
      // halfway vector
      vec3 h = ((pos - point).normalized() + lDir).normalized();
      
      // diffuse factor
      float dMult = fmax(0, dot(n, lDir));
      
      // specular factor
      float sMult = pow(fmax(0, dot(n, h)), s.mat.ns);
//      float sMult = pow(fmax(0, dot(v,r.normalized())), s.mat.ns);
      
      c.r += i.r * (s.mat.diffuse.r * dMult + s.mat.specular.r * sMult);
      c.g += i.g * (s.mat.diffuse.g * dMult + s.mat.specular.g * sMult);
      c.b += i.b * (s.mat.diffuse.b * dMult + s.mat.specular.b * sMult);
      
      return c;
   }
};

class spotLight : public light {
public:
   spotLight(Color intensity, vec3 position, vec3 direction, float angle1, float angle2) {
      i = intensity;
//      pos = position;
      p = position;
      dir = direction;
      a1 = angle1;
      a2 = angle2;
   }
   
   // neither have these
//   Color diffuse(material mat, vec3 lDir, vec3 n) override {
//      return Color(0,0,0);
//   }
//
//   Color specular(material mat, vec3 n, vec3 h) override {
//      Color c = Color(0,0,0);
//
//      return c;
//   }
//   Color findLight(sphere s, vec3 point) override
   Color findLight(sphere s, vec3 point, vec3 v, vec3 r) override
   {return Color(0,0,0);}

};
#endif

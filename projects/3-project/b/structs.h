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
//intersection raySphereIntersection(vec3 pos, vec3 dir);
intersection rayShapeIntersection(vec3 pos, vec3 dir);
Color getColor(intersection, int, vec3);

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

class triangle : public shape {
public:
   vec3 vertices[3];
   
   intersection intersect(vec3 rayPos, vec3 dir) {  // assuming ray starts in front of plane
//      std::cout << "rayPos: " << rayPos.x << " " << rayPos.y << " " << rayPos.z << std::endl;
//      std::cout << "dir: " << dir.x << " " << dir.y << " " << dir.z << std::endl;
      // t = -(p0 dot N + d) / (V dot N)
      // ray plane intersection
//      vec3 n = planeNormal();
      vec3 n = planeNormal(dir);
      if (dot(dir, n) == 0) return intersection(false, vec3(0,0,0), this);  // ray parallel to plane
//      if (dot(dir, n) > 0) n = -1 * n;
      
//      std::cout << "normal: " << n.x << " " << n.y << " " << n.z << std::endl;
      
      float d = -dot(vertices[0], n);
      float t = -(dot(rayPos, n) + d) / dot(dir, n);
      
//      std::cout << "t: " << t << std::endl;
      
      if (t <= displace) return intersection(false, vec3(0,0,0), this);
      
      // test if inside triangle
      vec3 p = rayPos + t*dir;
      vec3 e1 = vertices[1] - vertices[0];
      vec3 e2 = vertices[2] - vertices[1];
      vec3 e3 = vertices[0] - vertices[2];
      
      // barycentric
      float a1 = cross(p-vertices[0], e1).length() / 2;
      float a2 = cross(p-vertices[1], e2).length() / 2;
      float a3 = cross(p-vertices[2], e3).length() / 2;

      float a = cross(e1,e2).length() / 2;

      if (abs(a - (a1+a2+a3)) < 0.0001) {  // hit
         return intersection(true, p, this);
      }
      
      return intersection(false, vec3(0,0,0), this);
   }
   
   vec3 planeNormal(vec3 rayDir) {
      vec3 n = cross((vertices[1] - vertices[0]), (vertices[2] - vertices[0])).normalized();
      if (dot(n, rayDir) > 0) {
         n = cross((vertices[2] - vertices[0]), (vertices[1] - vertices[0])).normalized();
      }
      
      return n;
   };
   
   virtual vec3 findNormal(vec3) = 0;
   
};

class flatTriangle: public triangle {
public:
   vec3 normal;
   flatTriangle(vec3 v0, vec3 v1, vec3 v2, material m) {
      vertices[0] = v0;
      vertices[1] = v1;
      vertices[2] = v2;
      
      mat = m;
      
      normal = planeNormal((v0 - camPos).normalized());
//      std::cout << "normal: " << normal.x << " " << normal.y << " " << normal.z << std::endl;
   }
   
   vec3 findNormal(vec3 hitPoint) override {
      return normal;
   }
   
//   vec3 planeNormal(vec3 rayDir) override {
//   }
};

class normTriangle: public triangle {
public:
   vec3 normals[3];
   normTriangle(vec3 v0, vec3 v1, vec3 v2, vec3 n0, vec3 n1, vec3 n2, material m) {
      vertices[0] = v0;
      vertices[1] = v1;
      vertices[2] = v2;
      
      normals[0] = n0;
      normals[1] = n1;
      normals[2] = n2;
      
      mat = m;
   }
   
//   vec3 planeNormal(vec3 rayDir) override{  // TODO: need to check ordering!! somehow!!
//      return cross((vertices[1] - vertices[0]), (vertices[2] - vertices[0])).normalized();
//   }
   vec3 findNormal(vec3 hitPoint) override {
//      return vec3(0,0,0);
//      return planeNormal((hitPoint - camPos).normalized());
      // edges
      vec3 e1 = vertices[1] - vertices[0];
      vec3 e2 = vertices[2] - vertices[1];
      vec3 e3 = vertices[0] - vertices[2];
      
      // areas
      float a = cross(e1,e2).length() / 2;
      float a1 = cross(hitPoint-vertices[0], e1).length() / 2;
      float a2 = cross(hitPoint-vertices[1], e2).length() / 2;
      float a3 = cross(hitPoint-vertices[2], e3).length() / 2;

      // interpolate normals according to area weights
      return ((a1/a) * normals[2] + (a2/a) * normals[0] + (a3/a) * normals[1]).normalized();
      
      
   }
   
};

 
// lights - general class for point, directional, and spot lights
class light {
public:
   Color i;
   vec3 p, dir;
   float a1, a2;
   
   // refactor actual lighting calculations out to light classes to avoid issues
   virtual Color findLight(shape* s, vec3 point, vec3 v, vec3 r) = 0;
   virtual Color refract(shape* s, vec3 point, vec3 n) = 0;
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
//      if (raySphereIntersection(point, lDir).hit) return c;
      intersection hit = rayShapeIntersection(point, lDir);
      if (hit.hit && toLight.length() > (hit.point - point).length()) return c;
//      if (rayShapeIntersection(point, lDir).hit) return c;
      
      // normal
      vec3 n = s->findNormal(point);
//      if (dot(-1 * lDir, n) < 0) n = -1 * n;
      
      // halfway vector
//      vec3 h = ((camPos - point).normalized() + lDir).normalized();
      vec3 h = (v.normalized() + lDir).normalized();
      
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
   
   Color refract(shape* s, vec3 point, vec3 n) {
      // refraction vector
      vec3 lDir = (p - point).normalized();
      float cosi = dot(n, lDir);
      float thetai = acos(cosi);
      float nr = s->mat.ior;
      // cos(sin-1(theta r)) = (ni/nr) *sin(theta i)
      float thetar = asin(sin(thetai) / nr);
      float cosr = cos(thetar);

      vec3 t = (cosi/nr) * n - (1.0/nr) * lDir;

//      color = color + getColor(rayShapeIntersection(pS, t.normalized()), depth + 1, pS);
      return getColor(rayShapeIntersection(point, t.normalized()), maxDepth, point);
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
//      if (raySphereIntersection(point, lDir).hit) return c;
      if (rayShapeIntersection(point, lDir).hit) return c;
      
      // normal
      vec3 n = s->findNormal(point);
//      if (dot(-1 * lDir, n) > 0) n = -1 * n;
      
      // halfway vector
//      vec3 h = ((camPos - point).normalized() + lDir).normalized();
      vec3 h = (v.normalized() + lDir).normalized();
      
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
   Color refract(shape* s, vec3 point, vec3 n) {
      return Color();
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
   Color refract(shape* s, vec3 point, vec3 n) {
      return Color();
   }
};
#endif

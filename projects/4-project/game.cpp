// Partially adapted from sample code by Dr. Guy

// compile: g++ game.cpp -x c glad/glad.c -g -F/Library/Frameworks -framework SDL2 -framework OpenGL -o game

#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
 #include <SDL2/SDL.h>
 #include <SDL2/SDL_opengl.h>
#else
 #include <SDL.h>
 #include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// represents a grid cell on the map
struct cell {
   glm::vec3 center;
   char status;
   
   cell(glm::vec3 c, char s) {
      center = c;
      status = s;
   }
};

// miscellaneous variables
float screenWidth = 800, screenHeight = 800;

// map variables
int mapWidth, mapHeight, numWalls, numDoors;
std::vector<std::vector<cell>> map;

// model variables
std::vector<float> vertices;
std::vector<glm::vec2> modelBounds;  // vec2's are (offset, length) for each model
std::string modelFiles[] = {"models/cube.txt", "models/teapot.txt"} ;  // indices correspond ^


// reads a map file into the map structure for later handling
void readMapFile() {
   std::ifstream mapFile;
   mapFile.open("maps/map1.txt");
   
   // read parameters
   mapFile >> mapWidth;
   mapFile >> mapHeight;
   
   // read contents
   std::string row;
   int i, j;
   for (i = 0; i < mapHeight; i++) {
      map.push_back(std::vector<cell>());  // initialize row
      mapFile >> row;
      for (j = 0; j < mapWidth; j++) {
         glm::vec3 center = glm::vec3(i+0.5, j+0.5, 0.0);
         map[i].push_back(cell(center, row[j]));
      }
   }
   
   // check file read
   std::cout << "map: " << std::endl;
   for (i = 0; i < map.size(); i++) {
      for (j = 0; j < map[i].size(); j++) {
         std::cout << map[i][j].status;
      }
      std::cout << std::endl;
   }
   
   mapFile.close();
}

// loads models specified in modelFiles array
void loadModels() {
   std::ifstream modelFile;
   float v;
   int offset = 0, numLines = 0, i = 0;
   for(std::string model : modelFiles) {
      modelFile.open(model);
      modelFile >> numLines;
      for (i = 0; i < numLines; i++){
         modelFile >> v;
         vertices.push_back(v);
      }

      // store offset and length of model
      modelBounds.push_back(glm::vec2(offset, numLines / 8));
      offset += (numLines / 8);
      modelFile.close();
   }
   
   // check for correctness
//   for (glm::vec2 b : modelBounds) {
//      std::cout << "offset: " << b.x << " length: " << b.y << std::endl;
//   }
}

int main(int argc, char *argv[]){
   readMapFile();
//   // Set up OpenGL and SDL stuff
//   SDL_Init(SDL_INIT_VIDEO);  // Initialize Graphics (for OpenGL)
//
//   // Ask SDL to get a recent version of OpenGL (3.2 or greater)
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//
//   // Create a window (offsetx, offsety, width, height, flags)
//   SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
//
//   //Create a context to draw in
//   SDL_GLContext context = SDL_GL_CreateContext(window);
//
//   //Load OpenGL extentions with GLAD
//   if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
//      printf("\nOpenGL loaded\n");
//      printf("Vendor:   %s\n", glGetString(GL_VENDOR));
//      printf("Renderer: %s\n", glGetString(GL_RENDERER));
//      printf("Version:  %s\n\n", glGetString(GL_VERSION));
//   }
//   else {
//      printf("ERROR: Failed to initialize OpenGL context.\n");
//      return -1;
//   }

   // Load models, set up associated arrays and variables
   loadModels();
   float* modelData = new float[vertices.size()];  // am unsure about this
   for (int i = 0; i < vertices.size(); i++) {  // copy std::vector into array for vao
      modelData[i] = vertices[i];
   }
   
   // Allocate textures TODO: textures?
   // Create VAO/VBO
   // Build a Vertex Array Object (VAO) to store mapping of shader attributes to VBO
   GLuint vao;
   glGenVertexArrays(1, &vao);  // Create a VAO
   glBindVertexArray(vao);  // Bind the created VAO to the current context
   
   // Allocate memory on the graphics card to store geometry (vertex buffer object)
   GLuint vbo[1];
   glGenBuffers(1, vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);  // Set the vbo as the active array buffer
   glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), modelData, GL_STATIC_DRAW);
   
   
   // Set up shaders
   // Set up VAO offsets/contents/etc
   // Event loop
      // Handle all window events (e.g. key presses)
      // Clear screen
      // Adjust view and projection matrices (might not be needed for user-controlled movement)
      // Do....something with textures - make active? set up so geometry can find them?
      // Bind VAO, draw geometry, double buffer
         // set object color - maybe not needed if texturing only
         // Draw models from VBO given an offset and a length: manipulate model matrix, then draw
            // (can draw same model multiple times here using different model matrices)


//   // Clean Up
//   glDeleteProgram(texturedShader);
//    glDeleteBuffers(1, vbo);
//    glDeleteVertexArrays(1, &vao);
//
//   SDL_GL_DeleteContext(context);
//   SDL_Quit();
//   return 0;
}

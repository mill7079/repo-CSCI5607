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
#include "glm/gtx/rotate_vector.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// map variables, have to be declared before cell struct. here comes the bullshit again
int mapWidth, mapHeight, numWalls, numDoors;

// model variables
std::vector<float> vertices;
std::vector<glm::vec2> modelBounds;  // vec2's are (offset, length) for each model
std::string modelFiles[] = {"models/cube.txt", "models/teapot.txt"} ;  // indices correspond ^
float zOffset = 0;
float lookAngle = 0.f, angleSpeed = 1.8f;
float camRadius = 0.3f;

bool keys[5] = {false, false, false, false, false};

// represents a grid cell on the map
struct cell {
   glm::vec3 center;
   char status;
   
   cell() {};

   cell(glm::vec3 c, char s) {
      center = c;
      status = s;
      
      switch (s) {
         case 'a':
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'S':
         case '0':
            center.z -= 1.f;
            break;
         default:
            break;
      }
      
//      if (s == '0') {
//         center.z -= 1.f;
//      }
   }
   
   void walls(int shader) {  // there's probably a way to condense this but we'll get there
      GLint uniTexID = glGetUniformLocation(shader, "texID");
      GLint uniModel = glGetUniformLocation(shader, "model");
      
      glm::mat4 model = glm::mat4(1);
      bool wall = false;
      if (center.x < 1) {  // add left wall
         wall = true;
         model = glm::translate(model, glm::vec3(center.x - 1, center.y, zOffset));
      } else if (center.x > (mapWidth-1)) {  // add right wall
         wall = true;
         model = glm::translate(model, glm::vec3(center.x + 1, center.y, zOffset));
      }
      
      if (wall) {
         glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
         glUniform1i(uniTexID, 1);
         glDrawArrays(GL_TRIANGLES, modelBounds[0].x, modelBounds[0].y);
      }
      
      wall = false;
      model = glm::mat4(1);
      if (center.y < 1) {  // add lower wall
         wall = true;
         model = glm::translate(model, glm::vec3(center.x, center.y - 1, zOffset));
      } else if (center.y > (mapHeight-1)) {  // add upper wall
         wall = true;
         model = glm::translate(model, glm::vec3(center.x, center.y + 1, zOffset));
      }
      
      if (wall) {
         glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
         glUniform1i(uniTexID, 1);
         glDrawArrays(GL_TRIANGLES, modelBounds[0].x, modelBounds[0].y);
      }
   }
};

// 2D array of cells representing the map
std::vector<std::vector<cell>> map;

// miscellaneous variables
float screenWidth = 850, screenHeight = 850;
//cell start, end;
glm::vec2 cur, end;

// camera variables: position, look at point, and up vector
//glm::vec3 camPos = glm::vec3(3.f, 0.f, 10.f);
glm::vec3 camPos = glm::vec3(2.5f, 2.5f, 10.f);
//glm::vec3 camLook = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camLook = glm::vec3(2.5f, 2.5f, 9.f);
//glm::vec3 camUp = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camRight = normalize(glm::cross((camLook - camPos),camUp));
float moveBy = 2.0f/60.0f;
float rotateSpeed = 100000.0f;

// this is a distance function but if i call it distance things die so asdf it is
float asdf(glm::vec2 a, glm::vec2 b) {
//   std::cout << ";AOSKJDFA;LSDKJFA;SDJFALSDKJF;ASLDJF" << std::endl;
   float d = std::sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
//   std::cout << "distance: " << d << std::endl;
   return d;
}


// reads a map file into the map structure for later handling
void readMapFile() {
   std::ifstream mapFile;
   mapFile.open("maps/map2.txt");
   
   // read parameters
   mapFile >> mapWidth;
   mapFile >> mapHeight;
   
   // read contents
   std::string row;
   int i, j;
   for (i = 0; i < mapHeight; i++) {
      map.push_back(std::vector<cell>());  // initialize rows
   }
//   for (i = 0; i < mapHeight; i++) {
   for (i = mapHeight - 1; i >= 0; i--) {
      mapFile >> row;
      for (j = 0; j < mapWidth; j++) {
         glm::vec3 center = glm::vec3(j+0.5, i+0.5, zOffset);
//         glm::vec3 center = glm::vec3(i+0.5, zOffset, j+0.5);
         cell c = cell(center, row[j]);
         map[i].push_back(c);
         
         switch (c.status) {
            case 'S':
               cur = glm::vec2(i, j);
               camPos = c.center;
               camPos.z += 1.f;
               camUp = glm::vec3(0.f, 0.f, 1.f);
               camLook = c.center;
               camLook.x += 1.f;
               camLook.z += 1.f;
               break;
            case 'G':
               end = glm::vec2(i, j);
               break;
            default:
               break;
         }
//         std::cout << center.x << "," << center.y << " ";
      }
//      std::cout << std::endl;
   }
   
   // check file read
   std::cout << "map: " << std::endl;
//   for (i = 0; i < map.size(); i++) {
   for (i = map.size() - 1; i >= 0; i--) {
      for (j = 0; j < map[i].size(); j++) {
         std::cout << map[i][j].status;
      }
      std::cout << std::endl;
   }
   
   mapFile.close();
   
   std::cout << cur.x << " " << cur.y << std::endl;
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
   
   // test correctness
//   for (glm::vec2 b : modelBounds) {
//      std::cout << "offset: " << b.x << " length: " << b.y << std::endl;
//   }
}

// draw each model in level
void draw(int shader) {
   // set object color - maybe not needed if texturing only
   GLint color = glGetUniformLocation(shader, "inColor");
   glm::vec3 colVec(1,0,0);
   glUniform3fv(color, 1, glm::value_ptr(colVec));
   
   GLint uniTexID = glGetUniformLocation(shader, "texID");
   GLint uniModel = glGetUniformLocation(shader, "model");
   
   // draw map
   glm::mat4 model;
   for (std::vector<cell> row : map) {
      for (cell c : row) {
         model = glm::mat4(1);
         model = glm::translate(model, c.center);
         glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
         
         int tex;
         switch (c.status) {
            case 'W':
               tex = 1;
               break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'S':
            case '0':
               tex = 0;
               break;
//            case 'S':
//               tex = -1;
//               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(0,1,0)));
//               break;
            case 'G':
               tex = -1;
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(1,0,0)));
               break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
               tex = 2;
               break;
            default:
               tex = -1;
               break;
         }
         glUniform1i(uniTexID, tex);
         glDrawArrays(GL_TRIANGLES, modelBounds[0].x, modelBounds[0].y);
         
         // draw outer walls
         c.walls(shader);
         
         // draw key models
         model = glm::mat4(1);
         model = glm::translate(model, glm::vec3(c.center.x, c.center.y, c.center.z + 0.8f));
         model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
         glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
         tex = -1;
         bool key = true;
         switch (c.status) {
            case 'a':
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(1,0,0)));
               break;
            case 'b':
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(0,1,0)));
               break;
            case 'c':
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(0,0,1)));
               break;
            case 'd':
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(1,0,1)));
               break;
            case 'e':
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(1,1,0)));
               break;
            default:
               key = false;
               break;
         }
         
         if (key) {
            glUniform1i(uniTexID, tex);
            glDrawArrays(GL_TRIANGLES, modelBounds[1].x, modelBounds[1].y);
         }
      }
   }
}

bool isKey(char c) {
   return c >= 95 && c <= 101;
}

// Attempt to move camera, handling possible collisions and updating the current cell
void moveCell(glm::vec3 moveCam) {
   glm::vec3 newPos = camPos + moveCam * moveBy;
   float dist = asdf(map[cur.x][cur.y].center, newPos);
   int i, j, rNew, cNew;
   float newDist, minDist = dist;
   glm::vec2 newCell = cur;
   
   // check all neighboring cells for distance
   for (i = -1; i <= 1; i++) {
      for (j = -1; j <= 1; j++) {
         if (i == 0 && j == 0) continue;
         
         rNew = cur.x + i, cNew = cur.y + j;
         if (rNew < map.size() && rNew >= 0 &&
             cNew < map[rNew].size() && cNew >= 0) {
//            newDist = asdf(map[rNew][cNew].center, newPos);
            newDist = asdf(map[rNew][cNew].center, newPos) - camRadius;
            if (newDist < minDist) {
               minDist = newDist;
               newCell = glm::vec2(rNew, cNew);
            }
         }
      }
   }
   
   if (newPos.x - camRadius < 0 || newPos.x + camRadius >= mapWidth || newPos.y - camRadius < 0 || newPos.y + camRadius >= mapHeight) return;
   
   // don't move into wall if there's a collision
   char status = map[newCell.x][newCell.y].status;
   if (status != 'W' && (keys[status-65] || (status < 65 || status > 69))) {
      camPos = newPos;
      camLook += moveCam * moveBy;
   
//      if (newCell != cur) {
//         std::cout << "old cell: " << cur.x << " " << cur.y << std::endl;
//         std::cout << "new cell: " << newCell.x << " " << newCell.y << std::endl;
//
//      }
      cur = newCell;
   } else {  // move along wall
      glm::vec3 movement = moveCam * moveBy, axis;
      if (glm::length(movement) == 0) return;
      if (newCell.x != cur.x) {  // vertical wall; wall off to side
         axis = glm::vec3(1.f, 0.f, 0.f);
      } else if (newCell.y != cur.y) {  // horizontal wall; wall above or below
         axis = glm::vec3(0.f, 1.f, 0.f);
      }
      
      if (glm::dot(movement, axis) < 0) axis *= -1.f;
      
//      std::cout << "axis: " << axis.x << " " << axis.y << std::endl;
      float m = glm::dot(movement, axis);
      
      camPos += m*axis;
      camLook += m*axis;
      
      moveCell(glm::vec3(0,0,0));
   }
   
   if (isKey(status)) {
//      std::cout << "status: " << status << " -97: " << (status-97) << std::endl;
      keys[(status - 97)] = true;
//      std::cout << keys[0] << keys[1] << keys[2] << keys[3] << keys[4] << std::endl;
   }
}

// Prints a variety of location information. Used for help with debugging collisions
void debug() {
   std::cout << "current cell: " << cur.x << " " << cur.y << std::endl;
   std::cout << "current cell center: " << map[cur.x][cur.y].center.x << " " << map[cur.x][cur.y].center.y << std::endl;
   std::cout << "current camPos: " << camPos.x << " " << camPos.y << std::endl;
   std::cout << "distances:" << std::endl;
   for (int i = map.size() - 1; i >= 0; i--) {
      for (int j = 0; j < map[i].size(); j++) {
//         printf("%.2f ", glm::length(map[i][j].center-camPos));
         printf("%.2f ", asdf(map[i][j].center, camPos));
      }
      std::cout << std::endl;
   }
   
   std::cout << "centers: " << std::endl;
   for (int i = map.size() - 1; i >= 0; i--) {
      for (int j = 0; j < map[i].size(); j++) {
         glm::vec2 c = map[i][j].center;
         std::cout << c.x << "," << c.y << "  ";
      }
      std::cout << std::endl;
   }
}

GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);

int main(int argc, char *argv[]){
   // Set up OpenGL and SDL stuff
   SDL_Init(SDL_INIT_VIDEO);  // Initialize Graphics (for OpenGL)

   // Ask SDL to get a recent version of OpenGL (3.2 or greater)
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

   // Create a window (offsetx, offsety, width, height, flags)
   SDL_Window* window = SDL_CreateWindow("Maze Game", 0, 0, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

   //Create a context to draw in
   SDL_GLContext context = SDL_GL_CreateContext(window);

   //Load OpenGL extentions with GLAD
   if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
      printf("\nOpenGL loaded\n");
      printf("Vendor:   %s\n", glGetString(GL_VENDOR));
      printf("Renderer: %s\n", glGetString(GL_RENDERER));
      printf("Version:  %s\n\n", glGetString(GL_VERSION));
   }
   else {
      printf("ERROR: Failed to initialize OpenGL context.\n");
      return -1;
   }

   
   // Read map file to set up map
   readMapFile();
   
   // Load models, set up associated arrays and variables
   loadModels();
   float* modelData = new float[vertices.size()];  // am unsure about this
   for (int i = 0; i < vertices.size(); i++) {  // copy std::vector into array for vao
      modelData[i] = vertices[i];
   }
   
   
   
   
   // Allocate textures TODO: textures?
   
   //// Allocate Texture 0 (Wood) ///////
   SDL_Surface* surface = SDL_LoadBMP("textures/wood.bmp");
   if (surface==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex0;
    glGenTextures(1, &tex0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w,surface->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture
    
    SDL_FreeSurface(surface);
    //// End Allocate Texture ///////


   //// Allocate Texture 1 (Brick) ///////
   SDL_Surface* surface1 = SDL_LoadBMP("textures/brick.bmp");
   if (surface1==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex1;
    glGenTextures(1, &tex1);
    
    //Load the texture into memory
    glActiveTexture(GL_TEXTURE1);
    
    glBindTexture(GL_TEXTURE_2D, tex1);
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //How to filter
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface1->w,surface1->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface1->pixels);
    glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture
    
    SDL_FreeSurface(surface1);
   //// End Allocate Texture ///////
   
   
   // allocate door texture
   SDL_Surface* surface2 = SDL_LoadBMP("textures/door.bmp");
   if (surface2==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex2;
    glGenTextures(1, &tex2);
    
    //Load the texture into memory
    glActiveTexture(GL_TEXTURE2);
    
    glBindTexture(GL_TEXTURE_2D, tex2);
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //How to filter
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface2->w,surface2->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface2->pixels);
    glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture
    
    SDL_FreeSurface(surface2);
   //// End Allocate Texture ///////
   
   
   
   
   
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
   
   // Set up shaders TODO: is there a better way to do this that doesn't involve copying lots of code from sample code?
//   int shader = InitShader("textured-Vertex.glsl", "fragment.glsl");
   int shader = InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");
   
   // Set up VAO offsets/contents/etc; also from Dr. Guy's sample code
   // Tell OpenGL how to set fragment shader input
   GLint posAttrib = glGetAttribLocation(shader, "position");
   glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
     // Attribute, vals/attrib., type, isNormalized, stride, offset
   glEnableVertexAttribArray(posAttrib);
   
   GLint normAttrib = glGetAttribLocation(shader, "inNormal");
   glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
   glEnableVertexAttribArray(normAttrib);
   
   GLint texAttrib = glGetAttribLocation(shader, "inTexcoord");
   glEnableVertexAttribArray(texAttrib);
   glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

   GLint uniView = glGetUniformLocation(shader, "view");
   GLint uniProj = glGetUniformLocation(shader, "proj");

   glBindVertexArray(0);  // unbind the VAO
   glEnable(GL_DEPTH_TEST);
   
   // Event loop
   SDL_Event windowEvent;
   bool quit = false;
   while (!quit){
      // Handle all window events (e.g. key presses)
      float prevX = 0.0f, prevY = 0.0f, w = 0.0f;
//      lookAngle = 0.0f;
      while (SDL_PollEvent(&windowEvent)){
         if (windowEvent.type == SDL_QUIT) quit = true;
         if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
            quit = true;
         if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_y)
            debug();
         
         if (windowEvent.type == SDL_KEYDOWN) {
            glm::vec3 moveCam = glm::vec3(0,0,0);
            if (windowEvent.key.keysym.sym == SDLK_a) {  // left
               moveCam = -camRight;
            } else if (windowEvent.key.keysym.sym == SDLK_d) {  // right
               moveCam = camRight;
            } else if (windowEvent.key.keysym.sym == SDLK_w) {  // up
               moveCam = camUp;
            } else if (windowEvent.key.keysym.sym == SDLK_s) {  // down
               moveCam = -camUp;
            } else if (windowEvent.key.keysym.sym == SDLK_UP) {  // in
               moveCam = normalize(camLook - camPos);
            } else if (windowEvent.key.keysym.sym == SDLK_DOWN) {  // out
               moveCam = -normalize(camLook - camPos);
            } else if (windowEvent.key.keysym.sym == SDLK_LEFT) {  // rotate left
               w += angleSpeed;
            } else if (windowEvent.key.keysym.sym == SDLK_RIGHT) {  // rotate right
               w -= angleSpeed;
            } else {
               moveCam = glm::vec3(0,0,0);
            }
            
            lookAngle += w * 1/60.0f;
            
            camLook.x = camPos.x + std::cos(lookAngle);
            camLook.y = camPos.y + std::sin(lookAngle);
            
            // handles movement and cell collisions
            moveCell(moveCam);
            
//            if (checkPos()) {
//               camPos += moveCam * moveBy/60.0f;
//               camLook += moveCam * moveBy/60.0f;
//            }
            
//            camLook.x = camPos.x + std::sin(lookAngle);
//            camLook.y = camPos.y + std::cos(lookAngle);
//            glm::vec3 camRight = normalize(glm::cross((camLook - camPos),camUp));
         }
         
//         if (windowEvent.type == SDL_MOUSEMOTION && windowEvent.button.button == SDL_BUTTON_LEFT) {
////            moveX += windowEvent.motion.x;
//            float moveX = windowEvent.motion.x - prevX;
//            prevX = windowEvent.motion.x;
//            std::cout << moveX << std::endl;
//            camLook = glm::rotate(camLook, moveX/rotateSpeed, camUp);
//        }
      }
      
      // Clear screen
      glClearColor(.2f, 0.4f, 0.8f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      // Specify shader to use
      glUseProgram(shader);
      
      // Adjust view and projection matrices (might not be needed for user-controlled movement) (but I think SOMETHING still needs to be passed in)
//      lookAngle = std::atan((camLook.y - camPos.y)/(camLook.x - camPos.x));
//      camLook = camPos + std::cos(lookAngle);
//      camLook = camPos + std::sin(lookAngle);
      glm::mat4 view = glm::lookAt(camPos,  // Cam Position
                                   camLook,   // Look at point
                                   camUp);  // Up
      glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

      glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, 0.1f, 30.0f);  // FOV, aspect, near, far; far used to be 10.0f but that seemed short
      glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
      
      
      
      
      // make textures available to shader TODO: is there a way to make this shit an array?
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex0);
      glUniform1i(glGetUniformLocation(shader, "tex0"), 0);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, tex1);
      glUniform1i(glGetUniformLocation(shader, "tex1"), 1);
      
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, tex2);
      glUniform1i(glGetUniformLocation(shader, "tex2"), 2);
      
      
      
      
      // Bind VAO, draw geometry, double buffer
      glBindVertexArray(vao);
      draw(shader);
         // Draw models from VBO given an offset and a length: manipulate model matrix, then draw
            // (can draw same model multiple times here using different model matrices)
      SDL_GL_SwapWindow(window);
   }

   // Clean up
   glDeleteProgram(shader);
   glDeleteBuffers(1, vbo);
   glDeleteVertexArrays(1, &vao);

   SDL_GL_DeleteContext(context);
   SDL_Quit();
   delete modelData;
   return 0;
}




/// CODE FROM DR. GUY'S SAMPLE CODE

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile){
   FILE *fp;
   long length;
   char *buffer;

   // open the file containing the text of the shader code
   fp = fopen(shaderFile, "r");

   // check for errors in opening the file
   if (fp == NULL) {
      printf("can't open shader source file %s\n", shaderFile);
      return NULL;
   }

   // determine the file size
   fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
   length = ftell(fp);  // return the value of the current position

   // allocate a buffer with the indicated number of bytes, plus one
   buffer = new char[length + 1];

   // read the appropriate number of bytes from the file
   fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
   fread(buffer, 1, length, fp); // read all of the bytes

   // append a NULL character to indicate the end of the string
   buffer[length] = '\0';

   // close the file
   fclose(fp);

   // return the string
   return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName){
   GLuint vertex_shader, fragment_shader;
   GLchar *vs_text, *fs_text;
   GLuint program;

   // check GLSL version
   printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

   // Create shader handlers
   vertex_shader = glCreateShader(GL_VERTEX_SHADER);
   fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

   // Read source code from shader files
   vs_text = readShaderSource(vShaderFileName);
   fs_text = readShaderSource(fShaderFileName);

   // error check
   if (vs_text == NULL) {
      printf("Failed to read from vertex shader file %s\n", vShaderFileName);
      exit(1);
   }
//   else if (DEBUG_ON) {
//      printf("Vertex Shader:\n=====================\n");
//      printf("%s\n", vs_text);
//      printf("=====================\n\n");
//   }
   if (fs_text == NULL) {
      printf("Failed to read from fragent shader file %s\n", fShaderFileName);
      exit(1);
   }
//   else if (DEBUG_ON) {
//      printf("\nFragment Shader:\n=====================\n");
//      printf("%s\n", fs_text);
//      printf("=====================\n\n");
//   }

   // Load Vertex Shader
   const char *vv = vs_text;
   glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
   glCompileShader(vertex_shader); // Compile shaders
   
   // Check for errors
   GLint  compiled;
   glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
   if (!compiled) {
      printf("Vertex shader failed to compile:\n");
//      if (DEBUG_ON) {
         GLint logMaxSize, logLength;
         glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
         printf("printing error message of %d bytes\n", logMaxSize);
         char* logMsg = new char[logMaxSize];
         glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
         printf("%d bytes retrieved\n", logLength);
         printf("error message: %s\n", logMsg);
         delete[] logMsg;
//      }
      exit(1);
   }
   
   // Load Fragment Shader
   const char *ff = fs_text;
   glShaderSource(fragment_shader, 1, &ff, NULL);
   glCompileShader(fragment_shader);
   glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
   
   //Check for Errors
   if (!compiled) {
      printf("Fragment shader failed to compile\n");
//      if (DEBUG_ON) {
         GLint logMaxSize, logLength;
         glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
         printf("printing error message of %d bytes\n", logMaxSize);
         char* logMsg = new char[logMaxSize];
         glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
         printf("%d bytes retrieved\n", logLength);
         printf("error message: %s\n", logMsg);
         delete[] logMsg;
//      }
      exit(1);
   }

   // Create the program
   program = glCreateProgram();

   // Attach shaders to program
   glAttachShader(program, vertex_shader);
   glAttachShader(program, fragment_shader);

   // Link and set program to use
   glLinkProgram(program);

   return program;
}

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

// map variables, have to be declared before cell struct. here comes the bullshit again
int mapWidth, mapHeight, numWalls, numDoors;

// model variables
std::vector<float> vertices;
std::vector<glm::vec2> modelBounds;  // vec2's are (offset, length) for each model
std::string modelFiles[] = {"models/cube.txt", "models/teapot.txt"} ;  // indices correspond ^
float zOffset = 0;

// represents a grid cell on the map
struct cell {
   glm::vec3 center;
   char status;

   cell(glm::vec3 c, char s) {
      center = c;
      status = s;
      
      if (s == '0') {
         center.z -= 1.f;
      }
   }
   
   void walls(int shader) {
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
float screenWidth = 800, screenHeight = 800;

// camera variables: position, look at point, and up vector
//glm::vec3 camPos = glm::vec3(3.f, 0.f, 10.f);
glm::vec3 camPos = glm::vec3(2.5f, 2.5f, 10.f);
//glm::vec3 camLook = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camLook = glm::vec3(2.5f, 2.5f, 9.f);
//glm::vec3 camUp = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camRight = normalize(glm::cross((camLook - camPos),camUp));
float moveBy = 1.0f;


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
      map.push_back(std::vector<cell>());  // initialize rows
   }
//   for (i = 0; i < mapHeight; i++) {
   for (i = mapHeight - 1; i >= 0; i--) {
      mapFile >> row;
      for (j = 0; j < mapWidth; j++) {
         glm::vec3 center = glm::vec3(j+0.5, i+0.5, zOffset);
//         glm::vec3 center = glm::vec3(i+0.5, zOffset, j+0.5);
         map[i].push_back(cell(center, row[j]));
//         std::cout << center.x << "," << center.y << " ";
      }
//      std::cout << std::endl;
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
            case '0':
               tex = 0;
               break;
            case 'S':
               tex = -1;
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(0,1,0)));
               break;
            case 'G':
               tex = -1;
               glUniform3fv(color, 1, glm::value_ptr(glm::vec3(1,0,0)));
               break;
            default:
               tex = -1;
               break;
         }
         glUniform1i(uniTexID, tex);
         glDrawArrays(GL_TRIANGLES, modelBounds[0].x, modelBounds[0].y);
         
         c.walls(shader);
      }
   }
   
//   for (int i = -1; i <= mapHeight; i++) {
//      for (int j = -1; j <= mapWidth; j++) {
//         if (i == -1 || j == -1 || i == mapHeight || j == mapWidth) {
//            model = glm::mat4(1);
//            model = glm::translate(model, glm::vec3(i, j, zOffset));
//         }
//      }
//   }
   
//   // pass model matrix to shader
//   glm::mat4 model = glm::mat4(1);
//   model = glm::translate(model,glm::vec3(-2,-1,-.4));
////   GLint uniModel = glGetUniformLocation(shader, "model");
//   glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
//
//   // texture
////   GLint uniTexID = glGetUniformLocation(shader, "texID");
//   glUniform1i(uniTexID, -1);
//
//   // draw model
//   glDrawArrays(GL_TRIANGLES, modelBounds[0].x, modelBounds[0].y);
////   std::cout << modelBounds[0].x << " " << modelBounds[0].y << std::endl;
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
   SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

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
   SDL_Surface* surface = SDL_LoadBMP("wood.bmp");
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
   SDL_Surface* surface1 = SDL_LoadBMP("brick.bmp");
   if (surface==NULL){ //If it failed, print the error
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
      while (SDL_PollEvent(&windowEvent)){
         if (windowEvent.type == SDL_QUIT) quit = true;
         if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
            quit = true;
         
         if (windowEvent.type == SDL_KEYDOWN) {
            glm::vec3 moveCam;
            if (windowEvent.key.keysym.sym == SDLK_a) {  // left
               moveCam = -camRight;
            } else if (windowEvent.key.keysym.sym == SDLK_d) {  // right
               moveCam = camRight;
            } else if (windowEvent.key.keysym.sym == SDLK_w) {  // up
               moveCam = camUp;
            } else if (windowEvent.key.keysym.sym == SDLK_s) {  // down
               moveCam = -camUp;
            } else if (windowEvent.key.keysym.sym == SDLK_q) {  // in
               moveCam = normalize(camLook - camPos);
            } else if (windowEvent.key.keysym.sym == SDLK_e) {  // out
               moveCam = -normalize(camLook - camPos);
            } else {
               moveCam = glm::vec3(0,0,0);
            }
            camPos += moveCam * moveBy/60.0f;
            camLook += moveCam * moveBy/60.0f;
         }
      }
      
      // Clear screen
      glClearColor(.2f, 0.4f, 0.8f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      // Specify shader to use
      glUseProgram(shader);
      
      // Adjust view and projection matrices (might not be needed for user-controlled movement) (but I think SOMETHING still needs to be passed in)
      // TODO: move lookAt params out to variables
      glm::mat4 view = glm::lookAt(camPos,  // Cam Position
                                   camLook,   // Look at point
                                   camUp);  // Up
      glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

      glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, 1.0f, 30.0f);  // FOV, aspect, near, far; far used to be 10.0f but that seemed short
      glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
      
      
      
      
      // Do....something with textures - make active? set up so geometry can find them? TODO: is there a way to make this shit an array?
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex0);
      glUniform1i(glGetUniformLocation(shader, "tex0"), 0);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, tex1);
      glUniform1i(glGetUniformLocation(shader, "tex1"), 1);
      
      
      
      
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

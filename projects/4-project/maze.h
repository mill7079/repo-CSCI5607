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
#include <random>
#include <functional>
#include <list>

struct mcell {
   bool visited;
   bool right;
   bool down;
   char item;
   
   mcell() {
      visited = false;
      right = true;
      down = true;
      item = '!';
   }
};

std::vector<std::vector<mcell>> maze;

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(0, 24);
auto rnd = std::bind(distribution, generator);

std::string createFileName(int rows, int cols) {
   // create file name
   std::string fileName = "genmaps/maze";
   fileName += ("" + rows);
   fileName += "x";
   fileName += ("" + cols);
   fileName += ".txt";
   
   std::cout << "file name: " << fileName << std::endl;
   
   return fileName;
}

// solveMaze algorithm; place keys in dead ends and doors afterward
void placeDoors() {
   // reset all cells to unvisited
   for (int i = 0; i < maze.size(); i++) {
      for (int j = 0; j < maze[i].size(); j++) {
         maze[i][j].visited = false;
      }
   }
   
   // create vector of doors and keys
   std::vector<char> doors = {'E', 'e', 'D', 'd', 'C', 'c', 'B', 'b', 'A', 'a'};
   
   // bfs
   std::list<glm::vec2> q;
   q.push_back(glm::vec2(0,0));
   while (!q.empty()) {
      glm::vec2 cur = q.front();
      q.pop_front();
      int r = cur.x, c = cur.y;
      maze[r][c].visited = true;

      // found end point
      if (r == maze.size() - 1 && c == maze[r].size() - 1) break;
      
      // add door if key was added
      if (!doors.empty() && doors.size() % 2 == 1) {
         maze[r][c].item = doors.back();
         doors.pop_back();
      }

      // add all reachable AND UNVISITED neighbors
      int count = 0;
      if (r-1 >= 0 && !maze[r-1][c].visited && !maze[r-1][c].down) {  // upper neighbor
         count++;
         q.push_back(glm::vec2(r-1,c));
      }
      if (r+1 < maze.size() && !maze[r+1][c].visited && !maze[r][c].down) {  // lower neighbor
         count++;
         q.push_back(glm::vec2(r+1, c));
      }
      if (c-1 >= 0 && !maze[r][c-1].visited && !maze[r][c-1].right) {  // left neighbor
         count++;
         q.push_back(glm::vec2(r, c-1));
      }
      if (c+1 < maze[r].size() && !maze[r][c+1].visited && !maze[r][c].right) {  // right neighbor
         count++;
         q.push_back(glm::vec2(r, c+1));
      }
      
      // dead end; no reachable unvisited neighbors - add a key
      if (count == 0 && !doors.empty()) {
         maze[r][c].item = doors.back();
         doors.pop_back();
      }
   }
}

void generateMazeFile(std::string file, int rows, int cols) {
//   std::string output = "";
   int r = 2*rows+1, c = 2*cols+1;
   char print[r][c];
   
   // first draw maze with all walls
   int i, j;
   for (i = 0; i < r; i += 2) {  // horizontal border
      for (j = 1; j < c; j++) {
          print[i][j] = 'W';
      }
   }
   
   for (i = 0; i < r; i++) {  // vertical border
      for (j = 0; j < c; j += 2) {
          print[i][j] = 'W';
      }
  }
   
   for (i = 1; i < r; i += 2) {  // floor in spaces
      for (j = 1; j < c; j += 2){
          print[i][j] = '0';
      }
   }
   
   // remove walls according to maze
   for (i = 0; i < maze.size(); i++) {
      for (j = 0; j < maze[i].size(); j++) {
         if (!maze[i][j].down) {  // bottom
              print[2*(i+1)][2*j + 1] = '0';
         }
         if (!maze[i][j].right) {  // right
              print[2*i + 1][2*(j+1)] = '0';
         }
         if (maze[i][j].item != '!') {
            print[2*i + 1][2*j + 1] = maze[i][j].item;  // add keys/doors
         }
      }
   }
   
   print[1][1] = 'S';
   print[r-2][c-2] = 'G';
   
   // print the maze to the file
   std::ofstream newMaze;
   newMaze.open(file);
   std::cout << "file: " << file << std::endl;
   newMaze << c << " " << r << std::endl;
   for (i = 0; i < r; i++) {
      for (j = 0; j < c; j++) {
//         std::cout << print[i][j];
         newMaze << print[i][j];
      }
      newMaze << std::endl;
   }
   newMaze.close();
}

std::string makeMaze(int rows, int cols) {
//   std::cout << "asdfasdfasdf" << std::endl;
   std::cout << "in make maze" << std::endl;
   maze.clear();
   // initialize cells
   for (int i = 0; i < rows; i++) {
      maze.push_back(std::vector<mcell>());
      for (int j = 0; j < cols; j++) {
         maze[i].push_back(mcell());
      }
   }
   
   std::cout << "initialized cells" << std::endl;
   
   std::vector<glm::vec2> stack;
   stack.push_back(glm::vec2(0,0));
   std::cout << "init stack" << std::endl;
   maze[0][0].visited = true;
   std::cout << "1st cell true" << std::endl;
   while (!stack.empty()) {
      // get top of stack
      glm::vec2 cur = stack.back();
      
      // choose random unvisited neighbor
      glm::vec2 neighbors[4];
      int count = 0;
      if (cur.y-1 >= 0 && !maze[cur.x][cur.y-1].visited) {  // left
         neighbors[count] = (glm::vec2(cur.x, cur.y-1));
         count++;
      }
      if (cur.x-1 >= 0 && !maze[cur.x-1][cur.y].visited) {  // top
         neighbors[count] = (glm::vec2(cur.x-1, cur.y));
         count++;
      }
      if (cur.y+1 < cols && !maze[cur.x][cur.y+1].visited) {  // right
         neighbors[count] = (glm::vec2(cur.x, cur.y+1));
         count++;
      }
      if (cur.x+1 < rows && !maze[cur.x+1][cur.y].visited) {  // bottom
         neighbors[count] = (glm::vec2(cur.x+1, cur.y));
         count++;
      }
      
      if (count == 0) {
         stack.pop_back();
         continue;
      }
      
//      int n = std::rand() % static_cast<int>(count);
//      int n = distribution(generator);
      int n = rnd() % count;
      
//      std::cout << "n : "  << n << " count: " << count << std::endl;
//      glm::vec2 neigh = neighbors[n];
      int nx = neighbors[n].x;
      int ny = neighbors[n].y;
      
      // add index to stack, mark as visited
      stack.push_back(neighbors[n]);
      maze[nx][ny].visited = true;
      
      // remove wall
      if (nx != cur.x) {
         if (nx > cur.x) {
            maze[cur.x][cur.y].down = false;
         } else {
            maze[nx][cur.y].down = false;
         }
      } else {
         if (ny > cur.y) {
            maze[cur.x][cur.y].right = false;
         } else {
            maze[cur.x][ny].right = false;
         }
      }
   }
   
   placeDoors();
   
//   std::string file = createFileName(rows, cols);
   std::string file = "genmaps/newmaze.txt";
   generateMazeFile(file, rows, cols);
   
   return file;
}

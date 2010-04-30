#ifndef CJ_WALL_HPP
#define CJ_WALL_HPP

#include <iostream>
#include <vector>
#include <list>
#include <stdexcept>
#include <irrlicht.h>

using namespace std;
using namespace irr;
using namespace video;
using namespace scene;
using namespace gui;
using namespace core;

namespace cj
{

struct GraphNode
{
   vector3df point;
   GraphNode* connection[8];
   GraphNode* back;
   s32 backdir;
   f32 score;
   f32 count;
   s32 isWall;
   bool closed;
};

const f32 SQRT2 = sqrt(2);

class WallException
{
   public:
	  WallException(const char *msg);
      ~WallException();
      const char *Message(void) const;

   private:
	  const char *description;
};

class Wall
{
   public:
	static Wall& Instance();
      Wall(IrrlichtDevice* d, stringw t, u32 dsize = 10);
      ~Wall();
      void makeWall(u32 length, u32 width, vector3df position); //create a long wall
      void DrawNodes(void);
      GraphNode* FindCloseNode(s32 x, s32 z);
      std::list<vector3df> AStar(vector3df start, vector3df goal, s32 smooth, bool debug = false,
         vector<IBillboardSceneNode*> *circles = NULL);
      bool PathIsWide(vector3df from, vector3df to);
      bool operator== (const Wall& rhs) const;
      vector3df getRandomNodePosition(); // Returns the coords for a random node in the graph.
   private:
    static Wall* singleton;
      Wall(const Wall& other);
      Wall operator= (const Wall& rhs) const;

      void addNode(u32 size, vector3df position); //creates a cube of size and places it at position
      void ExpandSpace(vector3df a);
      void InsertPath(s32 x, s32 z);
      GraphNode* NotWall(vector3df p);
      void InsertList(std::list<GraphNode*> &glist, GraphNode* node, bool remove = false);
      GraphNode* FindNode(s32 x, s32 z);

      vector<IMeshSceneNode*> frame; //contains cubes created by addNode
      IrrlichtDevice *device; //get driver and smgr from this
      IVideoDriver* driver; //for drawing
      ISceneManager* smgr; //needed for adding cubes
      stringw texture; //path of texture file
      vector<GraphNode*> paths;
      s32 xpos;
      s32 xneg;
      s32 zpos;
      s32 zneg;
      s32 zrange;
      s32 dsize;
      static bool wall_exists;
};

}

#endif

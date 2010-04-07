#ifndef CJ_WALL_HPP
#define CJ_WALL_HPP

#include <vector>
#include <irrlicht.h>

using namespace std;
using namespace irr;
using namespace video;
using namespace scene;
using namespace gui;
using namespace core;

namespace cj
{

class Wall
{
    public:
        Wall(IrrlichtDevice* d, stringw t);
        ~Wall();
        void addNode(u32 size, vector3df position); //creates a cube of size and places it at position
        void makeWall(u32 length, u32 width, vector3df position); //create a long wall

    private:
        std::vector<IMeshSceneNode*> frame; //contains cubes created by addNode
        IrrlichtDevice *device; //get driver and smgr from this
        IVideoDriver* driver; //for drawing
        ISceneManager* smgr; //needed for adding cubes
        stringw texture; //path of texture file
};

}

#endif

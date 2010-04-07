#include "Wall.hpp"

namespace cj
{

Wall::Wall(IrrlichtDevice* d, stringw t) : device(d), texture(t)
{
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
}

Wall::~Wall()
{}

void Wall::addNode(u32 size, vector3df position)
// creates a cube of size and places it at position
{
    // add a wall node to the list
    frame.push_back(smgr->addCubeSceneNode(size, 0, -1, position));
    // set the texture
    frame[frame.size()-1]->setMaterialTexture(0, driver->getTexture(texture));
    // because we do not have a light source, lighting needs to be off or it will be black
    frame[frame.size()-1]->setMaterialFlag(EMF_LIGHTING, false);
    // create selector for collision detection
    ITriangleSelector* selector = smgr->createTriangleSelector(
        frame[frame.size()-1]->getMesh(), frame[frame.size()-1]);
    // attatch selector to wall node
    frame[frame.size()-1]->setTriangleSelector(selector);
    // discard the selector reference
    selector->drop();
}

void Wall::makeWall(u32 length, u32 width, vector3df position)
{
    for(f32 l = 0; l < length; l++)
    {
        for(f32 w = 0; w < width; w++)
        {  addNode(10, position - vector3df(l*10,0,w*10));  }
    }
}

}

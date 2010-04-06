#include "Wall.hpp"
//#include "assert_swig.hpp"
#include <cassert>

namespace cj {

// id=ctor
Wall::Wall(IrrlichtDevice* d, stringw t, vector3df lO) : localOrigin(lO), device(d), texture(t)
{
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
}

Wall::~Wall()
{
	// Dump all child nodes from the game scene
	for( NodeList::iterator it = frame.begin(); it != frame.end(); ++it )
	{
		assert(*it);
		(*it)->remove();
	}// for
}

void Wall::addNode(u32 size, const vector3df& position)
// creates a cube of size and places it at position
{
    // add a wall node to the list
    frame.push_back(smgr->addCubeSceneNode(size, 0, -1, localOrigin+position));
    // set the texture
    frame[frame.size()-1]->setMaterialTexture(0, driver->getTexture(texture));
    // because we do not have a light source, lighting needs to be off or it will be black
    frame[frame.size()-1]->setMaterialFlag(EMF_LIGHTING, false);
    // create selector for collision detection
    ITriangleSelector* selector = smgr->createTriangleSelector(
        frame[frame.size()-1]->getMesh(), frame[frame.size()-1]);
    // attach selector to wall node
    frame[frame.size()-1]->setTriangleSelector(selector);
    // discard the selector reference
    selector->drop();
}

void Wall::makeWall(u32 len, u32 wid, vector3df position)
{
    position += localOrigin;

    for(f32 l = 0; l < len; ++l)
    {
        for(f32 w = 0; w < wid; ++w)
        {  addNode(DEFAULT_CUBE_SIZE, position - vector3df(l*10,0,w*10));  }
    }

    length = len;
    width = wid;
}

}

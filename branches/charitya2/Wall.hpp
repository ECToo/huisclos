#ifndef WALL_H_INCLUDED
#define WALL_H_INCLUDED

#include <vector>
#include <irrlicht.h>

namespace cj {

using namespace std;
using namespace irr;
using namespace video;
using namespace scene;
using namespace gui;
using namespace core;

class Wall
{
    public:
	static const u32 DEFAULT_CUBE_SIZE = 10;

	// TODO: Split into overloaded methods, so that one doesn't need to construct a new default vector.
        Wall(IrrlichtDevice* d, stringw t, vector3df localOrigin = vector3df() );
        ~Wall();
        void addNode(u32 size, const vector3df& offset); //creates a cube of size and places it at position
        void makeWall(u32 length, u32 width, vector3df offset); //create a long wall

	// Identicality test:
	bool operator== (const Wall& rhs) const
	{	return &rhs == this;	}// ==()

	// ACCESSORS.  Note: retval of 0 means that makeWall() has not yet been used.
	u32 getLength() const { return length;	}//
	u32 getWidth() const { return width;	}//
	const vector3df& getPosition() const
	{	return localOrigin;	}// getPosition()
    private:
	typedef std::vector<IMeshSceneNode*> NodeList;// [I called it 'List' instead of 'Vector' just because I've fallen into that convention in other classes.]

	vector3df localOrigin; //initial placement of the 'Wall'; nodes (blocks) added to it are set relative to this position.
	u32 length;
	u32 width;
        NodeList frame; //contains cubes created by addNode
        IrrlichtDevice *device; //get driver and smgr from this
        IVideoDriver* driver; //for drawing
        ISceneManager* smgr; //needed for adding cubes
        const stringw texture; //path of texture file
};// Wall
}// cj

#endif // WALL_H_INCLUDED

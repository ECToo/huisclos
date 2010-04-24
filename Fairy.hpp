#ifndef __FAIRY__
#define __FAIRY__

#include "Agent.hpp"

namespace cj
{
using namespace irr;
using namespace irr::core;
//using namespace irr::scene;
//using namespace irr::video;
//using std::vector;

class Fairy : public Agent
{
public:
	// id=ctor
	//Fairy( const s32 HP, const s32 Str, const s32 Spd, const s32 Acc, IrrlichtDevice* d, const vector3df& position )
	Fairy( IrrlichtDevice& d, vector3df position = irr::core::vector3df(0,0,0) )
		//vector3df rotation = irr::core::vector3df(0,0,0),
		//vector3df scale = irr::core::vector3df(1.0f, 1.0f, 1.0f)
	: Agent( &d, "faerie.md2", "Faerie5.BMP", "", position, 100, 100, 40, 100)// FIXME: Define default vals for stats
	{}// c
	virtual ~Fairy() {}// d
};// Fairy

}// cj
#endif


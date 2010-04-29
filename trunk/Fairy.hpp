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
	static u32 DEFAULT_SPEED;
	//static const s32 DEFAULT_SPEED = 40;
	static u32 DEFAULT_ATTACK_MULTIPLIER; // Other factors aside, '1' would imply one attack per second.

	// id=ctor
	//Fairy( const s32 HP, const s32 Str, const s32 Spd, const s32 Acc, IrrlichtDevice* d, const vector3df& position )
	Fairy( IrrlichtDevice& d, vector3df position = irr::core::vector3df(0,0,0) );
	virtual ~Fairy();
	virtual u32 getAttackMultiplier() const;
};// Fairy

}// cj
#endif


// NB: Not currently part of project.
#include "Fairy.hpp"

namespace cj
{

u32 Fairy::DEFAULT_SPEED = 100;
u32 Fairy::DEFAULT_ATTACK_MULTIPLIER = 2;

// id=CTOR
Fairy::Fairy( IrrlichtDevice& d, vector3df position )
		//vector3df rotation = irr::core::vector3df(0,0,0),
		//vector3df scale = irr::core::vector3df(1.0f, 1.0f, 1.0f)
	: Agent( &d, "faerie.md2", "Faerie5.BMP", "", position, 100, 100, DEFAULT_SPEED, 100)// FIXME: Define default vals for stats
{}// c

Fairy::~Fairy() {}// d

u32 Fairy::getAttackMultiplier() const
{
	return Fairy::DEFAULT_ATTACK_MULTIPLIER;
}// getAttackMultiplier()
}// cj


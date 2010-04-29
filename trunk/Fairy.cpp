// NB: Not currently part of project.
#include "Fairy.hpp"

namespace cj
{

s32 Fairy::DEFAULT_SPEED = 100;

// id=CTOR
Fairy::Fairy( IrrlichtDevice& d, vector3df position )
		//vector3df rotation = irr::core::vector3df(0,0,0),
		//vector3df scale = irr::core::vector3df(1.0f, 1.0f, 1.0f)
	: Agent( &d, "faerie.md2", "Faerie5.BMP", "", position, 100, 100, DEFAULT_SPEED, 100)// FIXME: Define default vals for stats
{}// c

Fairy::~Fairy() {}// d


	//Fairy::Fairy( const s32 HP, const s32 Str, const s32 Spd, const s32 Acc, IrrlichtDevice* d, const vector3df& position ): Agent( d, "faerie.md2", "Faerie5.BMP", "", position)
//{}// ctor


//void Fairy::Die()
//{
	//setHP(0);
	//// FIXME:
	////clearAllActions();
	////setState( Agent::DEAD );
//dpr("NPC " << it->getID() << " died.");
	//// TODO: play death animation
//}// Die()

//void Fairy::TakeDamage( const s32 damage )
//{
	//assert( getHP() > 0 );
	//if( damage == 0 )
	//{ [> NOP <] }// if
	//else if( damage > getHP() )
	//{ 	setHP( 0 ); }// elif
	//else
	//{	setHP( getHP() - damage );	}// else

	//// If I've had it
	//if( HP == 0 )
	//{
		//Die();
		//assert( getState() == Agent::DEAD );
	//}// if
//}// TakeDamage()

//actions::ActAgentAttack* const Fairy::Attack( Agent& target )
//{
	//assert( Agent::getLineOfSightExists( *self, target ) );
	//actions::ActAgentAttack* newact = new actions::ActAgentAttack(*self, target);
	//assert(newact);
	//setCurrentAction(newact);
	//return newact;
//}// Attack()


//namespace actions
//{

//// ctor
//ActAgentAttack::ActAgentAttack( Agent& atk, Agent& targ )
//: attacker(atk), target(targ)
//{
	//agent.setAttackTarget(&target);
	////TODO: Attack animation
//}// ctor

//ActAgentAttack::~ActAgentAttack()
//{
	//agent.setAttackTarget( NULL );
//}// dtor

//bool ActAgentAttack::runTick( const f32 frameDeltaTime )
//{
	//// TODO:
	////calc hit chance based on ACC & dist
	////random val
	////if( val → hit )
	//{
		////TODO: calculate damage based on random val & STR
		//const s32 damage = 10;
		//target.TakeDamage(damage);
	//}

//dpr( getID() << " attacks agent " << target.getID() );
	//return true;// one-shot
//}// runTick()

//}// actions
}// cj


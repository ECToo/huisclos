#include "Agent.hpp"

namespace cj
{
// id=action
namespace actions
{

void MoveAction::start()
{
dpr("Move started.");
	assert( !started() );
	agent.animationRun();
	agent.getBody().setRotation( (destination - agent.getBody().getAbsolutePosition()).getHorizontalAngle() + vector3df(0,-90.00,0) );
	agent.getBody().updateAbsolutePosition();
	actions::IAction::start();// chain up
}// start()

bool MoveAction::runTick( const f32 deltaTime )
{
	assert( started() );

//dpr("ActAgentMove tick to " << destination);
	vector3df remaining = destination - agent.getBody().getAbsolutePosition();
	f32 dist = deltaTime * speed;
	if( dist >= remaining.getLength() )
	{ 	dist = remaining.getLength(); 	}// if

	vector3df distVec = (destination - agent.getBody().getPosition());
	distVec.normalize() *= dist;
//dpr("dist " << dist );

	// TODO: Optional:
	//agent.getDriver().draw3DLine( agent.getPosition().toIrr_vector3df(), dist.to_absVec(*agent.getBody().getParent()).toIrr_vector3df() );

	agent.getBody().setPosition( agent.getBody().getPosition() + distVec );
	agent.getBody().updateAbsolutePosition();

	if( agent.getBody().getAbsolutePosition() == destination )
	{
dpr("Arrived at " << destination);
		//waypoint.setFancyGraphic(false);
		return true;
	}// if
	else
	{	return false;	}// else
}// runTick()
	

// id=ATTACK
//AttackAction::AttackAction( Agent& atk, Agent& targ )
//: attacker(atk), target(targ)
//{}// ctor

//void AttackAction::start()
//{
//dpr( "Agent " << attacker.getID() << " attacks Agent " << target.getID() );
	////TODO: animationAttack();
	//const bool hit = true;//TODO: calc hit chance based on ACC & dist
	//if( hit )
	//{
		//const s32 damage = 10;//TODO: calculate damage based on random val & STR
		//target.TakeDamage(damage);
	//}// if
	//IAction::start();// chain up
//}// start()

//bool AttackAction::runTick( const f32 deltaTime )
//{
	////...
	//return false; // TODO: Actual retval should be true only when the entire attack sequence is complete.
//}// runTick()

}// actions
}// cj


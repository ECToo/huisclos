#include "Agent.hpp"

namespace cj
{
// id=action
namespace actions
{

// ACT-AGENT-ATTACK
ActAgentAttack::ActAgentAttack( Agent& atk, Agent& targ )
: attacker(atk), target(targ)
{
	attacker.setAttackTarget(&target);
	attacker.animationAttack();
}// ctor

ActAgentAttack::~ActAgentAttack()
{
	attacker.setAttackTarget( NULL );
}// dtor

bool ActAgentAttack::runTick( const f32 frameDeltaTime )
{

dpr( "Agent " << attacker.getID() << " attacks Agent " << target.getID() );
	// TODO:
	//calc hit chance based on ACC & dist
	//random val
	//if( val → hit )
	{
		//TODO: calculate damage based on random val & STR
		const s32 damage = 10;
		target.TakeDamage(damage);
	}

	return true;// one-shot
}// runTick()

// ACT-AGENT-SEEK-POS
ActAgentSeekPosition::ActAgentSeekPosition( Agent& agt, const vector3df& dest, f32 spd ): agent(agt), destination(dest), speed(spd)
{
	// FIXME: 
	agent.setHasMoveTarget( true );

	agent.animationRun();
}// ctor

ActAgentSeekPosition::~ActAgentSeekPosition() {}

bool ActAgentSeekPosition::runTick( const f32 frameDeltaTime )
{
//dpr("ActAgentMove tick to " << destination);
	vector3df remaining = destination - agent.getBody().getAbsolutePosition();
	f32 dist = frameDeltaTime * speed;
	if( dist >= remaining.getLength() )
	{ 	dist = remaining.getLength(); 	}// if

	vector3df distVec = (destination - agent.getBody().getPosition());
	distVec.normalize() *= dist;
//dpr("dist " << dist );

	// TODO: Optional:
	//agent.getDriver().draw3DLine( agent.getPosition().toIrr_vector3df(), dist.to_absVec(*agent.getBody().getParent()).toIrr_vector3df() );

	// TODO: : DON'T Set agent heading here.   agent.setRotation( ... );
	agent.getBody().setRotation( (destination -  agent.getBody().getAbsolutePosition()).getHorizontalAngle() + vector3df(0,-90.00,0) );
	agent.getBody().setPosition( agent.getBody().getPosition() + distVec );
	agent.getBody().updateAbsolutePosition();

	if( agent.getBody().getAbsolutePosition() == destination )
	{
//dpr("Arrived at " << destination);
		//waypoint.setFancyGraphic(false);
		agent.setHasMoveTarget( false );
		return true;
	}// if
	else
	{	return false;	}// else
}// runTick()

}// actions
}// cj


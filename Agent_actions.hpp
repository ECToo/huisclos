//#ifndef __AGENT_ACTIONS__
//#define __AGENT_ACTIONS__

#include "Agent.hpp"
#include "Action.hpp"

namespace cj
{
// id=act, id=actions
namespace actions
{
class ActAgentAttack : public ITickAction
{
public:
	ActAgentAttack( Agent& attacker, Agent& targ );
	virtual ~ActAgentAttack();
	virtual bool runTick( const f32 frameDeltaTime );

private:
	Agent& attacker;
	Agent& target;
};// ActAgentAttack

// id=act-agent-turn
class ActAgentTurn : public ITickAction
{
public:
	// CTOR
	ActAgentTurn( Agent& agt, const relAngle& angle, f32 spd ): agent(agt), totalAngle(angle), speed(spd) {}

	// DTOR
	virtual ~ActAgentTurn() {}

	bool runTick( const f32 frameDeltaTime )
	{
		relAngle angle( frameDeltaTime * speed );

		if( angle > totalAngle )
		{	angle = totalAngle;	}// if

		agent.turnAtomic( angle );
		totalAngle -= angle;
		return totalAngle.iszero(); // indicates completion
		//if( angle.iszero() )
		//{
			//agt.getActionsList().erase(*this);
		//}// if
	}// runTick()
private:
	Agent& agent;
	relAngle totalAngle;
	const f32 speed;
};// ActAgentTurn

// id=act-agent-move
class ActAgentMove : public ITickAction
{
public:
	// CTOR
	ActAgentMove( Agent& agt, const relVec& dist, f32 spd ): agent(agt), totalDist(dist), speed(spd), rangefinderPrevState(agt.getRangefinder())
	{
		// We need the rangefinder on, so, if it isn't, make a memo of the state and then enable it temporarily.
		if( !rangefinderPrevState )
		{	agt.setRangefinder(true, false);	}// if
	}// ctor

	// DTOR
	virtual ~ActAgentMove() {}

	bool runTick( const f32 frameDeltaTime )
	{
		relVec dist( frameDeltaTime * speed );

		// If we'd be going to far, scale back
		if( dist.getLengthSQ() > totalDist.getLengthSQ() )
		{	dist = totalDist;	}// if

		// TODO: Optional:
		//agent.getDriver().draw3DLine( agent.getAbsolutePosition().toIrr_vector3df(), dist.to_absVec(agent.getBody()).toIrr_vector3df() );

		// Do we avoid hitting a wall on the way?
		bool success = agent.moveAtomic( dist );
		if( success )
		{	totalDist -= dist;	}// if

		assert( relVec() == relVec(0.0,0.0) );// TODO: delete.

		if ( !success || (totalDist == relVec()) ) // i.e., we ran into a wall and had to stop OR we've completed the movement.
		{
			// If we're done, set the rangefinder back to the way it was.  Note that the *visibility* of the rangefinder isn't touched, here.
			agent.setRangefinder( rangefinderPrevState );
			return true;
		}// if
		else
		{	return false;	}// else
	}// runTick()
private:
	Agent& agent;
	relVec totalDist;
	const f32 speed;
	bool rangefinderPrevState; // Restored to this state upon termination
};// ActAgentMove

//class LoaderAgentSeekWaypoint
//{
//};//

// ************ ACT AGENT SEEK POSITION
// id=position
class ActAgentSeekPosition : public ITickAction
{
public:
	ActAgentSeekPosition( Agent& agt, const vector3df& dest, f32 spd );
	virtual ~ActAgentSeekPosition();
	virtual bool runTick( const f32 frameDeltaTime );
private:
	Agent& agent;
	const vector3df destination;
	const f32 speed;
};// ActAgentSeekPosition


//actions::ActAgentMove* const Agent::seek( const absVec& dest, f32 speed, f32 turnspeed )
//{
	//const absVec dest_conv = absVec::from_vector3df(dest);
	//actions::ActAgentMove* const newact = move( dest_conv.to_relVec(getBody()), speed );
	////actions::ActAgentMove* const newact = move( dest.to_relVec(getBody()), speed );
	//if( turnspeed != 0.0 )
	//{	turn( dest.to_relAngle(getBody()), turnspeed );	}// if
	//return newact;
//}// seek()

}// actions
}// cj


//#endif


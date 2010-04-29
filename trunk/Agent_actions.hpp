//#ifndef __AGENT_ACTIONS__
//#define __AGENT_ACTIONS__

#include "Agent.hpp"
#include "Action.hpp"

namespace cj
{
// id=act, id=actions
namespace actions
{

// Moves an Agent to a given destination at specified speed.  Simple reorientation.
class MoveAction : public actions::IAction//, public Timed
{
public:
	MoveAction( Agent& agt, const vector3df dest, f32 spd );
	virtual ~MoveAction() ;
	virtual void start();
	virtual bool runTick( const f32 deltaTime );
private:
	Agent& agent;
	const vector3df destination;
	const f32 speed;
};// MoveAction

// For traversing a list of points with A*.  TODO: A ctor that takes an iterator to a list of points.
class FollowPathAction : public ActionSeq
{
public:
	//virtual ~FollowPathAction()
	//{ assert( started() );	}
};// FollowPathAction

// id=die
class DieAction : public IAction, public Timed
{
public:
	DieAction( Agent& agt ): Timed(1), agent(agt)
	{}//agent, animationspeed, period): Timed( period )
	virtual ~DieAction() {}
	virtual void start()
	{
//dpr("Agent " << agent.getID() << " began to Die.");
		assert(!started());
		agent.animationDie();
		assert(!started());
		IAction::start();
	}// start()
	virtual bool runTick( f32 frameDeltaTime )
	{
		return true;// one-shot
		//if( Time-passed f32)
			//(if done return true
			 //(progn (set done) nil)))
	}// runTick()
private:
	Agent& agent;
};// DieAction


// id=ATTACK
class AttackAction : public IAction, public Timed
{
public:
	AttackAction( Agent& attacker, Agent& targ );
	virtual ~AttackAction();
	virtual void start();
	virtual bool runTick( const f32 deltaTime );
private:
	Agent& attacker;
	Agent& target;
};// AttackAction

//// id=act-agent-turn
//class ActAgentTurn : public ITickAction
//{
//public:
	//// CTOR
	//ActAgentTurn( Agent& agt, const relAngle& angle, f32 spd ): agent(agt), totalAngle(angle), speed(spd) {}

	//// DTOR
	//virtual ~ActAgentTurn() {}

	//bool runTick( const f32 deltaTime )
	//{
		//relAngle angle( deltaTime * speed );

		//if( angle > totalAngle )
		//{	angle = totalAngle;	}// if

		//agent.turnAtomic( angle );
		//totalAngle -= angle;
		//return totalAngle.iszero(); // indicates completion
		////if( angle.iszero() )
		////{
			////agt.getActionsList().erase(*this);
		////}// if
	//}// runTick()
//private:
	//Agent& agent;
	//relAngle totalAngle;
	//const f32 speed;
//};// ActAgentTurn


}// actions
}// cj


//#endif


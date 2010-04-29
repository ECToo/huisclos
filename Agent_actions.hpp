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

// id=ATTACK
//class AttackAction : public actions::IAction
//{
//public:
	//AttackAction( Agent& attacker, Agent& targ );
	//virtual ~AttackAction();
	//virtual void start();
	//virtual bool runTick( const f32 deltaTime );
//private:
	//Agent& attacker;
	//Agent& target;
//};// AttackAction

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


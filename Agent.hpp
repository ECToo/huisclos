#ifndef __AGENT__
#define __AGENT__

#include "assert_swig.hpp"
#include <irrlicht.h>
#include <vector>
#include <math.h>
#include "boost/utility.hpp" // boost::noncopyable

#include "Wall.hpp"
#include "Sensor.hpp"
#include "Action.hpp"
#include "Coordinates.hpp"

namespace cj
{
using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;
using std::vector;

struct pointOfInterest
{   //Simplified struct for now, class later
    f32 heading;
    f32 range;
    IAnimatedMeshSceneNode* object;
};

// fwd dec
namespace actions
{
	class ActAgentSeekPosition;
	class ActAgentAttack;
	//class ActAgentTurn;
	//class ActAgentMove;
	// TODO: ActAgentVisitWaypoints < ActionSequence
}//

// TODO: Change private to protected once subclass interface has been considered properly.
// id=agent
class Agent : public boost::noncopyable, public sensor::SSensors //, public IStateEntity
{
public:
	static const f32 DEFAULT_GIRTH;
	static const f32 MOVEMENT_SLOW;
	static const f32 MOVEMENT_FAST;
	static const f32 TURN_SLOW;
	static const f32 TURN_FAST;

	typedef vector<pointOfInterest> ContactsList;

	enum AgentState { MANUAL, DEAD, ATTACK, MOVE };

	// TODO: Polymorphism would be superior.  Would mean changing addAgent() yet again.
	enum MOB { FAIRY };// MOB

	static s32 nextAvailableID;
	static s32 genID(); // Returns the next unused Agent ID int.
	s32 getID() const;
	static bool getLineOfSightExists( const Agent& a1, const Agent& a2 );// true if l-o-s exists between the named Agents.

	// id=Ctor
	Agent(IrrlichtDevice* d, stringw mesh, stringw texture, stringw path, const vector3df& position, const s32 HP, const s32 Str, const s32 Spd, const s32 Acc);
	// id=DTOR
	virtual ~Agent();

	// Wrapper for ISceneNode#setPosition(); relative coords.
	void setPosition( const relVec& dest );
	// Wrapper for ISceneNode#setAbsolutePosition().
	void setAbsolutePosition( const absVec& dest );
	void setPosition( const absVec& dest );

	relAngle getRotation() const;
	void setRotation(const relAngle& rot);
	// TODO: with absAngleVector3d:
	//absAngle getAbsoluteRotation() const
	//{ 	return absAngle::from_rotation3df(getBody().getAbsoluteTransformation().getRotation());	}// getAbsoluteRotation()
	//void setAbsoluteRotation(const absAngle& rot) const
	//{ 	setRotation( rot.to_relAngle( getBody().getParent() ) );	}// setAbsoluteRotation()

	// Turn by a specified amount.
	void turnAtomic( const relAngle& theta );
	bool moveAtomic( const relVec& dest );

	// id=ACTIONS:
	// Also look in protected: section.
	actions::ActAgentSeekPosition* const Goto( const vector3df& dest, f32 speed );// Go straight to the destination.
	template <typename TWaypointsList> actions::ActionSequence* visitWaypoints( const TWaypointsList& pointsList, f32 speed );
	actions::ITickAction* const Seek( const vector3df& dest, const cj::Wall& w, f32 speed=0.00, bool debug=true );// Pathfind to the destination with A*.  Leaving speed=0 will cause that Agent's default speed to be used.
	void clearAllActions();
	void doTickActions( f32 frameDeltaTime );// Used by Game.

	vector<f32> DrawFeelers(bool debug = false);  //wall collision detection
	// TODO: Even better: make these func templates taking const_iterators.)
	template <typename TAgentsIterator> ContactsList DrawCircle(TAgentsIterator begin, const TAgentsIterator& end, bool debug = false);  //shows circular range and agents in range
	template <typename TAgentsIterator> void ClearCircle(TAgentsIterator begin, const TAgentsIterator& end);
	template <typename TAgentsIterator> vector<f32> DrawPieSlices(TAgentsIterator begin, const TAgentsIterator& end, bool debug = false);  //activation sectors
	void SetResolution(u32 r);  //change resolution
	void SetRange(u32 r);  //change range
	void SetAwareness(u32 a);  //change awareness

	// ACCESSORS
	const IAnimatedMeshSceneNode& getBody(void) const;
	IAnimatedMeshSceneNode& getBody(void);
	relVec getPosition() const;
	absPos getAbsolutePosition() const;

	bool getRangefinderVisible() const {	return rangefinderVisible;	}//
	bool getRadarVisible() const {	return radarVisible;	}//
	bool getActivationLevelsVisible() const {	return activationLevelsVisible;	}//

	void setRangefinderVisible( bool vis=true ) {	rangefinderVisible = vis;	}//
	void setRadarVisible( bool vis=true ) {	radarVisible = vis;	}//
	void setActivationLevelsVisible( bool vis=true ) {	activationLevelsVisible = vis;	}//

	AgentState getState() const;
	void setState( AgentState s );
	bool isDead() const;// predicate

	using sensor::SSensors::setRangefinder;
	using sensor::SSensors::setRadar;
	using sensor::SSensors::setActivation;
	// Idem, but setting visibility simultaneously:
	virtual void setRangefinder( bool mode, bool vis );
	virtual void setRadar( bool mode, bool vis );
	virtual void setActivation( bool mode, bool vis );

	// Sensor output:
	const vector<f32>& getRangefinderOutput() const { assert( getRangefinder() ); return feelersOutput; }//
	const ContactsList& getRadarOutput() const { assert( getRadar() ); return nearbyAgents; }//
	const vector<f32>& getActivationOutput() const { assert( getActivation() ); return activationLevels; }//

	// id=op
	virtual bool operator== (const Agent& rhs) const;
	virtual bool operator!= (const Agent& rhs) const;

	// Called by Game; not for end-user.  Unfortunately, two of the sensor types require iteration, and the list will be traversed twice.  Coroutines would solve the problem.
	template <typename TAgentsIterator> void updateSensors(const TAgentsIterator& begin, const TAgentsIterator& end);

	bool getHasMoveTarget() const;
	void setHasMoveTarget( const bool b );
	const Agent* getAttackTarget() const;
	Agent* getAttackTarget();
	void setAttackTarget( Agent* const targ );
	template <typename TAgentsList> vector<Agent*> getVisibleAgents( TAgentsList&, bool countIfDead=false ); // Returns list of all Agents visible to the caller.
	//template <typename TAgentsList> vector<Agent*> getVisibleAgents( typename TAgentsList::iterator it, const typename TAgentsList::iterator end ); // Returns list of all Agents visible to the caller.
	//bool isEnemyVisible(); // True if there is a line-of-sight to any other agent.

	// (used by ActAgentMove::runTick() for debug-line drawing.  TODO: Perh. make this a property of the Action itself?)
	IVideoDriver& getDriver() {	return *driver;	}// getDriver()

	s32 getHP() const { return HitPoints;	}//
	void setHP( const s32 hp ) {	HitPoints = hp;	}//
	s32 getStr() const { return Strength;	}//
	void setStr( const s32 str ) {	Strength = str;	}//
	s32 getSpd() const { return Speed;	}//
	void setSpd( const s32 spd ) {	Speed = spd;	}//
	s32 getAcc() const { return Accuracy;	}//
	void setAcc( const s32 acc ) {	Accuracy = acc;	}//

	void Die();
	void TakeDamage( const s32 damage );
	actions::ActAgentAttack* const Attack( Agent& target );

	//actions::ActAtkMelee* AttackMelee( Agent& target );
	//ActAtkRanged* AttackRanged( Agent& target );


	// Animation adjustment:
	virtual void animationStand();
	virtual void animationAttack();
	virtual void animationRun();
	virtual void animationDie();

	//virtual void onStateBegin();
	//virtual void onStateEnd();
	//virtual bool onTick( u32 frameDeltaTime_ms );

	// <TAG> CA - NOTE: Do not add public functions to Agent beyond this line.
	// This will be my section.
	bool MoveVector(vector3df distance);  //COLLISON MOVEMENT
protected:
	const actions::ITickAction* getCurrentAction() const;
	actions::ITickAction* getCurrentAction();
	void clearCurrentAction();
	void setCurrentAction( actions::ITickAction* const newact );

// id=private
private:
	IrrlichtDevice *device; //get driver and smgr from this
	IVideoDriver* driver; //for drawing
	ISceneManager* smgr; //needed for adding cubes
	ISceneCollisionManager* cmgr; //for collision detection
	IAnimatedMeshSceneNode* body;  //scene node
	IBillboardSceneNode * circle;  //node for circle
	u32 awareness;  //view range
	u32 resolution;  //density of sensors
	u32 range;  //range of sensors
	stringw path;  //directory of textures
	stringw texture;  //path+file of the body's texture

	// These control whether the debug visuals are used:
	bool rangefinderVisible;
	bool radarVisible;
	bool activationLevelsVisible;

	// These eclectically-named lists hold the output from the sensors, as recorded during the most recent tick.  Use the get*Output() accessors, above, instead.
	vector<f32> feelersOutput;
	ContactsList nearbyAgents;
	vector<f32> activationLevels;

	actions::ITickAction* currentAction;
	actions::ActionsList actionsList;
	AgentState currentState;
	Agent* attackTarget;
	bool hasMoveTarget;

	s32 HitPoints;
	s32 Strength;
	s32 Speed;
	s32 Accuracy;

	// Controlling the rate at which AI states are updated:
	//u32 period_ms;
	//u32 tickDeltaTime_ms;
	f32 period;
	f32 tickDeltaTime;

	// Ctor body utility function:
	void AgentCtorImpl(stringw mesh, const vector3df& p);

	// Setters for sensor data.
	void setRangefinderOutput(const vector<f32>& vec ) { feelersOutput = vec;	}//
	void setRadarOutput( const ContactsList& vec ) { nearbyAgents = vec;	}//
	void setActivationOutput( const vector<f32>& vec ) { activationLevels = vec;	}//
};// Agent
}// cj

#include "Agent_templates.hpp"

#endif// inc


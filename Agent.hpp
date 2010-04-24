#ifndef __AGENT__
#define __AGENT__

#include "assert_swig.hpp"
#include <irrlicht.h>
#include <vector>
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
class Agent : public boost::noncopyable, public sensor::SSensors
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

	// Ctor body utility function:
	void AgentCtorImpl(stringw mesh, const vector3df& p);

	// Setters for sensor data.
	void setRangefinderOutput(const vector<f32>& vec ) { feelersOutput = vec;	}//
	void setRadarOutput( const ContactsList& vec ) { nearbyAgents = vec;	}//
	void setActivationOutput( const vector<f32>& vec ) { activationLevels = vec;	}//
};// Agent

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
}// actions

//id=draw-circle
template <typename TAgentsIterator>
Agent::ContactsList Agent::DrawCircle(TAgentsIterator it, const TAgentsIterator& end, bool debug)
{   //draw sensing circle and find detected agents in the list
    vector<pointOfInterest> detected;
    //circle for visual debugging
    if(debug)
    {  circle->setVisible(true);  }
    //draw with a clear material so that lines show up
    SMaterial basic = SMaterial();
    basic.setFlag(EMF_LIGHTING, false);
    driver->setMaterial(basic);
    //draw lines in world coordinate system
    driver->setTransform(video::ETS_WORLD, core::matrix4());
    matrix4 transform = body->getAbsoluteTransformation();

    for( ; it != end; ++it)
    {
        pointOfInterest p;
        p.object = &(it->getBody());
        //do not count yourself
        if(p.object == body)
        {  continue; }

        //figure distance to other agent
        matrix4 other = p.object->getAbsoluteTransformation();
        p.range = transform.getTranslation().getDistanceFrom(other.getTranslation());

        if(p.range <= range)
        {   //if other agent is in range
            if(debug)
            {   //visual debug aids
                //draw a line to the other agent
                line3d<f32> line(transform.getTranslation(), other.getTranslation());
                driver->draw3DLine(line.start, line.end, SColor(0,50,50,136));
                //since we have no light source, this makes the agent black
                p.object->setMaterialFlag(EMF_LIGHTING, true);
            }

            //figure the heading relative to the agent
            vector3df relative = other.getTranslation() - transform.getTranslation();
            //normalize the vector for easy computation
            relative.normalize();
            //The active agent faces along the X axis in her local coordinate
            //system, so create an already normalized vector.
            vector3df aheading(1,0,0);
            transform.rotateVect(aheading);
            //Use the cross product to get the direction of the angle.
            vector3df cp = aheading.crossProduct(relative);
            //cross product angle behaves strange, so use dotproduct for the angle
            //FIX ME: use cross product to avoid extra calculation
            p.heading = aheading.dotProduct(relative);
            p.heading = acos(p.heading) * (180/PI);
            //correct rounding messiness
            if((p.heading * p.heading) <= 0.0000005)
            {  p.heading = 0;  }
            //This only works for our particular setup where the agents are in the XZ
            //plane. If that changes, this will need to change to determine the direction
            //of rotation from the agent's heading to the detected agent's location.
            if(cp.Y > 0.0005) //did not use 0 to take rounding errors into account
            {  p.heading = 360 - p.heading;  }
            //add the other agent to the list of interesting points
	    detected.push_back(p);
        }
        else
        {   //reset the lighting and material if agent is not in range
            p.object->setMaterialFlag(EMF_LIGHTING, false);
            p.object->setMaterialTexture(0, driver->getTexture(texture));
        }
    }

    return detected;
}

template <typename TAgentsIterator>
void Agent::ClearCircle(TAgentsIterator begin, const TAgentsIterator& end)
{   //reset the sensing circle
    circle->setVisible(false);
    for( TAgentsIterator it = begin; it != end; ++it)
    {   //reset the lighting and the texture from DrawCircle and DrawPieSlices debug mode
        it->getBody().setMaterialFlag(EMF_LIGHTING, false);
        it->getBody().setMaterialTexture(0, driver->getTexture(texture));
    }
}

template <typename TAgentsIterator>
vector<f32> Agent::DrawPieSlices(TAgentsIterator begin, const TAgentsIterator& end, bool debug)
{   //determines activation levels of different sectors
    vector<f32> activation(4, 0.0f);
    //find distances and heading to agents
    vector<pointOfInterest> detected = DrawCircle(begin, end);
    //draw with a clear material so that lines show up
    SMaterial basic = SMaterial();
    basic.setFlag(EMF_LIGHTING, false);
    basic.Thickness = 3;
    driver->setMaterial(basic);
    //draw lines in world coordinate system
    driver->setTransform(video::ETS_WORLD, core::matrix4());
    matrix4 transform = body->getAbsoluteTransformation();

    if(debug)
    {  //visual debugging aids
        circle->setVisible(true);
        line3d<f32> line;
        //start at the center of the agent
        line.start = transform.getTranslation();
        //end at the edge of the range
        line.end = vector3df(range,0,0);
        //transform it according to the agent's heading
        transform.transformVect(line.end);
        //rotate endpoint to 45 degrees and draw a line
        line.end.rotateXZBy(45, line.start);
        driver->draw3DLine(line.start, line.end, SColor(0,0,0,0));

        for(u32 i = 0; i < 3; i++)
        {   //draw the other 3 lines 90 degrees apart
            line.end.rotateXZBy(90, line.start);
            driver->draw3DLine(line.start, line.end, SColor(0,0,0,0));
        }
    }

    for(u32 i = 0; i < detected.size(); i++)
    {   //for all detected agents, add range to that agent's sector
        if(detected.at(i).heading <= 45  || detected.at(i).heading > 315)
        {   //extra modifier for something that is in the forward sector
            activation.at(0) += ((range - detected.at(i).range) * 2);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "purple.jpg"));
        }
        else if(detected.at(i).heading <= 135  && detected.at(i).heading > 45)
        {   //sector to the left
            activation.at(1) += (range - detected.at(i).range);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "green.jpg"));
        }
        else if(detected.at(i).heading <= 225  && detected.at(i).heading > 135)
        {   //sector to the rear is less noticable
            activation.at(2) += ((range - detected.at(i).range) / 2);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "yellow.jpg"));
        }
        else if(detected.at(i).heading <= 315  && detected.at(i).heading > 225)
        {   //sector to the right
            activation.at(3) += (range - detected.at(i).range);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "blue.jpg"));
        }
    }

    return activation;
}


template <typename TAgentsIterator>
void Agent::updateSensors(const TAgentsIterator& begin, const TAgentsIterator& end)
{
	if( getRangefinder() )
	{ setRangefinderOutput( DrawFeelers( getRangefinderVisible() ) ); }// if
	if( getRadar() )
	{ setRadarOutput( DrawCircle( begin, end, getRadarVisible() ) ); }// if
	if( getActivation() )
	{ setActivationOutput( DrawPieSlices( begin, end, getActivationLevelsVisible() ) ); }// if
}// updateSensors()

template <typename TWaypointsList>
actions::ActionSequence* Agent::visitWaypoints( const TWaypointsList& pointsList, f32 speed )
{
	using namespace cj::actions;

	ActionSequence* act = new ActionSequence;

	for( typename TWaypointsList::const_iterator it = pointsList.begin(); it != pointsList.end(); ++it )
	{
//dpr("Pushed " << *it);
		act->push_back( new ActAgentSeekPosition(*this, *it, speed) );
	}// for

	assert( act->size() == pointsList.size() );

	setCurrentAction(act);

	return act;
}// visitWaypoints()

template <typename TAgentsList>
vector<Agent*> Agent::getVisibleAgents( TAgentsList& allAgents, bool countIfDead )
//vector<Agent*> Agent::getVisibleAgents( typename TAgentsList::iterator it, const typename TAgentsList::iterator end )
{
	vector<Agent*> visAgents;

	for( typename TAgentsList::iterator it = allAgents.begin(); it != allAgents.end(); ++it )
	{
		if( 	*this != *it &&  
			( countIfDead || !it->isDead() ) && 
			getLineOfSightExists( *this, *it )
		  ){	visAgents.push_back(&*it);	}// if
	}// for

	return visAgents;
}// getVisibleAgents()

//actions::ActAgentMove* const Agent::seek( const absVec& dest, f32 speed, f32 turnspeed )
//{
	//const absVec dest_conv = absVec::from_vector3df(dest);
	//actions::ActAgentMove* const newact = move( dest_conv.to_relVec(getBody()), speed );
	////actions::ActAgentMove* const newact = move( dest.to_relVec(getBody()), speed );
	//if( turnspeed != 0.0 )
	//{	turn( dest.to_relAngle(getBody()), turnspeed );	}// if
	//return newact;
//}// seek()


}// cj

#endif// inc


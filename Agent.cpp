#include "Agent.hpp"
#include <cstdlib> // rand()
#include <ctime> // time()

namespace cj {

IState::IState(): hasStarted(false) {}

IState::~IState()
{	assert( started() );	}

void IState::start()
{
	assert( !started()	);
	hasStarted = true;
}// start()

void IState::stop()
{
	assert( hasStarted = true );
	hasStarted = false;
}// start()

bool IState::started() const
{	return hasStarted;	}//

// id=manual
void ManualState::start()
{
	assert( !started()	);
	IState::start();
}
void ManualState::stop()
{	IState::stop();	}

void ManualState::runTick( f32 frameDeltaTime )
{
	assert( started() );
	if( agent.getCurrentAction() )
	{
		const bool bDone = agent.getCurrentAction()->runTick(frameDeltaTime);
		if( bDone )
		{
dpr("Manual-state action done.");
			assert( agent.getCurrentAction() );
			agent.clearCurrentAction();
			agent.animationStand();
		}// if
	}// if
}// runTick()

void WanderState::start()
{
	assert( !agent.getCurrentAction() );
	resetWander();
	IState::start();
}// start

void WanderState::stop()
{
	assert( agent.getCurrentAction() );
	//delete wander;
	//wander = NULL;
	agent.clearCurrentAction();
	IState::stop();
}// stop()

void WanderState::resetWander()
{
	//if( wander )
	//{
		//delete wander;
	//}// if
	agent.clearCurrentAction();

	// Find a *valid* node to wander to:
	bool fail;
	do
	{
		try
		{
			fail = false;
			wander = agent.Seek( Wall::Instance().getRandomNodePosition(), static_cast<f32>(agent.getSpd()) ) ;
		}// try
		catch(PathfindException e)
		{
dpr( "Agent " << agent.getID() << " failed pathfind.  Retrying." );
			fail = true;
		}
	} while(fail);

dpr( "Agent " << agent.getID() << " began Wander circuit." );
//dpr( "Agent " << agent.getID() << " randomly chose node " << random << " at " << chosen );
}// resetWander()

void WanderState::runTick( f32 deltaTime )
{
	assert( started() );
	assert( agent.getCurrentAction() );
	if( !agent.isEnemyVisible() /*true*/ )
	{
		assert( wander == agent.getCurrentAction() );
		if( agent.getCurrentAction()->runTick(deltaTime) )
		//if( wander->runTick(deltaTime) )
		{
dpr( "Agent " << agent.getID() << " finished a Wander-state circuit." );
			resetWander();
		}// if
	}// if
	else // Attack mode
	{
		stop();
		agent.setState(agent.Fight);
	}// else
}// runTick()


//  id=death, id=dead
DeathState::DeathState(Agent& ag): agent(ag), playdead(NULL)
{}

void DeathState::start()
{
	assert( !started()	);
	assert( !agent.getCurrentAction() );
	playdead = new actions::DieAction(agent);
	agent.setCurrentAction(playdead);
	playdead->start();
	IState::start();
}// start()

void DeathState::runTick( f32 deltaTime )
{
	assert( playdead );
	assert( started() );
	assert( playdead == agent.getCurrentAction() );
	playdead->runTick(deltaTime);
}// runTick()

// id=fight, id=attack, id=atk
void FightState::start()
{
	assert( !started()	);
	assert( !agent.getCurrentAction() );
	assert( !attack );
	resetAttack();
	IState::start();
}// start()

void FightState::resetAttack()
{
	agent.clearCurrentAction();

	vector<Agent*> agentsSeen = agent.getVisibleAgents();
	const u32 random = rand() % agentsSeen.size();
dpr( "Agent " << agent.getID() << " attacking visible Agent #" << random );

	//it->getNearbyRandomEnemy()
	attack = new actions::AttackAction( agent, **(agentsSeen.begin() + random) );
	agent.setCurrentAction(attack);
	attack->start();
}// resetAttack()

void FightState::stop()
{
	assert( attack );
	assert( attack == agent.getCurrentAction() );
	agent.clearCurrentAction();
	attack = NULL;
	IState::stop();
}// stop()

void FightState::runTick( f32 frameDeltaTime )
{
//dpr( "FightState runtick" );
	if( agent.isEnemyVisible() )
	{
//dpr( "Enemy seen." );
		assert( attack == agent.getCurrentAction() );
		//if( !attack )
		//{	start();	}// if

		if( !attack || attack->runTick(frameDeltaTime) )
		{	resetAttack(); }// if
	}// if
	else // transition to MOVE.
	{
		stop();
		agent.setState( agent.Wander );
	}// else
}// runTick()

//id=agent
//
const f32 Agent::MOVEMENT_SLOW = 12.0f;
const f32 Agent::MOVEMENT_FAST = 60.0f;
const f32 Agent::TURN_SLOW = 70.0f;
const f32 Agent::TURN_FAST = 180.0f;

const f32 Agent::DEFAULT_GIRTH = 15.0; // FIXME: This was arbitrarily chosen.

Agent::agentsListCallback Agent::getAgentsList = NULL;

s32 Agent::genID()
{
	assert( nextAvailableID >= 0 );
	return nextAvailableID ++ ;
}// genID()

s32 Agent::getID() const
{
	assert( getBody().getID() < nextAvailableID );
	return getBody().getID();
}// getID()

// Counter for generating the next free ID#:
s32 Agent::nextAvailableID = 0;

bool Agent::getLineOfSightExists( const Agent& a1, const Agent& a2 )
{
	if( a1 == a2 ) { return true;	}// quick exit

	line3df line;
	line.start = a1.getBody().getPosition();
	line.end = a2.getBody().getPosition();
	triangle3df outtri;
	vector3df point;

	assert( a1.cmgr == a2.cmgr );

	return( !a1.cmgr->getSceneNodeAndCollisionPointFromRay( line, point, outtri ) );
}//


// id=ctor
Agent::Agent(IrrlichtDevice* d, stringw mesh, stringw t, stringw h, const vector3df& p, const s32 HP, const s32 Str, const s32 Spd, const s32 Acc):
	Timed(60),
	device(d),
	path(h),
	texture(t),
	rangefinderVisible(false),
	radarVisible(false),
	activationLevelsVisible(false),
 	feelersOutput(),
	nearbyAgents(),
	activationLevels(),
	currentAction(NULL),
	actionsList(),
	attackTarget(NULL),
	hasMoveTarget(false),
	Manual( new ManualState(*this) ),// TODO: plug leak
	Wander( new WanderState(*this) ),// TODO: plug leak
	Dead( new DeathState(*this) ),// TODO: plug leak
	Fight( new FightState(*this) ),// TODO: plug leak
	currentState( Manual ),
	HitPoints(HP),
	Strength(Str),
	Speed(Spd),
	Accuracy(Acc)
{ AgentCtorImpl(mesh, p); }// ctor

void Agent::AgentCtorImpl(stringw mesh, const vector3df& p)
{
    //initialize naming short cuts
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    cmgr = smgr->getSceneCollisionManager();
    //add the scene node to the scene manager
    body = smgr->addAnimatedMeshSceneNode(smgr->getMesh(mesh), 0, genID() );// NB: genID() arg added to keep agents' IDs from being the default '-1'.
    //turn lighting off since we do not have a light source
    body->setMaterialFlag(EMF_LIGHTING, false);
    //default stand animation
    animationStand();
    //texture for the agent
    body->setMaterialTexture(0, driver->getTexture(texture));
    //place the agent in the world
    body->setPosition( p );
    //default sensor settings
    resolution = 7;
    awareness = 180;
    range = 50;
    // id=circle
    //create the sensing circle and adjust the lighting and texture
    circle = smgr->addBillboardSceneNode(body, dimension2df(range*2,range*2));
    circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
    circle->setMaterialFlag(EMF_LIGHTING, false);
    circle->setMaterialTexture(0, driver->getTexture(path + "circle.png"));
    //circle is not drawn unless needed, so make it invisible
    circle->setVisible(false);

    assert( getState() );
    getState()->start();
}

// id=dtor
Agent::~Agent()
{
	// TODO: This call was segfaulting when the program closes until I stored the agents() vector in a pointer to control the deletion order.  Still looking for a better way.  Base-class fiddling is a nuisance.
	body->remove();
	body = NULL;
}// dtor


void Agent::setPosition( const relVec& dest )
{
	body->setPosition( dest.toIrr_vector3df() );
	body->updateAbsolutePosition();

	assert( getPosition() == dest );
//dpr(dest);
//dpr(getPosition());
//dpr(getBody().getAbsolutePosition() );
	//TODO: assert( getBody().getAbsolutePosition() == dest.to_absVec( *(getBody().getParent()) ).toIrr_vector3df() );
}// setPosition()

void Agent::setAbsolutePosition( const absVec& dest )
{	setPosition(dest); }// setAbsolutePosition()

void Agent::setPosition( const absVec& dest )
{
	setPosition( dest.to_relVec( *(getBody().getParent()) ) );
	//body->updateAbsolutePosition();

	assert( getAbsolutePosition() == dest );
	assert( getBody().getPosition() == dest.to_relVec( *(getBody().getParent()) ).toIrr_vector3df() );
}// setPosition()

relAngle Agent::getRotation() const { 	return relAngleVec3d::from_vector3df(getBody().getRotation()).to_relAngle();	}// getRotation()
void Agent::setRotation(const relAngle& rot) { 	getBody().setRotation( rot.to_relAngleVec3d().to_vector3df() ); 	}// setRotation()

// TODO: inline
//void Agent::turn( const relAngle& ang, f32 speed )
//{ 	actionsList.queueAction( new actions::ActAgentTurn(*this, ang, speed) ); }// turn()

// TODO: inline
// id=move
//actions::ActAgentMove* const Agent::move( const relVec& dest, f32 speed )
//{
//	actions::ActAgentMove* const newact = new actions::ActAgentMove(*this, dest, speed);
//	actionsList.queueAction( newact );
//	return newact;
//}// move()

vector<f32> Agent::DrawFeelers(bool debug)
{   //defaults for just one feeler
    vector<f32> endpoints(resolution);
    s32 begin = 0;
    s32 end = 0;
    s32 step = 1;
    //since the driver draws everything in the world coordinate system
    //we need to translate and rotate everything by the agent's transform
    matrix4 transform = body->getAbsoluteTransformation();

    if(resolution != 1)
    {   //start half of the awareness angle to the left
        begin = (s32)awareness / (-2);
        //and end half of the awareness to the right
        end = awareness / 2;
        //degree of seperation of feelers
        step = awareness / (resolution - 1);
    }

    // ⁅I need both an accumulator and a stepwise counter here; I added the iterator.⁆
    vector<f32>::iterator it = endpoints.end(); // TODO: Better as a reverse iterator?
    for(  int i = begin; i <= end; i += step)
    {
        line3d<f32> line;
        //start at the center of the agent
        line.start = transform.getTranslation();
        //end at the edge of the range
        line.end = vector3df(range,0,0);
        //transform it according to the agent's heading
        transform.transformVect(line.end);
        //rotate endpoint to the next angle
        line.end.rotateXZBy(i, line.start);
        vector3df point;
        triangle3df outtri;
        //make sure we are using a clear material to start
        SMaterial basic = SMaterial();
        basic.setFlag(EMF_LIGHTING, false);
        driver->setMaterial(basic);
        //draw relative to absolute coordinates
        driver->setTransform(video::ETS_WORLD, matrix4());

        if(cmgr->getSceneNodeAndCollisionPointFromRay(line, point, outtri))
		{   //collision point modifies the endpoint and outlines the hit triangle
		    line.end = point;
		    if(debug)
		    {  driver->draw3DTriangle(outtri, video::SColor(255,0,0,130));  }
        }

        //draw the feeler and add the distance to collision to the result list
        if(debug)
        {  driver->draw3DLine(line.start, line.end, SColor(102,120,255,136));  }

	*(--it) = line.start.getDistanceFrom(line.end);
    }

    return endpoints;
}


void Agent::SetResolution(u32 r)
{   //resolution must be odd and at least 1
    resolution = r;
    if(resolution <= 2)
    {  resolution = 1;  }
    else if(resolution % 2 == 0)
    {  --resolution;  }
}

void Agent::SetRange(u32 r)
{  range = r;  }

void Agent::SetAwareness(u32 a)
{  awareness = a;  }


// OPERATORS
// ==|=!(): Standard address check.
inline bool Agent::operator==(const Agent& rhs) const
{	return( this == &rhs ); }// ==()
inline bool Agent::operator!=(const Agent& rhs) const
{	return !( this == &rhs ); }// !=()

vector<Agent*> Agent::getVisibleAgents( bool countIfDead ) const
{
	vector<Agent*> allAgents = Agent::getAgentsList();
	vector<Agent*> visAgents;


	for( vector<Agent*>::iterator it = allAgents.begin(); it != allAgents.end(); ++it )
	{
		if( this != *it &&
			( countIfDead || !(*it)->isDead() ) &&
			getLineOfSightExists( *this, **it )
		  ){	visAgents.push_back(*it);	}// if
	}// for

	return visAgents;
}// getVisibleAgents()

bool Agent::isEnemyVisible( bool countIfDead ) const
{
	return !getVisibleAgents(countIfDead).empty();
}// isEnemyVisible()

void Agent::setRangefinder( bool mode, bool vis )
{
	setRangefinderVisible(vis);
	sensor::SSensors::setRangefinder(mode); // chain up
}// setRangefinder()
void Agent::setRadar( bool mode, bool vis )
{
	setRadarVisible(vis);
	sensor::SSensors::setRadar(mode); // chain up
}// setRadar()
void Agent::setActivation( bool mode, bool vis )
{
	setActivationLevelsVisible(vis);
	sensor::SSensors::setActivation(mode); // chain up
}// setActivation()

const IAnimatedMeshSceneNode& Agent::getBody(void) const
{  return *body;  }
IAnimatedMeshSceneNode& Agent::getBody(void)
{  return *body;  }
relVec Agent::getPosition() const
{	return relVec::from_position( getBody() );	}// getPosition()
absPos Agent::getAbsolutePosition() const
{	return absPos::from_vector3df( getBody().getAbsolutePosition() );	}// getAbsolutePosition()

Agent::AgentState Agent::getState() const
{	return currentState;	}// getState()

void Agent::setState( Agent::AgentState s )
{
	if( getState() != s )
	{
		assert( !isDead() );
		currentState = s;
		currentState->start();
	}// if
}// setState()

bool Agent::isDead() const
{	return getState() == this->Dead;	}// isDead()

void Agent::turnAtomic( const relAngle& theta ) {	setRotation( getRotation() + theta );	}// turnAtomic()

// id=moveAtomic
bool Agent::moveAtomic( const relVec& dest )
{
    // remember old values
    u32 old_awareness = awareness;
    u32 old_range = range;
    u32 old_resolution = resolution;
    // to speed up collision detection and make sure we detect from all angles
    awareness = 360;
    range = 5;  // NOTE: adjust to your liking, this allows hair to go through the walls
    resolution = 7;
    // record success or failure of movement
    bool moved = false;
    relVec old_position = getPosition();
    //vector3df old_position = body->getPosition();

    //vector<f32> cpoints = DrawFeelers(false);
    f32 oldc = 0;
    f32 newc = 0;

    // find collision amounts for current position
    for(vector<f32>::const_iterator it = getRangefinderOutput().begin(); it != getRangefinderOutput().end(); ++it )
    {  oldc += *it;  }
    //for(u32 i = 0; i < cpoints.size(); i++)
    //{  oldc += cpoints.at(i);  }

    // find collision amounts for new position
    setPosition( old_position + dest );

    // NOTE: MUST call this anytime you use setPosition and want to use the new
    // position of the IAnimatedSceneNode in the same tick
    body->updateAbsolutePosition();

    const vector<f32> cpoints = DrawFeelers(false);

    for(u32 i = 0; i < cpoints.size(); i++)
    {  newc += cpoints.at(i);  }

    // the 0.0001 is for rounding errors and to slightly favor movement
    if(oldc - 0.0001 > newc)
    {
        moved = false;
        setPosition(old_position);
    }
    else
    { moved = true; }

    // reset to orignal values
    awareness = old_awareness;
    range = old_range;
    resolution = old_resolution;

    return moved;
}

const actions::IAction* Agent::getCurrentAction() const
{	return currentAction;	}
actions::IAction* Agent::getCurrentAction()
{	return currentAction;	}

void Agent::clearCurrentAction()
{
	if( currentAction != NULL )
	{
dpr( "Clearing action." );
		delete currentAction;
		currentAction = NULL;
	}// if
}// clearCurrentAction()

void Agent::setCurrentAction( actions::IAction* const newact )
{
	clearCurrentAction();
dpr("Setting action.");

	currentAction = newact;
}// setCurrentAction()

actions::MoveAction* const Agent::MoveTo( const vector3df dest, f32 speed )
{
	actions::MoveAction* newact = new actions::MoveAction(*this, dest, speed );
	assert(newact);
	setCurrentAction(newact);

	assert( !newact->started() );
	newact->start();
	return newact;
}// MoveTo()

//actions::ActAgentSeekPosition* const Agent::Goto( const vector3df& dest, f32 speed )
//{
	//actions::ActAgentSeekPosition* newact = new actions::ActAgentSeekPosition(*this, dest, speed );
	//assert(newact);
	//setCurrentAction(newact);
	//// TODO: Move into Action:
	//setHasMoveTarget( true );
	//animationRun();
	//return newact;
//}// Goto()

// id=seek
actions::FollowPathAction* const Agent::Seek( const vector3df dest, /*const Wall& w,*/ f32 speed, bool debug )
{
	//return visitWaypoints( const_cast<Wall&>(w).AStar(getBody().getPosition(), dest, 2, debug),
	return visitWaypoints( Wall::Instance().AStar(getBody().getPosition(), dest, 2, debug),
		(speed == 0 ? getSpd() : speed) );
}// Seek()
	////return visitWaypoints( const_cast<Wall&>(w).AStar(getBody().getPosition(), dest, 2, debug),
	//std::list<vector3df> points = Wall::Instance().AStar(getBody().getPosition(), dest, 2, debug);
	//while( points.empty() )
	//{
//dpr( "Picking another point." );
	//,
	//return visitWaypoints(
		//(speed == 0 ? getSpd() : speed) );
//actions::IAction* const Agent::Seek( const vector3df& dest, const Wall& w, f32 speed, bool debug )
//{
	//return visitWaypoints( const_cast<Wall&>(w).AStar(getBody().getPosition(), dest, 2, debug),
		//(speed == 0 ? getSpd() : speed) );
//}// Seek()

void Agent::clearAllActions()
{
	// TODO: Fairly unimportant for now, with our using just one action
	clearCurrentAction();
}// clearAllActions()

//inline void Agent::turnAtomic( const relAngle& theta )
//{	setRotation( getRotation() + theta );	}// turnAtomic()

// id=runtick, id=run-tick
void Agent::runTick( f32 frameDeltaTime )
{
	assert( getState() );
	//if( getCurrentAction() )
	//{
		if( periodMet(frameDeltaTime) )
		{
//dpr("runtick");
			getState()->runTick(frameDeltaTime);
		}// if
	//}// if
	//using namespace actions;

	////for( ActionsList::iterator it = actionsList.begin(); it != actionsList.end(); ++it )
	////{	it->runTick( frameDeltaTime );	}// for
	//if( getCurrentAction() )
	//{
		//// TODO: If possible, use ints and ms--faster than floats
		////tickDeltaTime_ms += frameDeltaTime_ms;
		////if( tickDeltaTime_ms >= period_ms )
		////{
			////tickDeltaTime_ms = 0;
		//tickDeltaTime += frameDeltaTime;
		//if( tickDeltaTime >= period )
		//{
////dpr( "Running tick " << tickDeltaTime );
			////const bool bDone = getCurrentAction()->runTick(frameDeltaTime);
			//const bool bDone = getCurrentAction()->runTick(tickDeltaTime);
			//if( bDone )
			//{
				//assert( getCurrentAction() );
				//clearCurrentAction();
			//}// if
			//else // restart wait for next tick
			//{ 	tickDeltaTime = 0.00; }// else
		//}// if
	//}// if
}// runTick

//void Agent::setHasMoveTarget( const bool b )
//{	 hasMoveTarget = b;	}
//bool Agent::getHasMoveTarget() const
//{	return hasMoveTarget;	}//

//const Agent* Agent::getAttackTarget() const
//{	return attackTarget;	}//

//Agent* Agent::getAttackTarget()
//{	return attackTarget;	}//

//void Agent::setAttackTarget( Agent* const targ )
//{	attackTarget = targ;	}

//bool Agent::isEnemyVisible()
//{


void Agent::Die()
{
dpr("Agent " << getID() << " died.");
	setHP(0);
	clearAllActions();
	setState( this->Dead );
}// Die()

void Agent::TakeDamage( const s32 damage )
{
dpr( "HP: " << getHP() );
	assert( getHP() > 0 );
	if( damage == 0 )
	{ /* NOP */ }// if
	else if( damage > getHP() )
	{ 	setHP( 0 ); }// elif
	else
	{	setHP( getHP() - damage );	}// else

	// If I've had it
	if( getHP() == 0 )
	{
		Die();
//		assert( getState() == Agent::DEAD );
	}// if
}// TakeDamage()



void Agent::animationStand()
{     getBody().setMD2Animation(scene::EMAT_STAND);	}//  Agent::animationStand()

void Agent::animationAttack()
{     getBody().setMD2Animation(scene::EMAT_ATTACK);	}//  Agent::animationAttack()

void Agent::animationDie()
{
	getBody().setLoopMode(false);
	getBody().setMD2Animation(scene::EMAT_DEATH_FALLFORWARD);
}//

void Agent::animationRun()
{     getBody().setMD2Animation(scene::EMAT_RUN);	}//

// id=charity
// <TAG> CA - NOTE: Do not add functions to Agent beyond this line.
// This will be my section.

bool Agent::MoveVector(vector3df distance)
{
    // remember old values
    u32 old_awareness = awareness;
    u32 old_range = range;
    u32 old_resolution = resolution;
    // to speed up collision detection and make sure we detect from all angles
    awareness = 360;
    range = 5;
    resolution = 7;
    // record success or failure of movement
    bool moved = false;
    vector3df old_position = body->getPosition();
    vector<f32> cpoints = DrawFeelers(false);
    f32 oldc = 0;
    f32 newc = 0;

    // find collision amounts for current position
    for(u32 i = 0; i < cpoints.size(); i++)
    {  oldc += cpoints.at(i);  }

    // find collision amounts for new position
    body->setPosition(old_position + distance);
    // NOTE: MUST call this anytime you use setPosition and want to use the new
    // position of the IAnimatedSceneNode in the same tick
    body->updateAbsolutePosition();
    cpoints = DrawFeelers(false);
    for(u32 i = 0; i < cpoints.size(); i++)
    {  newc += cpoints.at(i);  }

    // the 0.0001 is for rounding errors and to slightly favor movement
    if(oldc - 0.0001 > newc)
    {
        moved = false;
        body->setPosition(old_position);
    }
   else
   {
      line3d<f32> line;
      //start at old position
      line.start = old_position;
      //end at the new position
      line.end = old_position + distance;
      vector3df point;
      triangle3df outtri;
      //draw relative to absolute coordinates
      driver->setTransform(video::ETS_WORLD, matrix4());

      if(cmgr->getSceneNodeAndCollisionPointFromRay(line, point, outtri))
		{  //new position is not in line of sight of old position, so we
         //move only to the collision point - range
         vector3df d = distance;
         d.normalize();
         d *= range;
         body->setPosition(point - d);
      }

      moved = true;
   }

    // reset to orignal values
    awareness = old_awareness;
    range = old_range;
    resolution = old_resolution;

    return moved;
}


}// cj


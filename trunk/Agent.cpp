#include "Agent.hpp"
// TEST
namespace cj {


const f32 Agent::MOVEMENT_SLOW = 12.0f;
const f32 Agent::MOVEMENT_FAST = 60.0f;
const f32 Agent::TURN_SLOW = 70.0f;
const f32 Agent::TURN_FAST = 180.0f;

const f32 Agent::DEFAULT_GIRTH = 15.0; // FIXME: This was arbitrarily chosen.

s32 Agent::genID()
{
	assert( nextAvailableID >= 0 );
	return nextAvailableID ++ ;
}// genID()

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


// id=ctor ⁅This is a search marker for me in Vim⁆
Agent::Agent(IrrlichtDevice* d, stringw mesh, stringw t, stringw h, const vector3df& p):
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
	currentState( Agent::MOVE ),
	attackTarget(NULL),
	hasMoveTarget(false)
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
    body->setMD2Animation(scene::EMAT_STAND);
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
{	currentState = s;	}// setState()

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

// TODO: Do I need this?
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

void Agent::clearCurrentAction()
{
	if( currentAction != NULL )
	{
		delete currentAction;
		currentAction = NULL;
	}// if
}// clearCurrentAction()

void Agent::setCurrentAction( actions::ITickAction* const newact )
{
	clearCurrentAction();

	currentAction = newact;
}// setCurrentAction()

actions::ActAgentSeekPosition* const Agent::Goto( const vector3df& dest, f32 speed )
{
	actions::ActAgentSeekPosition* newact = new actions::ActAgentSeekPosition(*this, dest, speed );
	assert(newact);
	setCurrentAction(newact);
	return newact;
}// Goto()

// id=seek
actions::ITickAction* const Agent::Seek( const vector3df& dest, f32 speed, const Wall& w, bool debug )
{
	return visitWaypoints( const_cast<Wall&>(w).AStar(getBody().getPosition(), dest, 2, debug), speed );
}// Seek()

void Agent::clearAllActions()
{
	// TODO: Fairly unimportant for now, with our using just one action
	clearCurrentAction();
}// clearAllActions()

//inline void Agent::turnAtomic( const relAngle& theta )
//{	setRotation( getRotation() + theta );	}// turnAtomic()

void Agent::doTickActions( f32 frameDeltaTime )
{
	using namespace actions;

	//for( ActionsList::iterator it = actionsList.begin(); it != actionsList.end(); ++it )
	//{	it->runTick( frameDeltaTime );	}// for
	if( currentAction )
	{
		const bool bDone = currentAction->runTick(frameDeltaTime);
		if( bDone )
		{
			delete currentAction;
			currentAction = NULL;
		}// if
	}// if
}// doTickActions

const Agent* Agent::getAttackTarget() const
{	return attackTarget;	}//

Agent* Agent::getAttackTarget()
{	return attackTarget;	}//

void Agent::setAttackTarget( Agent* const targ )
{	attackTarget = targ;	}

//bool Agent::isEnemyVisible()
//{

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
    range = 7;
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

// id=action
namespace actions
{

ActAgentSeekPosition::ActAgentSeekPosition( Agent& agt, const vector3df& dest, f32 spd ): agent(agt), destination(dest), speed(spd)
{}// ctor

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

	agent.getBody().setPosition( agent.getBody().getPosition() + distVec );
	agent.getBody().updateAbsolutePosition();

	if( agent.getBody().getAbsolutePosition() == destination )
	{
//dpr("Arrived at " << destination);
		//waypoint.setFancyGraphic(false);
		return true;
	}// if
	else
	{	return false;	}// else
}// runTick()

}// actions

}// cj

